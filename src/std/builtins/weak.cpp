#include "std.h"

static FunctionPackage package(
    {
        {"weak-cons", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return to_object(std::make_shared<SchemeWeakPair>(l.front(), l.back()));
        }}},
        {"weak-pair/car?", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-pair/car?: weak pair required");
            return wp->car.expired() ? scheme_false : scheme_true;
        }}},
        {"weak-car", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-car: weak pair required");
            auto res = wp->car.lock();
            return res ? res : scheme_false;
        }}},
        {"weak-cdr", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-cdr: weak pair required");
            return wp->cdr;
        }}},
        {"weak-set-car!", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-set-car!: weak pair and value required");
            wp->car = l.back();
            return scheme_empty;
        }}},
        {"weak-set-cdr!", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-set-cdr!: weak pair and value required");
            wp->cdr = l.back();
            return scheme_empty;
        }}},
    }
);
