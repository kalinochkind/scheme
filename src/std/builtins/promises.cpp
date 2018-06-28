#include "std.h"

static FunctionPackage package(
    {
        {"force", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto t = std::dynamic_pointer_cast<SchemePromise>(l.front());
            if(!t)
                return l.front();
            return to_object(t->force());
        }}},
        {"promise-forced?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto t = std::dynamic_pointer_cast<SchemePromise>(l.front());
            if(!t)
                throw eval_error("promise-forced?: a promise required");
            return t->value ? scheme_true : scheme_false;
        }}},
        {"promise-value", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("promise-value: a forced promise required");
            auto t = std::dynamic_pointer_cast<SchemePromise>(l.front());
            if(!t || !t->value)
                throw eval_error("promise-value: a forced promise required");
            return t->value;
        }}},
    }
);
