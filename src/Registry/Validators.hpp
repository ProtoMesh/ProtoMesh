#include "Registry.hpp"

// TODO Accomodate for: addition -> deletion -> addition -> entry
// TODO Add master key as override
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

static const string DEFAULT_VALIDATOR = EVERYBODY_HIS_OWN;
