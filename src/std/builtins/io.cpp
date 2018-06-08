#include <iostream>
#include "parser.h"
#include "std.h"

static Package package(
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
            try
            {
                return do_quote(readObject(std::cin), dummy, 0).first;
            }
            catch(end_of_input)
            {
                return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeName>("eof"));
            }
        }
        },
    }
);