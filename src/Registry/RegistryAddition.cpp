#include "Registry.hpp"

#ifdef UNIT_TESTING
#include "catch.hpp"
#endif

template class Registry<vector<uint8_t>>;

/// -------------------------------------------- Validation & HEAD Updates --------------------------------------------

static duk_ret_t nativePrint(duk_context *ctx) {
    duk_push_string(ctx, " ");
    duk_insert(ctx, 0);
    duk_join(ctx, duk_get_top(ctx) - 1);
    printf("%s\n", duk_safe_to_string(ctx, -1));
    return 0;
}

static void jsErrorHandler(void *udata, const char *msg) {
    (void) udata;  /* ignored in this case, silence warning */

    /* Note that 'msg' may be NULL. */
    cerr << "*** FATAL ERROR: " << (msg ? msg : "no message") << endl;
}

template <typename VALUE_T>
vector<bool> Registry<VALUE_T>::validateEntries(string validator) {
    // TODO Verify the signature
    // entry.verifySignature(trustedKeys)

    const char* iterator = R"(
        function validate(entries) {
            var validationResults = [];

            for (var entry in entries) {
                validationResults.push(validator(entries, entry));
            }

            return validationResults;
        }
    )";

    duk_context* ctx(duk_create_heap(NULL, NULL, NULL, NULL, jsErrorHandler));

    duk_push_c_function(ctx, nativePrint, DUK_VARARGS);
    duk_put_global_string(ctx, "print");

    auto pushEntryObject = [&] (RegistryEntry<VALUE_T> entry) {
        duk_idx_t obj_idx;

        auto pushObjectProperty = [&] (string key, string value) {
            duk_push_string(ctx, value.c_str());
            duk_put_prop_string(ctx, obj_idx, key.c_str());
        };

        string type;
        switch (entry.type) {
            case UPSERT: type = "UPSERT"; break;
            case DELETE: type = "DELETE"; break;
        }

        obj_idx = duk_push_object(ctx);
        pushObjectProperty("uuid", string(entry.uuid));
        pushObjectProperty("parentUUID", string(entry.parentUUID));
        pushObjectProperty("publicKeyUsed", string(entry.publicKeyUsed.begin(), entry.publicKeyUsed.end()));
        pushObjectProperty("type", type);
        pushObjectProperty("key", entry.key);
    };

    auto pushEntries = [&] () {
        duk_idx_t arr_idx;

        arr_idx = duk_push_array(ctx);
        duk_uarridx_t index = 0;
        for (auto entry : this->entries) {
            pushEntryObject(entry);
            duk_put_prop_index(ctx, arr_idx, index++);
        }

        return arr_idx;
    };

    /// Register the validator
    duk_eval_string(ctx, validator.c_str());
    duk_push_string(ctx, "validator");

    /// Register the validation iterator and set it as the global object
    duk_eval_string(ctx, iterator);
    duk_get_global_string(ctx, "validate");

    /// Push the entries as an argument to the iterator
    pushEntries();

    /// Call the iterator
    if (duk_pcall(ctx, 1 /*numargs*/ ) != 0) {
        printf("Error: %s\n", duk_safe_to_string(ctx, -1));
    } else {
        if (duk_is_array(ctx, -1)) {
            vector<bool> validationResults;

            for (duk_uarridx_t i = 0; i < duk_get_length(ctx, -1); i++) {
                duk_get_prop_index(ctx, -1, i);

                // TODO this is very hacky....do this without to_string but instead fetch the boolean directly
                string boolean(duk_safe_to_string(ctx, -1));
                if (boolean == "false") validationResults.push_back(false);
                else if (boolean == "true") validationResults.push_back(true);
                else {
                    cerr << "Validator returned non-boolean value." << endl;
                    return validationResults;
                }

                duk_pop(ctx);
            }
            return validationResults;
        }
    }

    duk_pop(ctx);

    return {};
}


template <typename VALUE_T>
void Registry<VALUE_T>::updateHead(bool save) {
    using namespace lumos::registry;

    this->headState.clear();
    this->hashChain.clear();

    string validator = R"(
        function validator(entries, entryID) {
            // print('VALIDATOR CALLED w/', entryID, JSON.stringify(entries));
            // TODO Implement logic
            return true;
        }
    )";
    vector<bool> validationResults = validateEntries(validator);

    flatbuffers::FlatBufferBuilder builder;
    vector<flatbuffers::Offset<Entry>> entryOffsets;

    string lastHash = "";

    size_t entryIndex = 0;
    for (auto &entry : entries) {
        // Save entries
        entryOffsets.push_back(entry.toFlatbufferOffset(builder));
        // Generate hash for the entry
        lastHash = Crypto::hash::sha512(lastHash + entry.getSignatureText());
        this->hashChain.push_back(lastHash);
        // Check if the entry is valid
        if (!(entryIndex < validationResults.size() && validationResults[entryIndex])) continue;
//        if (entry.verifySignature(this->trustedKeys) != RegistryEntry::Verify::OK) continue; /// TODO Verify signature
        // Update head state
        switch (entry.type) {
            case RegistryEntryType::UPSERT:
                this->headState[entry.key] = entry.value;
                break;
            case RegistryEntryType::DELETE:
                this->headState.erase(entry.key);
                break;
        }

        entryIndex++;
    }

    if (save) {
        auto entries = builder.CreateVector(entryOffsets);
        auto registry = CreateRegistry(builder, entries);
        builder.Finish(registry, RegistryIdentifier());

        uint8_t *buf = builder.GetBufferPointer();
        vector<uint8_t> serializedRegistry(buf, buf + builder.GetSize());
        this->stor->set(REGISTRY_STORAGE_PREFIX + this->name, serializedRegistry);
    }
}



/// ---------------------------------------- Entry deserialization & Addition -----------------------------------------

template <typename VALUE_T>
bool Registry<VALUE_T>::addEntry(RegistryEntry<VALUE_T> newEntry, bool save) {
    if (!newEntry.valid) return false;

    auto lastBorder = this->entries.size();
    auto insertAt = [&] (size_t index) {
        this->entries.insert(this->entries.begin() + index, newEntry);
        this->updateHead(save);

#ifdef UNIT_TESTING
        if (debug) cout << "Inserted entry at " << index << ": " << newEntry.uuid << endl << this->getEntries() << endl;
#endif
    };

    /// Go through from the back
    for (unsigned long i = this->entries.size(); i-- > 0;) {
        auto entry = this->entries[i];

        // We reached our direct parent. Insert after it
        if (entry.uuid == newEntry.parentUUID) {
            insertAt(i + 1);
            return true;
        }

        /// We've hit a different entry that has the same parent
        if (entry.parentUUID == newEntry.parentUUID) {
            /// In case we are smaller save its position for later
            if (newEntry.uuid < entry.uuid) lastBorder = i;
                /// In case we are bigger take the position of the last border we encountered and insert ourselves there
            else if (newEntry.uuid > entry.uuid) {
                insertAt(lastBorder);
                return true;
            }
                /// This entry is equal (a duplicate) to the entry we encountered! Abort and don't insert.
            else return false;
        }
    }

//    // TODO Search for entries that we are a child of and insert after them w/ reverse logic
//    cout << "searching for parents" << endl;
//    for (unsigned long i = 0; i < this->entries.size(); i++) {
//        auto entry = this->entries[i];
//
//        // Encountered entry wants to have us as a child
//        if (entry.parentUUID == newEntry.uuid) {
//            cout << "found an ancestor" << endl;
//            insertAt(i);
//            return true;
//        }
//    }

    /// We haven't found any parent or ancestor so just add it to the back (or front if its parent is empty)
    /// IMPORTANT: It is possible that we are smaller than the only ancestor found. Because of that we insert at lastBorder
    ///              which contains its position or, if that is not applicable, defaults to the end of the list.
    if (newEntry.parentUUID == Crypto::UUID::Empty()) insertAt(0);
    else insertAt(lastBorder);

    return true;
}

template <typename VALUE_T>
void Registry<VALUE_T>::addEntries(list<RegistryEntry<VALUE_T>> newEntries, size_t startingIndex, bool save) {
#ifdef UNIT_TESTING
    if (debug) cout << "Got entries list with size: " << newEntries.size() << endl;
#endif


    // TODO This still fails if there are multiple entries with their parent == Crypto::UUID::Empty() at arbitrary locations


    size_t prevSize = newEntries.size();
    while (newEntries.size() > 0) {
        vector<RegistryEntry<VALUE_T>> pendingAdditions;

        /// Search for entries in newEntries that have parents in this->entries
        for (size_t ei = startingIndex; ei < this->entries.size(); ei++) {
            auto entry = this->entries[ei];

            auto it = newEntries.begin();
            while (it != newEntries.end()) {
                if ((*it).parentUUID == entry.uuid || (*it).parentUUID == Crypto::UUID::Empty()) {
                    /// Insert the entry into the list of pending additions
                    pendingAdditions.push_back((*it));
                    it = newEntries.erase(it);
                } else if ((*it).uuid == entry.uuid) {
                    /// It is a duplicate so just erase it
                    it = newEntries.erase(it);
                } else it++;
            }
        }

        /// Add the matched entries to the registry
        for (auto addition : pendingAdditions) this->addEntry(addition, save);


        /// If the size didn't change there's a hanging branch left over.
        if (prevSize == newEntries.size()) {
            // TODO This is relatively inefficient ~ O(n^2)
            /// Search for entries with a missing parent and insert them
            auto it = newEntries.begin();
            while (it != newEntries.end()) {
                bool hasParent = false;
                for (auto entry : newEntries) if (entry.uuid == (*it).parentUUID) { hasParent = true; break; }
                for (auto entry : pendingAdditions) if (entry.uuid == (*it).parentUUID) { hasParent = true; break; }
                if (!hasParent) {
                    pendingAdditions.push_back((*it));
                    it = newEntries.erase(it);
                } else it++;
            }

            for (auto addition : pendingAdditions) {
                this->addEntry(addition, save);
            }
        }

        prevSize = newEntries.size();
    }
}

template <typename VALUE_T>
bool Registry<VALUE_T>::addSerializedEntry(const lumos::registry::Entry* serialized, bool save) {
    return this->addEntry(RegistryEntry<VALUE_T>(serialized), save);
}



#ifdef UNIT_TESTING
    #include "flatbuffers/idl.h"

    SCENARIO("Database/Registry", "[registry]") {
        GIVEN("a cleared registry and a KeyPair") {
            Crypto::asym::KeyPair pair(Crypto::asym::generateKeyPair());

            map<PUB_HASH_T, Crypto::asym::PublicKey *> keys = { { pair.pub.getHash(), &pair.pub } };
            DummyNetworkHandler dnet;
            DummyStorageHandler dstor;
            REL_TIME_PROV_T drelTimeProv(new DummyRelativeTimeProvider);

            Registry<vector<uint8_t>> reg("someRegistry", &dstor, &dnet, drelTimeProv);

            WHEN("a serialized entry is added twice") {

                std::string schemafile;
                std::string jsonfile;
                bool ok = flatbuffers::LoadFile("src/buffers/registry/entry.fbs", false, &schemafile) &&
                          flatbuffers::LoadFile("src/test/data/registry_entry.json", false, &jsonfile);
                REQUIRE(ok);

                flatbuffers::Parser parser;
                const char *include_directories[] = { "src/buffers/", "src/buffers/registry", nullptr };
                ok = parser.Parse(schemafile.c_str(), include_directories) &&
                     parser.Parse(jsonfile.c_str(), include_directories);

                CAPTURE(parser.error_);
                REQUIRE(ok);

                auto entry = lumos::registry::GetEntry(parser.builder_.GetBufferPointer());
                reg.addSerializedEntry(entry);
                reg.addSerializedEntry(entry);

                THEN("the second one should be omitted") {
                    REQUIRE(reg.entries.size() == 1);
                }
            }

            WHEN("a value is set") {
                string key("someKey");
                vector<uint8_t> val = {1, 2, 3, 4, 5};
                vector<uint8_t> empty = {};
                reg.set(key, val, pair);
                string prevHeadHash(reg.getHeadHash());

                THEN("has should be true") {
                    REQUIRE(reg.has(key));
                }
                THEN("the read value should be equal") {
                    REQUIRE( reg.get(key) == val );
                }

                AND_WHEN("the registry is cleared") {
                    reg.clear();

                    THEN("the read value should be empty") {
                        REQUIRE( reg.get(key) == empty );
                    }
                }

                AND_WHEN("the value is deleted") {
                    reg.del(key, pair);

                    THEN("the read value should be empty") {
                        REQUIRE( reg.get(key) == empty );
                    }

                    THEN("the head hash should differ") {
                        REQUIRE_FALSE( reg.getHeadHash() == prevHeadHash );
                    }
                }
            }

            // TODO Reimplement these tests according to the new way of doing things!
//            WHEN("a value is set with a different public key (not in the list of trusted keys)") {
//                string key("someKey");
//                string val("someValue");
//                Crypto::asym::KeyPair otherPair(Crypto::asym::generateKeyPair());
//                reg.set(key, val, otherPair);
//
//                THEN("has should be false") {
//                    REQUIRE_FALSE(reg.has(key));
//                }
//
//                THEN("the read value should be empty") {
//                    REQUIRE( reg.get(key) == "" );
//                }
//            }
        }
    }
#endif