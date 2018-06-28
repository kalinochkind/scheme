#include <algorithm>
#include "std.h"

static FunctionPackage package(
    {
        {"make-vector", {1, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
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
        }}},
        {"list->vector", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto lp = std::dynamic_pointer_cast<SchemePair>(l.front());
            if(!lp)
                throw eval_error("list->vector: list required");
            auto vec = SchemeVector::from_list(lp);
            return to_object(vec);
        }}},
        {"vector->list", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto lp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            if(!lp)
                throw eval_error("vector->list: vector required");
            auto lst = lp->to_list();
            return to_object(lst);
        }}},
        {"vector-grow", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto lp = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!vp || !lp)
                throw eval_error("vector-grow: vector and length required");
            auto np = std::make_shared<SchemeVector>();
            np->vec = vp->vec;
            np->vec.resize(lp->value, scheme_empty);
            return to_object(np);
        }}},
        {"vector-length", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto lp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            if(!lp)
                throw eval_error("vector-length: vector required");
            return to_object(std::make_shared<SchemeInt>(lp->vec.size()));
        }}},
        {"vector-ref", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!vp || !ip)
                throw eval_error("vector-ref: vector and index reuired");
            if(ip->value < 0 || ip->value >= (int) vp->vec.size())
                throw eval_error("vector-ref: out of range");
            return vp->vec[ip->value];
        }}},
        {"vector-set!", {3, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            if(!vp || !ip)
                throw eval_error("vector-set!: vector, index and value required");
            if(ip->value < 0 || ip->value >= (int) vp->vec.size())
                throw eval_error("vector-set!: out of range");
            vp->vec[ip->value] = l.back();
            return scheme_empty;
        }}},
        {"subvector", {3, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
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
        }}},
        {"quick-sort!", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto cp = std::dynamic_pointer_cast<SchemeProcedure>(l.back());
            if(!vp || !cp)
                throw eval_error("quick-sort!: vector and comparator required");
            std::sort(vp->vec.begin(), vp->vec.end(),
                      [cp](const std::shared_ptr<SchemeObject> &a, const std::shared_ptr<SchemeObject> &b) {
                          return cp->execute({a, b}).force_value()->to_bool();
                      });
            return l.front();
        }}},
        {"merge-sort!", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto vp = std::dynamic_pointer_cast<SchemeVector>(l.front());
            auto cp = std::dynamic_pointer_cast<SchemeProcedure>(l.back());
            if(!vp || !cp)
                throw eval_error("merge-sort!: vector and comparator required");
            std::stable_sort(vp->vec.begin(), vp->vec.end(),
                             [cp](const std::shared_ptr<SchemeObject> &a, const std::shared_ptr<SchemeObject> &b) {
                                 return cp->execute({a, b}).force_value()->to_bool();
                             });
            return l.front();
        }}},
    }
);
