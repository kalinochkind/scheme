#include "std.h"

static FunctionPackage package(
    {
        {"make-cell",     [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("make-cell: one argument required");
            return to_object(std::make_shared<SchemeCell>(l.front()));
        }
        },
        {"cell-contents", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("cell-contents: a cell required");
            auto cp = std::dynamic_pointer_cast<SchemeCell>(l.front());
            if(!cp)
                throw eval_error("cell-contents: a cell required");
            return cp->value;
        }
        },
        {"set-cell-contents!", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("set-cell-contents!: cell and value required");
            auto cp = std::dynamic_pointer_cast<SchemeCell>(l.front());
            if(!cp)
                throw eval_error("set-cell-contents!: cell and value required");
            cp->value = l.back();
            return scheme_empty;
        }
        },
    }
);
