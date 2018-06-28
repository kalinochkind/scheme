#include <iostream>
#include "parser.h"
#include "std.h"

static FunctionPackage package(
    {
        {"display", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::cout << l.front()->printable();
            return scheme_empty;
        }}},
        {"read", {0, 0, [](const std::list<std::shared_ptr<SchemeObject>> &) {
            Context dummy;
            auto x = read_object(std::cin);
            if(x.result == parse_result_t::ERROR)
                throw eval_error("read: parse error: " + x.error);
            if(x.result == parse_result_t::END)
                return to_object(std::make_shared<SchemeSymbol>("eof"));
            return do_quote(x.node, dummy, 0).first;
        }}},
    }
);