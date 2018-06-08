#include "std.h"


std::pair<std::shared_ptr<SchemeObject>, bool>
do_quote(std::shared_ptr<ASTNode> node, Context &context, int quasi_level)
{
    if(node->type == ast_type_t::STRING || node->type == ast_type_t::INT || node->type == ast_type_t::FLOAT ||
       node->type == ast_type_t::BOOL)
    {
        return {node->evaluate(context).force_value(), false};
    }
    if(node->type == ast_type_t::NAME)
    {
        return {std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeName>(node->value)), false};
    }
    auto lst = scheme_nil;
    if(quasi_level > 0 && node->list.size() && node->list.front()->type == ast_type_t::NAME)
    {
        auto head = node->list.front()->value;
        if(head == "unquote")
        {
            if(quasi_level == 1)
            {
                if(node->list.size() != 2)
                    throw eval_error("unquote: one argument required");
                return {node->list.back()->evaluate(context).force_value(), false};
            }
            --quasi_level;
        }
        else if(head == "unquote-splicing")
        {
            if(quasi_level == 1)
            {
                if(node->list.size() != 2)
                    throw eval_error("unquote-splicing: one argument required");
                return {node->list.back()->evaluate(context).force_value(), true};
            }
            --quasi_level;
        }
        else if(head == "quasiquote")
            ++quasi_level;
    }
    for(auto i = node->list.rbegin(); i != node->list.rend(); ++i)
    {
        auto quoted = do_quote(*i, context, quasi_level);
        if(quoted.second)
        {
            std::list<std::shared_ptr<SchemeObject>> to_insert;
            while(quoted.first != scheme_nil)
            {
                auto sublst = std::dynamic_pointer_cast<SchemePair>(quoted.first);
                if(!sublst)
                    throw eval_error("unquote-slicing: the expression did not evaluate to a list");
                to_insert.push_back(sublst->car);
                quoted.first = sublst->cdr;
            }
            for(auto j = to_insert.rbegin(); j != to_insert.rend(); ++j)
                lst = std::make_shared<SchemePair>(*j, lst);
        }
        else
            lst = std::make_shared<SchemePair>(quoted.first, lst);
    }
    return {std::dynamic_pointer_cast<SchemeObject>(lst), false};
}