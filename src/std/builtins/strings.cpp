#include "std.h"

static Package package(
    {
        {"make-string",            [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() < 1 || l.size() > 2)
                throw eval_error("make-string: length required");
            auto lp = std::dynamic_pointer_cast<SchemeInt>(l.front());
            if(!lp)
                throw eval_error("make-string: length required");
            int len = lp->value;
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
            return std::dynamic_pointer_cast<SchemeObject>(str);
        }
        },
        {"string",                 [](const std::list<std::shared_ptr<SchemeObject>> &l) {
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
            return std::dynamic_pointer_cast<SchemeObject>(str);
        }
        },
        {"string->list",           [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("string->list: a string required");
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!sp)
                throw eval_error("string->list: a string required");
            std::shared_ptr<SchemePair> lst = std::dynamic_pointer_cast<SchemePair>(scheme_nil);
            for(auto it = sp->value.rbegin(); it != sp->value.rend(); ++it)
            {
                lst = std::make_shared<SchemePair>(std::make_shared<SchemeChar>(*it), lst);
            }
            return std::dynamic_pointer_cast<SchemeObject>(lst);
        }
        },
        {"string-length",          [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("string-length: a string required");
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            if(!sp)
                throw eval_error("string-length: a string required");
            auto len = std::make_shared<SchemeInt>(sp->value.length());
            return std::dynamic_pointer_cast<SchemeObject>(len);
        }
        },
        {"string-ref",             [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("string-ref: string and index required");
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!sp || !ip)
                throw eval_error("string-ref: string and index reuired");
            if(ip->value < 0 || ip->value >= (int) sp->value.length())
                throw eval_error("string-ref: out of range");
            auto ch = std::make_shared<SchemeChar>(sp->value[ip->value]);
            return std::dynamic_pointer_cast<SchemeObject>(ch);
        }
        },
        {"string-set!",            [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 3)
                throw eval_error("string-set!: string, index and char required");
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto ip = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            auto cp = std::dynamic_pointer_cast<SchemeChar>(l.back());
            if(!sp || !ip || !cp)
                throw eval_error("string-set!: string, index and char required");
            if(ip->value < 0 || ip->value >= (int) sp->value.length())
                throw eval_error("string-set!: out of range");
            sp->value[ip->value] = cp->value;
            return scheme_empty;
        }
        },
        {"substring",              [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 3)
                throw eval_error("substring: string and indices required");
            auto sp = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto bp = std::dynamic_pointer_cast<SchemeInt>(*next(l.begin()));
            auto ep = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!sp || !bp || !ep)
                throw eval_error("substring: string and indices required");
            if(ep->value > (int) sp->value.length() || bp->value < 0 || ep->value < bp->value)
                throw eval_error("substring: invalid range");
            auto ns = std::make_shared<SchemeString>(sp->value.substr(bp->value, ep->value - bp->value));
            return std::dynamic_pointer_cast<SchemeObject>(ns);
        }
        },
        {"string-append",          [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::string res;
            for(const auto &i : l)
            {
                auto p = std::dynamic_pointer_cast<SchemeString>(i);
                if(!p)
                    throw eval_error("string-append: strings required");
                res += p->value;
            }
            auto s = std::make_shared<SchemeString>(res);
            return std::dynamic_pointer_cast<SchemeObject>(s);
        }
        },
        {"string-pad-left",        [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() < 2 || l.size() > 3)
                throw eval_error("string-pad-left: string and length required");
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
            return std::dynamic_pointer_cast<SchemeObject>(str);
        }
        },
        {"string-pad-right",       [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() < 2 || l.size() > 3)
                throw eval_error("string-pad-right: string and length required");
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
            return std::dynamic_pointer_cast<SchemeObject>(str);
        }
        },
        {"string=?",               [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("string=?: two strings required");
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string=?: two strings required");
            return (p1->value == p2->value) ? scheme_true : scheme_false;
        }
        },
        {"string<?",               [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("string<?: two strings required");
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string<?: two strings required");
            return (p1->value < p2->value) ? scheme_true : scheme_false;
        }
        },
        {"string-search-forward",  [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("string-search-forward: two strings required");
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string-search-forward: two strings required");
            size_t pos = p2->value.find(p1->value);
            if(pos == std::string::npos)
                return scheme_false;
            auto res = std::make_shared<SchemeInt>(pos);
            return std::dynamic_pointer_cast<SchemeObject>(res);
        }
        },
        {"string-search-backward", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("string-search-backward: two strings required");
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string-search-backward: two strings required");
            size_t pos = p2->value.rfind(p1->value);
            if(pos == std::string::npos)
                return scheme_false;
            pos += p1->value.length();
            auto res = std::make_shared<SchemeInt>(pos);
            return std::dynamic_pointer_cast<SchemeObject>(res);
        }
        },
        {"string-search-all",      [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("string-search-all: two strings required");
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
            return std::dynamic_pointer_cast<SchemeObject>(res);
        }
        },
        {"string-match-forward",   [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("string-match-forward: two strings required");
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string-match-forward: two strings required");
            size_t pos = 0;
            while(pos < p1->value.length() && pos < p2->value.length() && p1->value[pos] == p2->value[pos])
                ++pos;
            auto res = std::make_shared<SchemeInt>(pos);
            return std::dynamic_pointer_cast<SchemeObject>(res);
        }
        },
        {"string-match-backward",  [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("string-match-backward: two strings required");
            auto p1 = std::dynamic_pointer_cast<SchemeString>(l.front());
            auto p2 = std::dynamic_pointer_cast<SchemeString>(l.back());
            if(!p1 || !p2)
                throw eval_error("string-match-backward: two strings required");
            size_t pos = 0;
            while(pos < p1->value.length() && pos < p2->value.length() &&
                  p1->value[p1->value.length() - 1 - pos] == p2->value[p2->value.length() - 1 - pos])
                ++pos;
            auto res = std::make_shared<SchemeInt>(pos);
            return std::dynamic_pointer_cast<SchemeObject>(res);
        }
        },
    }
);
