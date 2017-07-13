#include "Registry.hpp"

// TODO Accomodate for: addition -> deletion -> addition -> entry
static const string EVERYBODY_HIS_OWN = R"(
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

/// Specific validator for nodes registry working by the following principle
///     By default only the master key may add entries (nodes).
///     Once a node is added he has permission to add other entries (nodes).
///     The rule that only the node who added an entry (or the master) may modify/remove entries still applies.
static const string MASTER_AND_SLAVES = R"(
    function validator(entries, entryID) {
        var entry = entries[entryID];
        
    }
)";

static const string DEFAULT_VALIDATOR = EVERYBODY_HIS_OWN;
