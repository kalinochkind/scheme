#include <iostream>
#include "std.h"

static FunctionPackage package(
    {
        {"length",    [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("length: a list required");
            auto p = std::dynamic_pointer_cast<SchemePair>(l.front());
            long long len;
            if(!p || (len = p->listLength()) < 0)
                throw eval_error("length: a list required");
            return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeInt>(len));
        }
        },
        {"list-head", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("list-head: list and position required");
            auto lp = std::dynamic_pointer_cast<SchemePair>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!lp || !ip)
                throw eval_error("list-head: list and position required");
            if(ip->value <= 0)
                return scheme_nil;
            if(lp == scheme_nil)
                throw eval_error("list-head: invalid or too short list");
            auto nl = std::make_shared<SchemePair>(lp->car, scheme_nil);
            auto cl = nl;
            for(long long i=1;i<ip->value;++i)
            {
                lp = std::dynamic_pointer_cast<SchemePair>(lp->cdr);
                if(!lp || lp == scheme_nil)
                    throw eval_error("list-head: invalid or too short list");
                auto node =  std::make_shared<SchemePair>(lp->car, scheme_nil);
                cl->cdr = node;
                cl = node;
            }
            return std::dynamic_pointer_cast<SchemeObject>(nl);
        }
        },
    }
);
