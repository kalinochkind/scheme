#include <iostream>
#include "std.h"
#include "eval.h"

static Package package(
    {
        {"display", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("display: one argument required");
            std::cout << l.front()->printable();
            return scheme_empty;
        }
        },
    }
);