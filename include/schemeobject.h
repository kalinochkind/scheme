#ifndef SCHEME_SCHEMEOBJECT_H
#define SCHEME_SCHEMEOBJECT_H

#include <string>
#include <list>
#include <memory>
#include <map>

struct SchemeObject;
struct SchemeFunc;

using context_map_t = std::map<std::string, std::shared_ptr<SchemeObject>>;

enum class ast_type_t
{
    LIST, NAME, INT, FLOAT, STRING
};

struct Context
{
    std::list<std::shared_ptr<context_map_t>> locals;

    Context(): locals() {};

    std::shared_ptr<SchemeObject> get(const std::string &name) const;

    void set(const std::string &name, std::shared_ptr<SchemeObject> value);

    void assign(const std::string &name, std::shared_ptr<SchemeObject> value);

    void newFrame();

    void delFrame();
};

struct ASTNode
{
    ast_type_t type;
    std::string value;
    std::list<std::shared_ptr<ASTNode>> list;

    ASTNode(): type(ast_type_t::LIST), value(), list() {};

    std::shared_ptr<SchemeObject> evaluate(Context &context, std::shared_ptr<SchemeFunc> tail_func = nullptr);

    std::string toString() const;
};


struct SchemeObject
{
    virtual ~SchemeObject() {};

    virtual std::string toString() const = 0;

    virtual bool toBool() const
    {
        return true;
    }
};

struct SchemeInt: public SchemeObject
{
    long long value;

    SchemeInt(long long a): value(a) {};

    std::string toString() const override;
};

struct SchemeFloat: public SchemeObject
{
    double value;

    SchemeFloat(double a): value(a) {};

    std::string toString() const override;
};

struct SchemeFunc: public SchemeObject
{
    std::list<std::string> params;
    std::list<ASTNode> body;
    Context context;
    bool arglist;

    SchemeFunc(): params(), body(), context(), arglist(false) {};

    std::string toString() const override;
};

struct SchemeBuiltinFunc: public SchemeFunc
{
    std::string name;

    SchemeBuiltinFunc(std::string s): name(s) {};

    std::string toString() const override;
};

struct SchemeBool: public SchemeObject
{
    bool value;

    SchemeBool(bool a): value(a) {};

    std::string toString() const override;

    bool toBool() const override
    {
        return value;
    }
};

struct SchemeString: public SchemeObject
{
    std::string value;

    SchemeString(std::string s): value(s) {};

    std::string toString() const override;
};

struct SchemePair: public SchemeObject
{
    std::shared_ptr<SchemeObject> car, cdr;

    SchemePair(std::shared_ptr<SchemeObject> a, std::shared_ptr<SchemeObject> b): car(a), cdr(b) {};

    std::string toString() const override;
};

struct SchemeName: public SchemeObject
{
    std::string value;

    SchemeName(std::string s): value(s) {};

    std::string toString() const override;
};

struct SchemePromise: public SchemeObject
{
    std::shared_ptr<SchemeObject> value;
    std::shared_ptr<SchemeFunc> func;

    SchemePromise(std::shared_ptr<SchemeFunc> f): value(nullptr), func(f) {};

    std::string toString() const override;

    std::shared_ptr<SchemeObject> force();
};

#endif