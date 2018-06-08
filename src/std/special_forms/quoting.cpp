#include "std.h"
#include "eval.h"

static Package package(
    {
        {"quote",      [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                          std::shared_ptr<SchemeFunc>) {
            if(l.size() != 1)
                throw eval_error("quote: one argument required");
            return do_quote(l.front(), context, 0).first;
        }
        },
        {"quasiquote", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                          std::shared_ptr<SchemeFunc>) {
            if(l.size() != 1)
                throw eval_error("quasiquote: one argument required");
            return do_quote(l.front(), context, 1).first;
        }
        },
    }
);