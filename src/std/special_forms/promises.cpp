#include "std.h"

static std::shared_ptr<SchemeObject> make_promise(std::shared_ptr<ASTNode> body, Context &context)
{
    auto f = std::make_shared<SchemeFunc>();
    f->context = context;
    f->body = {*body};
    return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemePromise>(f));
}

static SpecialFormPackage package(
    {
        {"delay",       [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            if(l.size() != 1)
                throw eval_error("delay: one argument required");
            return ExecutionResult(make_promise(l.front(), context));
        }
        },
        {"cons-stream", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            if(l.size() != 2)
                throw eval_error("cons-stream: two arguments required");
            return ExecutionResult(std::dynamic_pointer_cast<SchemeObject>(
                std::make_shared<SchemePair>(l.front()->evaluate(context).force_value(), make_promise(l.back(), context))));
        }
        },
    }
);