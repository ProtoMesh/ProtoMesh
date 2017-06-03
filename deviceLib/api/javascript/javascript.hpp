#ifndef UCL_JAVASCRIPT_HPP
#define UCL_JAVASCRIPT_HPP

#include <string>
#include "duktape.h"

using namespace std;

class JSContext {
    duk_context* ctx;
public:
    JSContext();
    ~JSContext();
};


#endif //UCL_JAVASCRIPT_HPP
