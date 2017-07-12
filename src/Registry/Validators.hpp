#include "Registry.hpp"

static const string EVERYBODY_ITS_OWN = R"(
    function validator(entries, entryID) {
        // print('VALIDATOR CALLED w/', entryID, JSON.stringify(entries));
        // TODO Implement logic
        return true;
    }
)";

static const string DEFAULT_VALIDATOR = EVERYBODY_ITS_OWN;