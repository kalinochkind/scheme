#include "std.h"

static BuiltinFunction pair_function(const std::string &name)
{
    return {1, 1, [name](const std::list<std::shared_ptr<SchemeObject>> &l) {
        auto p = l.front();
        for(size_t i = name.length() - 2; i; --i)
        {
            auto pp = std::dynamic_pointer_cast<SchemePair>(p);
            if(!pp || p == scheme_nil)
                throw eval_error(name + ": invalid argument");
            p = name[i] == 'a' ? pp->car : pp->cdr;
        }
        return p;
    }};
}


static FunctionPackage package(
    {
        {"cons", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return to_object(std::make_shared<SchemePair>(l.front(), l.back()));
        }}},
        {"set-car!", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePair>(l.front());
            if(!p || p == scheme_nil)
                throw eval_error("set-car!: a pair required");
            p->car = l.back();
            return l.front();
        }}},
        {"set-cdr!", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePair>(l.front());
            if(!p || p == scheme_nil)
                throw eval_error("set-cdr!: a pair required");
            p->cdr = l.back();
            return l.front();
        }}},
        {"car", pair_function("car")},
        {"cdr", pair_function("cdr")},
        {"caar", pair_function("caar")},
        {"cadr", pair_function("cadr")},
        {"cdar", pair_function("cdar")},
        {"cddr", pair_function("cddr")},
        {"caaar", pair_function("caaar")},
        {"caadr", pair_function("caadr")},
        {"cadar", pair_function("cadar")},
        {"caddr", pair_function("caddr")},
        {"cdaar", pair_function("cdaar")},
        {"cdadr", pair_function("cdadr")},
        {"cddar", pair_function("cddar")},
        {"cdddr", pair_function("cdddr")},
    }
);
