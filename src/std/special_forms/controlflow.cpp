#include "std.h"
#include "eval.h"

static Package package(
    {
        {"do",    [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                     std::shared_ptr<SchemeFunc> tail_func) {
            if(l.size() < 2 || l.front()->type != ast_type_t::LIST || (*next(l.begin()))->type != ast_type_t::LIST)
                throw eval_error("do: lists required");
            context_map_t new_frame;
            auto vars = l.front()->list;
            auto cond = *next(l.begin());
            if(cond->list.empty())
                throw eval_error("do: empty condition");
            for(auto &&i : vars)
            {
                if(i->type != ast_type_t::LIST || i->list.empty() || i->list.size() > 3 ||
                   i->list.front()->type != ast_type_t::NAME)
                    throw eval_error("do: bad variable list");
                if(new_frame.count(i->list.front()->value))
                    throw eval_error("do: duplicate variables");
                new_frame[i->list.front()->value] = i->list.empty() ? nullptr : (*next(i->list.begin()))->evaluate(
                    context);
            }
            Context local_context = context;
            local_context.newFrame(new_frame);
            while(true)
            {
                auto cond_value = cond->list.front()->evaluate(local_context);
                if(cond_value->toBool())
                {
                    for(auto it = next(cond->list.begin()); it != cond->list.end(); ++it)
                    {
                        cond_value = (*it)->evaluate(local_context, next(it) == cond->list.end() ? tail_func : nullptr);
                    }
                    return cond_value;
                }
                for(auto it = next(next(l.begin())); it != l.end(); ++it)
                {
                    (*it)->evaluate(local_context);
                }
                new_frame.clear();
                for(auto &&i : vars)
                {
                    if(i->list.size() < 3)
                        new_frame[i->list.front()->value] = local_context.get(i->list.front()->value);
                    else
                        new_frame[i->list.front()->value] = i->list.back()->evaluate(local_context);
                }
                local_context.delFrame();
                local_context.newFrame(new_frame);
            }
        }
        },
        {"cond",  [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                     std::shared_ptr<SchemeFunc> tail_func) {
            for(auto branch : l)
            {
                if(branch->type != ast_type_t::LIST || branch->list.size() < 1)
                    throw eval_error("cond: non-empty lists required");
                std::shared_ptr<SchemeObject> br;
                if((branch->list.front()->type == ast_type_t::NAME && branch->list.front()->value == "else") ||
                   (br = branch->list.front()->evaluate(context))->toBool())
                {
                    if(branch->list.size() == 1)
                    {
                        if(branch->list.front()->type == ast_type_t::NAME && branch->list.front()->value == "else")
                            throw eval_error("cond: else branch has no expressions");
                        return br;
                    }
                    if(branch->list.size() == 3 && ((*next(branch->list.begin()))->type == ast_type_t::NAME &&
                                                    (*next(branch->list.begin()))->value == "=>"))
                    {
                        auto func = std::dynamic_pointer_cast<SchemeFunc>(branch->list.back()->evaluate(context));
                        if(!func)
                            throw eval_error("cond: => requires a function");
                        std::list<std::shared_ptr<SchemeObject>> args{br};
                        if(func == tail_func)
                            throw tail_call(args);
                        return execute_function(func, args);
                    }
                    std::shared_ptr<SchemeObject> res;
                    for(auto i = next(branch->list.begin()); i != branch->list.end(); ++i)
                    {
                        res = (*i)->evaluate(context, std::next(i) == branch->list.end() ? tail_func : nullptr);
                    }
                    return res;
                }
            }
            return scheme_empty;
        }
        },
        {"if",    [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                     std::shared_ptr<SchemeFunc> tail_func) {
            if(l.size() > 3 || l.size() < 2)
                throw eval_error("if: 2 or 3 arguments required");
            if(l.front()->evaluate(context)->toBool())
                return (*next(l.begin()))->evaluate(context, tail_func);
            else if(l.size() == 3)
                return (*next(next(l.begin())))->evaluate(context, tail_func);
            else
                return scheme_empty;
        }
        },
        {"case",  [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                     std::shared_ptr<SchemeFunc> tail_func) {
            if(l.empty())
                throw eval_error("case: argument required");
            auto value = l.front()->evaluate(context);
            for(auto it = next(l.begin()); it != l.end(); ++it)
            {
                bool passed = false;
                if((*it)->type != ast_type_t::LIST || (*it)->list.empty())
                    throw eval_error("case: non-empty lists required");
                auto values = (*it)->list.front();
                if(values->type == ast_type_t::NAME && values->value == "else")
                    passed = true;
                else if(values->type != ast_type_t::LIST)
                {
                    throw eval_error("case: lists required");
                }
                else
                {
                    for(auto &&j : values->list)
                    {
                        if(eq_test(value, do_quote(j, context, 0).first))
                        {
                            passed = true;
                            break;
                        }
                    }
                }
                if(passed)
                {
                    auto result = scheme_empty;
                    for(auto j = next((*it)->list.begin()); j != (*it)->list.end(); ++j)
                    {
                        result = (*j)->evaluate(context, next(j) == (*it)->list.end() ? tail_func : nullptr);
                    }
                    return result;
                }
            }
            return scheme_empty;
        }
        },
        {"and",   [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                     std::shared_ptr<SchemeFunc> tail_func) {
            if(l.empty())
                return scheme_true;
            std::shared_ptr<SchemeObject> res;
            for(auto i = l.begin(); i != l.end(); ++i)
            {
                if(!(res = (*i)->evaluate(context, next(i) == l.end() ? tail_func : nullptr))->toBool())
                    return scheme_false;
            }
            return res;
        }
        },
        {"or",    [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                     std::shared_ptr<SchemeFunc> tail_func) {
            std::shared_ptr<SchemeObject> res;
            for(auto i = l.begin(); i != l.end(); ++i)
            {
                if((res = (*i)->evaluate(context, next(i) == l.end() ? tail_func : nullptr))->toBool())
                    return res;
            }
            return scheme_false;
        }
        },
        {"begin", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context,
                     std::shared_ptr<SchemeFunc> tail_func) {
            if(!l.size())
                throw eval_error("begin: at least one argument required");
            std::shared_ptr<SchemeObject> res;
            for(auto i = l.begin(); i != l.end(); ++i)
            {
                res = (*i)->evaluate(context, next(i) == l.end() ? tail_func : nullptr);
            }
            return res;
        }
        },
    }
);