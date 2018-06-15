#include <set>
#include "std.h"

static Package package(
    {
        {"pair?",        [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("pair?: one argument required");
            auto p = std::dynamic_pointer_cast<SchemePair>(l.front());
            return (p && p != scheme_nil) ? scheme_true : scheme_false;
        }
        },
        {"symbol?",      [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("symbol?: one argument required");
            auto p = std::dynamic_pointer_cast<SchemeName>(l.front());
            return p ? scheme_true : scheme_false;
        }
        },
        {"string?",      [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("string?: one argument required");
            auto p = std::dynamic_pointer_cast<SchemeString>(l.front());
            return p ? scheme_true : scheme_false;
        }
        },
        {"char?",        [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("char?: one argument required");
            auto p = std::dynamic_pointer_cast<SchemeChar>(l.front());
            return p ? scheme_true : scheme_false;
        }
        },
        {"number?",      [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("number?: one argument required");
            auto p1 = std::dynamic_pointer_cast<SchemeInt>(l.front());
            if(p1)
                return scheme_true;
            auto p2 = std::dynamic_pointer_cast<SchemeFloat>(l.front());
            return p2 ? scheme_true : scheme_false;
        }
        },
        {"environment?", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("environment?: one argument required");
            auto p = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            return p ? scheme_true : scheme_false;
        }
        },
        {"boolean?",     [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("boolean?: one argument required");
            auto p = std::dynamic_pointer_cast<SchemeBool>(l.front());
            return p ? scheme_true : scheme_false;
        }
        },
        {"vector?",      [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("vector?: one argument required");
            auto p = std::dynamic_pointer_cast<SchemeVector>(l.front());
            return p ? scheme_true : scheme_false;
        }
        },
        {"procedure?",   [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("procedure?: one argument required");
            auto p = std::dynamic_pointer_cast<SchemeFunc>(l.front());
            return p ? scheme_true : scheme_false;
        }
        },
        {"list?",        [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("list?: one argument required");
            auto p = std::dynamic_pointer_cast<SchemePair>(l.front());
            return p && p->listLength() >= 0 ? scheme_true : scheme_false;
        }
        },
    }
);