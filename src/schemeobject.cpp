#include <set>
#include "std.h"
#include "char.h"


static std::string quote_string(const std::string &s)
{
    std::string ans = "\"";
    for(char c : s)
    {
        switch(c)
        {
            case '"':
            case '\\':
                ans.push_back('\\');
                ans.push_back(c);
                break;
            case '\n':
                ans.push_back('\\');
                ans.push_back('n');
                break;
            case '\t':
                ans.push_back('\\');
                ans.push_back('t');
                break;
            case '\f':
                ans.push_back('\\');
                ans.push_back('f');
                break;
            default:
                ans.push_back(c);
        }
    }
    return ans + "\"";
}

std::shared_ptr<ASTNode> SchemeObject::to_AST() const
{
    throw eval_error("Cannot evaluate " + external_repr());
}

bool SchemeObject::is_eq(const std::shared_ptr<SchemeObject> &) const
{
    return false;
}

std::string SchemeInt::external_repr() const
{
    return std::to_string(value);
}

std::shared_ptr<ASTNode> SchemeInt::to_AST() const
{
    return std::make_shared<ASTNode>(ast_type_t::INT, external_repr());
}

bool SchemeInt::is_eq(const std::shared_ptr<SchemeObject> &a) const
{
    auto p = std::dynamic_pointer_cast<SchemeInt>(a);
    return p && value == p->value;
}

std::string SchemeFloat::external_repr() const
{
    return std::to_string(value);
}

std::shared_ptr<ASTNode> SchemeFloat::to_AST() const
{
    return std::make_shared<ASTNode>(ast_type_t::FLOAT, external_repr());
}

bool SchemeFloat::is_eq(const std::shared_ptr<SchemeObject> &a) const
{
    auto p = std::dynamic_pointer_cast<SchemeFloat>(a);
    return p && value == p->value;
}

std::string SchemeCompoundProcedure::external_repr() const
{
    return "<compound-procedure" + (name.empty() ? "" : (" " + name)) + ">";
}

std::string SchemePrimitiveProcedure::external_repr() const
{
    return "<primitive-procedure " + name + ">";
}

bool SchemePrimitiveProcedure::is_eq(const std::shared_ptr<SchemeObject> &a) const
{
    auto p = std::dynamic_pointer_cast<SchemePrimitiveProcedure>(a);
    return p && name == p->name;
}

std::string SchemeSpecialForm::external_repr() const
{
    return "<special form " + name + ">";
}

std::string SchemeBool::external_repr() const
{
    return value ? "#t" : "#f";
}

std::shared_ptr<ASTNode> SchemeBool::to_AST() const
{
    return std::make_shared<ASTNode>(ast_type_t::BOOL, value ? "t" : "f");
}

std::string SchemeString::external_repr() const
{
    return quote_string(value);
}

std::shared_ptr<ASTNode> SchemeString::to_AST() const
{
    return std::make_shared<ASTNode>(ast_type_t::STRING, value);
}

std::string SchemeString::printable() const
{
    return value;
}

std::string SchemeChar::external_repr() const
{
    return std::string("#\\") + char_to_char_name(value);
}

std::shared_ptr<ASTNode> SchemeChar::to_AST() const
{
    return std::make_shared<ASTNode>(ast_type_t::CHAR, normalize_char_name(char_to_char_name(value)));
}

std::string SchemeChar::printable() const
{
    return std::string(1, value);
}

bool SchemeChar::is_eq(const std::shared_ptr<SchemeObject> &a) const
{
    auto p = std::dynamic_pointer_cast<SchemeChar>(a);
    return p && value == p->value;
}

std::string SchemePair::external_repr() const
{
    if(!car)
        return "()";
    auto pcar = std::dynamic_pointer_cast<SchemeSymbol>(car);
    if(pcar)
    {
        std::string prefix = "";
        if(pcar->value == "quote")
            prefix = "'";
        else if(pcar->value == "quasiquote")
            prefix = "`";
        else if(pcar->value == "unquote")
            prefix = ",";
        else if(pcar->value == "unquote-splicing")
            prefix = ",@";
        if(prefix.length())
        {
            auto pcdr = std::dynamic_pointer_cast<SchemePair>(cdr);
            if(pcdr->cdr == scheme_nil)
                return prefix + pcdr->car->external_repr();
        }
    }
    std::string res = "(" + car->external_repr();
    std::shared_ptr<SchemeObject> p = cdr;
    std::shared_ptr<SchemePair> pp;
    std::set<const SchemePair *> visited{this};
    while((pp = std::dynamic_pointer_cast<SchemePair>(p)))
    {
        if(p == scheme_nil)
            return res + ")";
        res += " " + pp->car->external_repr();
        if(visited.count(pp.get()))
            return res + " ...)";
        p = pp->cdr;
        visited.insert(pp.get());
    }
    return res + " . " + p->external_repr() + ")";
}

std::shared_ptr<ASTNode> SchemePair::to_AST() const
{
    auto a = std::make_shared<ASTNode>();
    const SchemePair *p(this);
    while(p && p != scheme_nil.get())
    {
        a->list.push_back(p->car->to_AST());
        p = dynamic_cast<const SchemePair*>(p->cdr.get());
    }
    if(!p)
    {
        return SchemeObject::to_AST();
    }
    return a;
}

long long SchemePair::list_length() const
{
    const SchemePair *p = this;
    std::set<const SchemePair*> visited{p};
    for(long long ans=0;;++ans)
    {
        if(!p)
            return -1;
        if(p == scheme_nil.get())
            return ans;
        p = std::dynamic_pointer_cast<SchemePair>(p->cdr).get();
        if(visited.count(p))
            return -2;
        visited.insert(p);
    }
}

std::string SchemeWeakPair::external_repr() const
{
    return "<weak-pair>";
}

std::string SchemeCell::external_repr() const
{
    return "<cell>";
}


std::shared_ptr<SchemeVector> SchemeVector::from_list(std::shared_ptr<SchemePair> list)
{
    auto v = std::make_shared<SchemeVector>();
    std::set<const SchemePair *> visited{list.get()};
    while(list && list != scheme_nil)
    {
        v->vec.push_back(list->car);
        list = std::dynamic_pointer_cast<SchemePair>(list->cdr);
        if(visited.count(list.get()))
            throw eval_error("Vector initialization failed: not a list");
        visited.insert(list.get());
    }
    if(!list)
        throw eval_error("Vector initialization failed: not a list");
    return v;
}

std::shared_ptr<SchemePair> SchemeVector::to_list() const
{
    auto l = std::dynamic_pointer_cast<SchemePair>(scheme_nil);
    for(auto it=vec.rbegin(); it != vec.rend(); ++it)
    {
        l = std::make_shared<SchemePair>(*it, l);
    }
    return l;
}

std::string SchemeVector::external_repr() const
{
    return "#" + to_list()->external_repr();
}

std::shared_ptr<ASTNode> SchemeVector::to_AST() const
{
    auto a = to_list()->to_AST();
    a->type = ast_type_t::VECTOR;
    return a;
}

long long SchemeSymbol::uninterned_counter = 0;

std::string SchemeSymbol::external_repr() const
{
    if(uninterned)
        return "<uninterned-symbol " + value + ">";
    return value;
}

std::shared_ptr<ASTNode> SchemeSymbol::to_AST() const
{
    if(uninterned)
        return SchemeObject::to_AST();
    return std::make_shared<ASTNode>(ast_type_t::NAME, value);
}

bool SchemeSymbol::is_eq(const std::shared_ptr<SchemeObject> &a) const
{
    auto p = std::dynamic_pointer_cast<SchemeSymbol>(a);
    return p && value == p->value && !uninterned && !p->uninterned;
}

std::string SchemePromise::external_repr() const
{
    return "<promise>";
}

std::shared_ptr<SchemeObject> SchemePromise::force()
{
    if(!value)
    {
        value = func->execute({}).force_value();
        func.reset();
    }
    return value;
}

std::string SchemeEnvironment::external_repr() const
{
    return "<environment>";
}

bool Context::get(const std::string &name, std::shared_ptr<SchemeObject> &res) const
{
    for(auto i = locals.rbegin(); i != locals.rend(); ++i)
    {
        if((*i)->count(name))
        {
            res = (**i)[name];
            return true;
        }
    }
    return false;
}

void Context::set(const std::string &name, std::shared_ptr<SchemeObject> value)
{
    (*locals.back())[name] = value;
}

bool Context::assign(const std::string &name, std::shared_ptr<SchemeObject> value)
{
    for(auto i = locals.rbegin(); i != locals.rend(); ++i)
    {
        if((*i)->count(name))
        {
            (**i)[name] = value;
            return true;
        }
    }
    return false;
}

void Context::new_frame()
{
    locals.push_back(std::make_shared<context_map_t>());
}

void Context::new_frame(const context_map_t &vars)
{
    locals.push_back(std::make_shared<context_map_t>(vars));
}

void Context::delete_frame()
{
    locals.pop_back();
}