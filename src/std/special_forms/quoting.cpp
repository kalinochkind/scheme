#include "std.h"

static SpecialFormPackage package(
    {
        {"quote",      [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(l.size() != 1)
                throw eval_error("quote: one argument required");
            return ExecutionResult(do_quote(l.front(), context, 0).first);
        }
        },
        {"quasiquote", [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(l.size() != 1)
                throw eval_error("quasiquote: one argument required");
            return ExecutionResult(do_quote(l.front(), context, 1).first);
        }
        },
    }
);