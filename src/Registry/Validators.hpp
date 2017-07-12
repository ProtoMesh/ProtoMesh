#include "Registry.hpp"

static const string EVERYBODY_ITS_OWN = R"(
    function validator(entries, entryID) {
            var entry = entries[entryID];
            // Find the insertion of this key
            for (var e in entries) {
                e = entries[e];
                if (e.key == entry.key && e.type === 'UPSERT') {
                    if (e.publicKeyUsed == entry.publicKeyUsed) {
                        return true;
                    } else {
                        return false;
                    }
                }
            }

            // Since the key was never inserted before it is valid
            return true;
    }
)";

static const string DEFAULT_VALIDATOR = EVERYBODY_ITS_OWN;
