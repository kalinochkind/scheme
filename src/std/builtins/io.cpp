#include <iostream>
#include "parser.h"
#include "std.h"

static FunctionPackage package(
    {
        {"display", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("display: one argument required");
            std::cout << l.front()->printable();
            return scheme_empty;
        }
        },
        {"read",    [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size())
                throw eval_error("read: no arguments required");
            Context dummy;
            auto x = readObject(std::cin);
            if(x.result == parse_result_t::ERROR)
                throw eval_error("read: parse error: " + x.error);
            if(x.result == parse_result_t::END)
                return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeSymbol>("eof"));
            return do_quote(x.node, dummy, 0).first;
        }
        },
    }
);