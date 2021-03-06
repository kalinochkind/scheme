#include "std.h"

static FunctionPackage package(
    {
        {"symbol->string", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeSymbol>(l.front());
            if(!sp)
                throw eval_error("symbol->string: a symbol required");
            return to_object(std::make_shared<SchemeString>(sp->value));
        }}},
        {"string->symbol", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!sp)
                throw eval_error("string->symbol: a string required");
            return to_object(std::make_shared<SchemeSymbol>(sp->value));
        }}},
        {"string->uninterned-symbol", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!sp)
                throw eval_error("string->uninterned-symbol: a string required");
            auto res = std::make_shared<SchemeSymbol>(sp->value);
            res->uninterned = true;
            return to_object(res);
        }}},
        {"generate-uninterned-symbol", {0, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::string prefix = "G";
            if(l.size())
            {
                auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
                if(sp)
                    prefix = sp->value;
                else
                {
                    auto np = std::dynamic_pointer_cast<SchemeSymbol>(l.front());
                    if(np)
                        prefix = np->value;
                    else
                    {
                        auto ip = std::dynamic_pointer_cast<SchemeInt>(l.front());
                        if(ip && ip->value >= 0)
                            SchemeSymbol::uninterned_counter = ip->value;
                        else if(l.front() != scheme_false)
                            throw eval_error("generate-uninterned-symbol: invalid argument");
                    }
                }
            }
            auto res = std::make_shared<SchemeSymbol>(prefix + std::to_string(SchemeSymbol::uninterned_counter++));
            res->uninterned = true;
            return to_object(res);
        }}},
    }
);
