#include "std.h"

static SpecialFormPackage package(
    {
        {"apply",           [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            if(l.size() < 2)
                throw eval_error("apply: function and list of arguments required");
            auto f = std::dynamic_pointer_cast<SchemeFunc>(l.front()->evaluate(context).force_value());
            auto tail = std::dynamic_pointer_cast<SchemePair>(l.back()->evaluate(context).force_value());
            if(!f || !tail)
                throw eval_error("apply: function and list of arguments required");
            std::list<std::shared_ptr<SchemeObject>> args;
            for(auto i = next(l.begin()); next(i) != l.end(); ++i)
                args.push_back((*i)->evaluate(context).force_value());
            while(tail && tail != scheme_nil)
            {
                args.push_back(tail->car);
                tail = std::dynamic_pointer_cast<SchemePair>(tail->cdr);
            }
            return ExecutionResult(f, args);
        }
        },
        {"set!",            [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            if(!(l.size() == 1 || l.size() == 2) || l.front()->type != ast_type_t::NAME)
                throw eval_error("set!: name and value required");
            auto res = l.size() == 2 ? l.back()->evaluate(context).force_value() : nullptr;
            context.assign(l.front()->value, res);
            return ExecutionResult(res ? res : scheme_empty);
        }
        },
        {"the-environment", [](const std::list<std::shared_ptr<ASTNode>> &, Context &context) {
            return ExecutionResult(
                std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeEnvironment>(context)));
        }
        },
    }
);