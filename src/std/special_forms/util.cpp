#include "std.h"
#include "eval.h"

static Package package(
    {
        {"apply",           [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc> tail_func) {
            if(l.size() < 2)
                throw eval_error("apply: function and list of arguments required");
            auto f = std::dynamic_pointer_cast<SchemeFunc>(l.front()->evaluate(context));
            auto tail = std::dynamic_pointer_cast<SchemePair>(l.back()->evaluate(context));
            if(!f || !tail)
                throw eval_error("apply: function and list of arguments required");
            std::list<std::shared_ptr<SchemeObject>> args;
            for(auto i = next(l.begin()); next(i) != l.end(); ++i)
                args.push_back((*i)->evaluate(context));
            while(tail && tail != scheme_nil)
            {
                args.push_back(tail->car);
                tail = std::dynamic_pointer_cast<SchemePair>(tail->cdr);
            }
            if(f == tail_func)
                throw tail_call(args);
            return execute_function(f, args);
        }
        },
        {"set!",            [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            if(!(l.size() == 1 || l.size() == 2) || l.front()->type != ast_type_t::NAME)
                throw eval_error("set!: name and value required");
            auto res = l.size() == 2 ? l.back()->evaluate(context) : nullptr;
            context.assign(l.front()->value, res);
            return res ? res : scheme_empty;
        }
        },
        {"the-environment", [](const std::list<std::shared_ptr<ASTNode>> &, Context &context,
                               std::shared_ptr<SchemeFunc>) {
            return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeEnvironment>(context));
        }
        },
    }
);