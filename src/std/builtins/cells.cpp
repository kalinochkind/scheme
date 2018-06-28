#include "std.h"

static FunctionPackage package(
    {
        {"make-cell", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return to_object(std::make_shared<SchemeCell>(l.front()));
        }}},
        {"cell-contents", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto cp = std::dynamic_pointer_cast<SchemeCell>(l.front());
            if(!cp)
                throw eval_error("cell-contents: a cell required");
            return cp->value;
        }}},
        {"set-cell-contents!", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto cp = std::dynamic_pointer_cast<SchemeCell>(l.front());
            if(!cp)
                throw eval_error("set-cell-contents!: cell and value required");
            cp->value = l.back();
            return scheme_empty;
        }}},
    }
);
