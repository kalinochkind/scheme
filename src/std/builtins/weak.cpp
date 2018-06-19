#include "std.h"

static FunctionPackage package(
    {
        {"weak-cons",      [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("weak-cons: two arguments required");
            return to_object(std::make_shared<SchemeWeakPair>(l.front(), l.back()));
        }
        },
        {"weak-pair/car?", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("weak-pair/car?: weak pair required");
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-pair/car?: weak pair required");
            return wp->car.expired() ? scheme_false : scheme_true;
        }
        },
        {"weak-car",       [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("weak-car: weak pair required");
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-car: weak pair required");
            auto res = wp->car.lock();
            return res ? res : scheme_false;
        }
        },
        {"weak-cdr",       [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("weak-cdr: weak pair required");
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-cdr: weak pair required");
            return wp->cdr;
        }
        },
        {"weak-set-car!",  [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("weak-set-car!: weak pair and value required");
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-set-car!: weak pair and value required");
            wp->car = l.back();
            return scheme_empty;
        }
        },
        {"weak-set-cdr!",  [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("weak-set-cdr!: weak pair and value required");
            auto wp = std::dynamic_pointer_cast<SchemeWeakPair>(l.front());
            if(!wp)
                throw eval_error("weak-set-cdr!: weak pair and value required");
            wp->cdr = l.back();
            return scheme_empty;
        }
        },
    }
);
