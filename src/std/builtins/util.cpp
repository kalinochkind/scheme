#include "std.h"

std::chrono::milliseconds get_current_time()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
}

static FunctionPackage package(
    {
        {"runtime", {0, 0, [](const std::list<std::shared_ptr<SchemeObject>> &) {
            return to_object(std::make_shared<SchemeInt>((get_current_time() - start_time).count()));
        }}},
        {"error", {0, -1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::string res = "error: ";
            for(auto i : l)
            {
                res += i->external_repr() + " ";
            }
            throw eval_error(res);
            return scheme_empty;
        }}},
        {"eq?", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return eq_test(l.front(), l.back()) ? scheme_true : scheme_false;
        }}},
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
    }
);
