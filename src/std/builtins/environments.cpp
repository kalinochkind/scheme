#include "std.h"

static FunctionPackage package(
    {
        {"environment-has-parent?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            if(!p)
                throw eval_error("environment-has-parent?: an environment required");
            return p->context.locals.size() == 1 ? scheme_false : scheme_true;
        }}},
        {"environment-parent", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto p = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            if(!p || p->context.locals.size() == 1)
                throw eval_error("environment-parent: an environment with parent required");
            Context ctx = p->context;
            ctx.delete_frame();
            return to_object(std::make_shared<SchemeEnvironment>(ctx));
        }}},
        {"environment-bound-names", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto e = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            if(!e)
                throw eval_error("environment-bound-names: an environment required");
            auto p = std::dynamic_pointer_cast<SchemePair>(scheme_nil);
            for(auto i = e->context.locals.back()->rbegin(); i != e->context.locals.back()->rend(); ++i)
            {
                p = std::make_shared<SchemePair>(std::make_shared<SchemeSymbol>(i->first), p);
            }
            return to_object(p);
        }}},
        {"environment-macro-names", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto e = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            if(!e)
                throw eval_error("environment-macro-names: an environment required");
            auto p = std::dynamic_pointer_cast<SchemePair>(scheme_nil);
            for(auto i = e->context.locals.back()->rbegin(); i != e->context.locals.back()->rend(); ++i)
            {
                if(std::dynamic_pointer_cast<SchemeSpecialForm>(i->second))
                    p = std::make_shared<SchemePair>(std::make_shared<SchemeSymbol>(i->first), p);
            }
            return to_object(p);
        }}},
        {"environment-bindings", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto e = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            if(!e)
                throw eval_error("environment-bindings: an environment required");
            auto p = std::dynamic_pointer_cast<SchemePair>(scheme_nil);
            for(auto i = e->context.locals.back()->rbegin(); i != e->context.locals.back()->rend(); ++i)
            {
                std::shared_ptr<SchemePair> el;
                if(i->second)
                    el = std::make_shared<SchemePair>(std::make_shared<SchemeSymbol>(i->first),
                                                      std::make_shared<SchemePair>(i->second, scheme_nil));
                else
                    el = std::make_shared<SchemePair>(std::make_shared<SchemeSymbol>(i->first), scheme_nil);
                p = std::make_shared<SchemePair>(el, p);
            }
            return to_object(p);
        }}},
        {"environment-reference-type", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto e = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            auto s = std::dynamic_pointer_cast<SchemeSymbol>(l.back());
            if(!e || !s)
                throw eval_error("environment-reference-type: environment and symbol required");
            std::shared_ptr<SchemeObject> val;
            if(!e->context.get(s->value, val))
                return to_object(std::make_shared<SchemeSymbol>("unbound"));
            if(!val)
                return to_object(std::make_shared<SchemeSymbol>("unassigned"));
            if(std::dynamic_pointer_cast<SchemeSpecialForm>(val))
                return to_object(std::make_shared<SchemeSymbol>("macro"));
            return to_object(std::make_shared<SchemeSymbol>("normal"));
        }}},
        {"environment-assigned?", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto e = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            auto s = std::dynamic_pointer_cast<SchemeSymbol>(l.back());
            if(!e || !s)
                throw eval_error("environment-assigned?: environment and symbol required");
            std::shared_ptr<SchemeObject> val;
            if(!e->context.get(s->value, val) || std::dynamic_pointer_cast<SchemeSpecialForm>(val))
                throw eval_error("environment-assigned?: symbol is not bound");
            return val ? scheme_true : scheme_false;
        }}},
        {"environment-lookup", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto e = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            auto s = std::dynamic_pointer_cast<SchemeSymbol>(l.back());
            if(!e || !s)
                throw eval_error("environment-lookup: environment and symbol required");
            std::shared_ptr<SchemeObject> val;
            if(!e->context.get(s->value, val) || !val || std::dynamic_pointer_cast<SchemeSpecialForm>(val))
                throw eval_error("environment-lookup: symbol is not assigned");
            return val;
        }}},
        {"environment-lookup-macro", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto e = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            auto s = std::dynamic_pointer_cast<SchemeSymbol>(l.back());
            if(!e || !s)
                throw eval_error("environment-lookup-macro: environment and symbol required");
            std::shared_ptr<SchemeObject> val;
            if(!e->context.get(s->value, val) || !val)
                return scheme_false;
            auto sf = std::dynamic_pointer_cast<SchemeSpecialForm>(val);
            return sf ? sf : scheme_false;
        }}},
        {"environment-assign!", {3, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto e = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            auto s = std::dynamic_pointer_cast<SchemeSymbol>(*next(l.begin()));
            if(!e || !s)
                throw eval_error("environment-assign!: environment, symbol and value required");
            if(!e->context.assign(s->value, l.back()))
                throw eval_error("environment-assign!: unable to assign");
            return l.back();
        }}},
        {"environment-define", {3, 3, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto e = std::dynamic_pointer_cast<SchemeEnvironment>(l.front());
            auto s = std::dynamic_pointer_cast<SchemeSymbol>(*next(l.begin()));
            if(!e || !s)
                throw eval_error("environment-define: environment, symbol and value required");
            e->context.set(s->value, l.back());
            return l.back();
        }}},
    }
);
