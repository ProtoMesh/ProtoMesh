#include <iostream>
#include "javascript.hpp"

JSContext::JSContext() : ctx(duk_create_heap_default()) {

}

JSContext::~JSContext() {
    duk_pop(this->ctx);
    duk_destroy_heap(this->ctx);
}
