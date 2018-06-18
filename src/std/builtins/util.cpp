#include "std.h"

std::chrono::milliseconds get_current_time()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
}

bool eq_test(std::shared_ptr<SchemeObject> a, std::shared_ptr<SchemeObject> b)
{
    if(a == b)
        return true;
    {
        auto p1 = std::dynamic_pointer_cast<SchemeSymbol>(a);
        auto p2 = std::dynamic_pointer_cast<SchemeSymbol>(b);
        if(p1 && p2 && !p1->uninterned && !p2->uninterned)
            return p1->value == p2->value;
    }
    {
        auto p1 = std::dynamic_pointer_cast<SchemeBuiltinFunc>(a);
        auto p2 = std::dynamic_pointer_cast<SchemeBuiltinFunc>(b);
        if(p1 && p2)
            return p1->name == p2->name;
    }
    {
        auto p1 = std::dynamic_pointer_cast<SchemeInt>(a);
        auto p2 = std::dynamic_pointer_cast<SchemeInt>(b);
        if(p1 && p2)
            return p1->value == p2->value;
    }
    {
        auto p1 = std::dynamic_pointer_cast<SchemeFloat>(a);
        auto p2 = std::dynamic_pointer_cast<SchemeFloat>(b);
        if(p1 && p2)
            return p1->value == p2->value;
    }
    {
        auto p1 = std::dynamic_pointer_cast<SchemeChar>(a);
        auto p2 = std::dynamic_pointer_cast<SchemeChar>(b);
        if(p1 && p2)
            return p1->value == p2->value;
    }
    return false;
}


static FunctionPackage package(
    {
        {"runtime",  [](const std::list<std::shared_ptr<SchemeObject>> &) {
            return to_object(std::make_shared<SchemeInt>((get_current_time() - start_time).count()));
        }
        },
        {"error",    [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::string res = "error: ";
            for(auto i : l)
            {
                res += i->externalRepr() + " ";
            }
            throw eval_error(res);
            return scheme_empty;
        }
        },
        {"cons",     [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("cons: 2 arguments required");
            return to_object(std::make_shared<SchemePair>(l.front(), l.back()));
        }
        },
        {"eq?",      [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("eq?: 2 arguments required");
            return eq_test(l.front(), l.back()) ? scheme_true : scheme_false;
        }
        },
        {"set-car!", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("set-car!: 2 arguments required");
            auto p = std::dynamic_pointer_cast<SchemePair>(l.front());
            if(!p || p == scheme_nil)
                throw eval_error("set-car!: a pair required");
            p->car = l.back();
            return l.front();
        }
        },
        {"set-cdr!", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("set-cdr!: 2 arguments required");
            auto p = std::dynamic_pointer_cast<SchemePair>(l.front());
            if(!p || p == scheme_nil)
                throw eval_error("set-cdr!: a pair required");
            p->cdr = l.back();
            return l.front();
        }
        },
        {"eval",     [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::shared_ptr<SchemeEnvironment> e;
            if(l.size() != 2 || !(e = std::dynamic_pointer_cast<SchemeEnvironment>(l.back())))
                throw eval_error("eval: code and environment required");
            return l.front()->toAST()->evaluate(e->context).force_value();  // tail call in eval?
        }
        },
        {"apply",           [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() < 2)
                throw eval_error("apply: function and list of arguments required");
            auto f = std::dynamic_pointer_cast<SchemeFunc>(l.front());
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
        }
        },
    }
);
