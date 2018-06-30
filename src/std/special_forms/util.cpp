#include "std.h"

static SpecialFormPackage package(
    {
        {"set!",            [](const std::list<std::shared_ptr<ASTNode>> &l, const Context &context) {
            if(!(l.size() == 1 || l.size() == 2) || l.front()->type != ast_type_t::NAME)
                throw eval_error("set!: name and value required");
            auto res = l.size() == 2 ? l.back()->evaluate(context).force_value() : nullptr;
            if(!context.assign(l.front()->value, res))
                throw eval_error("set!: unbound variable");
            return ExecutionResult(res ? res : scheme_empty);
        }
        },
        {"the-environment", [](const std::list<std::shared_ptr<ASTNode>> &, const Context &context) {
            return to_object(std::make_shared<SchemeEnvironment>(context));
        }
        },
    }
);