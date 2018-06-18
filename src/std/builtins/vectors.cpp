#include <algorithm>
#include "std.h"

static FunctionPackage package(
    {
        {"make-vector",   [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() < 1 || l.size() > 2)
                throw eval_error("make-vector: length required");
            auto lp = std::dynamic_pointer_cast<SchemeInt>(l.front());
            if(!lp)
                throw eval_error("make-vector: length required");
            long long len = lp->value;
            std::shared_ptr<SchemeObject> c = scheme_empty;
            if(l.size() == 2)
            {
                c = l.back();
            }
            auto vec = std::make_shared<SchemeVector>();
            vec->vec.resize(len, c);
            return to_object(vec);
        }
        },
        {"list->vector",  [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("list->vector: list required");
            auto lp = std::dynamic_pointer_cast<SchemePair>(l.front());
            if(!lp)
                throw eval_error("list->vector: list required");
            auto vec = SchemeVector::fromList(lp);
            return to_object(vec);
        }
        },
        {"vector->list",  [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("vector->list: vector required");
            auto lp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            if(!lp)
                throw eval_error("vector->list: vector required");
            auto lst = lp->toList();
            return to_object(lst);
        }
        },
        {"vector-grow",   [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("vector-grow: vector and length required");
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto lp = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!vp || !lp)
                throw eval_error("vector-grow: vector and length required");
            auto np = std::make_shared<SchemeVector>();
            np->vec = vp->vec;
            np->vec.resize(lp->value, scheme_empty);
            return to_object(np);
        }
        },
        {"vector-length", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("vector-length: vector required");
            auto lp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            if(!lp)
                throw eval_error("vector-length: vector required");
            return to_object(std::make_shared<SchemeInt>(lp->vec.size()));
        }
        },
        {"vector-ref",    [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("vector-ref: vector and index required");
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!vp || !ip)
                throw eval_error("vector-ref: vector and index reuired");
            if(ip->value < 0 || ip->value >= (int) vp->vec.size())
                throw eval_error("vector-ref: out of range");
            return vp->vec[ip->value];
        }
        },
        {"vector-set!",   [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 3)
                throw eval_error("vector-set!: vector, index and value required");
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            if(!vp || !ip)
                throw eval_error("vector-set!: vector, index and value required");
            if(ip->value < 0 || ip->value >= (int) vp->vec.size())
                throw eval_error("vector-set!: out of range");
            vp->vec[ip->value] = l.back();
            return scheme_empty;
        }
        },
        {"subvector",     [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 3)
                throw eval_error("subvector: vector and indices required");
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto bp = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            auto ep = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!vp || !bp || !ep)
                throw eval_error("subvector: vector and indices required");
            if(ep->value > (int) vp->vec.size() || bp->value < 0 || ep->value < bp->value)
                throw eval_error("subvector: invalid range");
            auto ns = std::make_shared<SchemeVector>();
            ns->vec = std::vector<std::shared_ptr<SchemeObject>>(
                vp->vec.begin() + bp->value, vp->vec.begin() + ep->value);
            return to_object(ns);
        }
        },
        {"quick-sort!",   [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("quick-sort!: vector and comparator required");
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto cp = std::dynamic_pointer_cast<SchemeFunc>(l.back());
            if(!vp || !cp)
                throw eval_error("quick-sort!: vector and comparator required");
            std::sort(vp->vec.begin(), vp->vec.end(),
                      [cp](const std::shared_ptr<SchemeObject> &a, const std::shared_ptr<SchemeObject> &b) {
                          return execute_function(cp, {a, b}).force_value()->toBool();
                      });
            return l.front();
        }
        },
        {"merge-sort!",   [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("merge-sort!: vector and comparator required");
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto cp = std::dynamic_pointer_cast<SchemeFunc>(l.back());
            if(!vp || !cp)
                throw eval_error("merge-sort!: vector and comparator required");
            std::stable_sort(vp->vec.begin(), vp->vec.end(),
                             [cp](const std::shared_ptr<SchemeObject> &a, const std::shared_ptr<SchemeObject> &b) {
                                 return execute_function(cp, {a, b}).force_value()->toBool();
                             });
            return l.front();
        }
        },
    }
);
