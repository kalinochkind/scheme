#include "std.h"

static SpecialFormPackage package(
    {
        {"do", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
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
                    context).force_value();
            }
            Context local_context = context;
            local_context.new_frame(new_frame);
            while(true)
            {
                auto cond_value = cond->list.front()->evaluate(local_context);
                if(cond_value.force_value()->to_bool())
                {
                    for(auto it = next(cond->list.begin()); it != cond->list.end(); ++it)
                    {
                        cond_value.force_value();
                        cond_value = (*it)->evaluate(local_context);
                    }
                    return cond_value;
                }
                for(auto it = next(next(l.begin())); it != l.end(); ++it)
                {
                    (*it)->evaluate(local_context).force_value();
                }
                new_frame.clear();
                for(auto &&i : vars)
                {
                    if(i->list.size() < 3)
                    {
                        if(!local_context.get(i->list.front()->value, new_frame[i->list.front()->value]))
                            throw eval_error("do: unbound variable");
                    }
                    else
                        new_frame[i->list.front()->value] = i->list.back()->evaluate(local_context).force_value();
                }
                local_context.delete_frame();
                local_context.new_frame(new_frame);
            }
        }
        },
        {"cond", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            for(auto branch : l)
            {
                if(branch->type != ast_type_t::LIST || branch->list.size() < 1)
                    throw eval_error("cond: non-empty lists required");
                ExecutionResult br;
                if((branch->list.front()->type == ast_type_t::NAME && branch->list.front()->value == "else") ||
                   (br = branch->list.front()->evaluate(context)).force_value()->to_bool())
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
                        auto func = std::dynamic_pointer_cast<SchemeCompoundProcedure>(
                            branch->list.back()->evaluate(context).force_value());
                        if(!func)
                            throw eval_error("cond: => requires a function");
                        return ExecutionResult(func, {br.value});
                    }
                    ExecutionResult res;
                    for(auto i = next(branch->list.begin()); i != branch->list.end(); ++i)
                    {
                        res.force_value();
                        res = (*i)->evaluate(context);
                    }
                    return res;
                }
            }
            return ExecutionResult(scheme_empty);
        }
        },
        {"if", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            if(l.size() > 3 || l.size() < 2)
                throw eval_error("if: 2 or 3 arguments required");
            if(l.front()->evaluate(context).force_value()->to_bool())
                return (*next(l.begin()))->evaluate(context);
            else if(l.size() == 3)
                return (*next(next(l.begin())))->evaluate(context);
            else
                return ExecutionResult(scheme_empty);
        }
        },
        {"case", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            if(l.empty())
                throw eval_error("case: argument required");
            auto value = l.front()->evaluate(context).force_value();
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
                    auto result = ExecutionResult(scheme_empty);
                    for(auto j = next((*it)->list.begin()); j != (*it)->list.end(); ++j)
                    {
                        result.force_value();
                        result = (*j)->evaluate(context);
                    }
                    return result;
                }
            }
            return ExecutionResult(scheme_empty);
        }
        },
        {"and", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            if(l.empty())
                return ExecutionResult(scheme_true);
            for(auto i = l.begin(); next(i) != l.end(); ++i)
            {
                auto res = (*i)->evaluate(context).force_value();
                if(!res->to_bool())
                    return ExecutionResult(res);
            }
            return l.back()->evaluate(context);
        }
        },
        {"or", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            if(l.empty())
                return ExecutionResult(scheme_false);
            for(auto i = l.begin(); next(i) != l.end(); ++i)
            {
                auto res = (*i)->evaluate(context).force_value();
                if(res->to_bool())
                    return ExecutionResult(res);
            }
            return l.back()->evaluate(context);
        }
        },
        {"begin", [](const std::list<std::shared_ptr<ASTNode>> &l, Context &context) {
            if(!l.size())
                throw eval_error("begin: at least one argument required");
            ExecutionResult res;
            for(auto i = l.begin(); i != l.end(); ++i)
            {
                res.force_value();
                res = (*i)->evaluate(context);
            }
            return res;
        }
        },
    }
);