#include <set>
#include "std.h"

static FunctionPackage package(
    {
        {"pair?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePair>(l.front());
            return (p && p != scheme_nil) ? scheme_true : scheme_false;
        }}},
        {"symbol?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeSymbol>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"string?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeString>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"char?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeChar>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"number?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p1 = std::dynamic_pointer_cast<SchemeInt>(l.front());
            if(p1)
                return scheme_true;
            auto p2 = std::dynamic_pointer_cast<SchemeFloat>(l.front());
            return p2 ? scheme_true : scheme_false;
        }}},
        {"environment?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"boolean?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeBool>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"vector?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeVector>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"procedure?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeProcedure>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"compound-procedure?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeCompoundProcedure>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"primitive-procedure?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePrimitiveProcedure>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"list?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePair>(l.front());
            return p && p->list_length() >= 0 ? scheme_true : scheme_false;
        }}},
        {"cell?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeCell>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"promise?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemePromise>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
        {"weak-pair?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            return p ? scheme_true : scheme_false;
        }}},
    }
);