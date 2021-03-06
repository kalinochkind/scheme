#include <algorithm>
#include "std.h"

static FunctionPackage package(
    {
        {"make-string", {1, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto lp = std::dynamic_pointer_cast<SchemeInt>(l.front());
            if(!lp)
                throw eval_error("make-string: length required");
            long long len = lp->value;
            if(len < 0)
                throw eval_error("make-string: invalid length");
            char c = 0;
            if(l.size() == 2)
            {
                auto cp = std::dynamic_pointer_cast<SchemeChar>(l.back());
                if(!cp)
                    throw eval_error("make-string: invalid character");
                c = cp->value;
            }
            auto str = std::make_shared<SchemeString>("");
            str->value.resize(len, c);
            return to_object(str);
        }}},
        {"string", {0, -1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto str = std::make_shared<SchemeString>("");
            str->value.resize(l.size());
            int idx = 0;
            for(const auto &p : l)
            {
                auto cp = std::dynamic_pointer_cast<SchemeChar>(p);
                if(!cp)
                    throw eval_error("string: chars required");
                str->value[idx++] = cp->value;
            }
            return to_object(str);
        }}},
        {"string->list", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!sp)
                throw eval_error("string->list: a string required");
            std::shared_ptr<SchemePair> lst = std::dynamic_pointer_cast<SchemePair>(scheme_nil);
            for(auto it = sp->value.rbegin(); it != sp->value.rend(); ++it)
            {
                lst = std::make_shared<SchemePair>(std::make_shared<SchemeChar>(*it), lst);
            }
            return to_object(lst);
        }}},
        {"string-length", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!sp)
                throw eval_error("string-length: a string required");
            auto len = std::make_shared<SchemeInt>(sp->value.length());
            return to_object(len);
        }}},
        {"string-ref", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!sp || !ip)
                throw eval_error("string-ref: string and index reuired");
            if(ip->value < 0 || ip->value >= (int) sp->value.length())
                throw eval_error("string-ref: out of range");
            auto ch = std::make_shared<SchemeChar>(sp->value[ip->value]);
            return to_object(ch);
        }}},
        {"string-set!", {3, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            auto cp = std::dynamic_pointer_cast<SchemeChar>(l.back());
            if(!sp || !ip || !cp)
                throw eval_error("string-set!: string, index and char required");
            if(ip->value < 0 || ip->value >= (int) sp->value.length())
                throw eval_error("string-set!: out of range");
            sp->value[ip->value] = cp->value;
            return scheme_empty;
        }}},
        {"substring", {3, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto bp = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            auto ep = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!sp || !bp || !ep)
                throw eval_error("substring: string and indices required");
            if(ep->value > (int) sp->value.length() || bp->value < 0 || ep->value < bp->value)
                throw eval_error("substring: invalid range");
            auto ns = std::make_shared<SchemeString>(sp->value.substr(bp->value, ep->value - bp->value));
            return to_object(ns);
        }}},
        {"string-append", {0, -1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::string res;
            for(const auto &i : l)
            {
                auto p = std::dynamic_pointer_cast<SchemeString>(i);
                if(!p)
                    throw eval_error("string-append: strings required");
                res += p->value;
            }
            auto s = std::make_shared<SchemeString>(res);
            return to_object(s);
        }}},
        {"string-pad-left", {2, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto lp = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!lp || !sp)
                throw eval_error("string-pad-left: string and length required");
            char c = ' ';
            if(l.size() == 3)
            {
                auto cp = std::dynamic_pointer_cast<SchemeChar>(l.back());
                if(!cp)
                    throw eval_error("string-pad-left: invalid character");
                c = cp->value;
            }
            std::string res;
            if(lp->value >= (int) sp->value.length())
                res = std::string(lp->value - sp->value.length(), c) + sp->value;
            else
                res = sp->value.substr(sp->value.length() - lp->value);
            auto str = std::make_shared<SchemeString>(res);
            return to_object(str);
        }}},
        {"string-pad-right", {2, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto lp = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!lp || !sp)
                throw eval_error("string-pad-right: string and length required");
            char c = ' ';
            if(l.size() == 3)
            {
                auto cp = std::dynamic_pointer_cast<SchemeChar>(l.back());
                if(!cp)
                    throw eval_error("string-pad-right: invalid character");
                c = cp->value;
            }
            std::string res;
            if(lp->value >= (int) sp->value.length())
                res = sp->value + std::string(lp->value - sp->value.length(), c);
            else
                res = sp->value.substr(0, lp->value);
            auto str = std::make_shared<SchemeString>(res);
            return to_object(str);
        }}},
        {"string=?", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string=?: two strings required");
            return (p1->value == p2->value) ? scheme_true : scheme_false;
        }}},
        {"string<?", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string<?: two strings required");
            return (p1->value < p2->value) ? scheme_true : scheme_false;
        }}},
        {"string-search-forward", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string-search-forward: two strings required");
            size_t pos = p2->value.find(p1->value);
            if(pos == std::string::npos)
                return scheme_false;
            auto res = std::make_shared<SchemeInt>(pos);
            return to_object(res);
        }}},
        {"string-search-backward", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string-search-backward: two strings required");
            size_t pos = p2->value.rfind(p1->value);
            if(pos == std::string::npos)
                return scheme_false;
            pos += p1->value.length();
            auto res = std::make_shared<SchemeInt>(pos);
            return to_object(res);
        }}},
        {"string-search-all", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string-search-all: two strings required");
            auto res = std::dynamic_pointer_cast<SchemePair>(scheme_nil);
            size_t pos = p2->value.rfind(p1->value);
            while(pos != std::string::npos)
            {
                res = std::make_shared<SchemePair>(std::make_shared<SchemeInt>(pos), res);
                if(pos == 0)
                    break;
                pos = p2->value.rfind(p1->value, pos - 1);
            }
            return to_object(res);
        }}},
        {"string-match-forward", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string-match-forward: two strings required");
            size_t pos = 0;
            while(pos < p1->value.length() && pos < p2->value.length() && p1->value[pos] == p2->value[pos])
                ++pos;
            auto res = std::make_shared<SchemeInt>(pos);
            return to_object(res);
        }}},
        {"string-match-backward", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string-match-backward: two strings required");
            size_t pos = 0;
            while(pos < p1->value.length() && pos < p2->value.length() &&
                  p1->value[p1->value.length() - 1 - pos] == p2->value[p2->value.length() - 1 - pos])
                ++pos;
            auto res = std::make_shared<SchemeInt>(pos);
            return to_object(res);
        }}},
        {"reverse-substring!", {3, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto bp = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            auto ep = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!sp || !bp || !ep)
                throw eval_error("reverse-substring!: string and indices required");
            if(ep->value > (int) sp->value.length() || bp->value < 0 || ep->value < bp->value)
                throw eval_error("reverse-substring!: invalid range");
            std::reverse(sp->value.begin() + bp->value, sp->value.begin() + ep->value);
            return l.front();
        }}},
        {"set-string-length!", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!sp || !ip || ip->value < 0)
                throw eval_error("set-string-length!: string and length required");
            sp->value.resize(ip->value);
            return scheme_empty;
        }}},
    }
);
