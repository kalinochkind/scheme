#include "std.h"

static FunctionPackage package(
    {
        {"eval", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::shared_ptr<SchemeEnvironment> e;
            if(!(e = std::dynamic_pointer_cast<SchemeEnvironment>(l.back())))
                throw eval_error("eval: code and environment required");
            return l.front()->to_AST()->evaluate(e->context).force_value();  // tail call in eval?
        }}},
        {"apply", {2, -1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto f = std::dynamic_pointer_cast<SchemeProcedure>(l.front());
            auto tail = std::dynamic_pointer_cast<SchemePair>(l.back());
            if(!f || !tail)
                throw eval_error("apply: function and list of arguments required");
            std::list<std::shared_ptr<SchemeObject>> args;
            for(auto i = next(l.begin()); next(i) != l.end(); ++i)
                args.push_back(*i);
            while(tail && tail != scheme_nil)
            {
                args.push_back(tail->car);
                tail = std::dynamic_pointer_cast<SchemePair>(tail->cdr);
            }
            if(!tail)
                throw eval_error("apply: invalid list");
            return ExecutionResult(f, args);
        }}},
        {"procedure-environment", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto f = std::dynamic_pointer_cast<SchemeCompoundProcedure>(l.front());
            if(!f)
                throw eval_error("procedure-environment: compound procedure required");
            return to_object(std::make_shared<SchemeEnvironment>(f->context));
        }}},
        {"procedure-arity", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto f = std::dynamic_pointer_cast<SchemeProcedure>(l.front());
            if(!f)
                throw eval_error("procedure-arity: a procedure required");
            auto low = std::make_shared<SchemeInt>(f->arity.first);
            return to_object(std::make_shared<SchemePair>(low, f->arity.second >= 0 ? std::make_shared<SchemeInt>(
                f->arity.second) : scheme_false));
        }}},
    }
);
