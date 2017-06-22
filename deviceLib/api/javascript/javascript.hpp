#ifndef OPEN_HOME_JAVASCRIPT_HPP
#define OPEN_HOME_JAVASCRIPT_HPP

#include <string>
#include "duktape.h"

using namespace std;

class JSContext {
    duk_context* ctx;
public:
    JSContext();
    ~JSContext();
};


#endif //OPEN_HOME_JAVASCRIPT_HPP
