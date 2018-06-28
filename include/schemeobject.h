#ifndef SCHEME_SCHEMEOBJECT_H
#define SCHEME_SCHEMEOBJECT_H

#include <string>
#include <list>
#include <memory>
#include <map>
#include <chrono>
#include <vector>


struct SchemeObject;
struct SchemeProcedure;

using Arity = std::pair<long long, long long>;

class eval_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

extern std::chrono::milliseconds start_time;


struct TailContext
{
    std::shared_ptr<SchemeProcedure> func;
    std::list<std::shared_ptr<SchemeObject>> args;

    TailContext(const std::shared_ptr<SchemeProcedure> &func, const std::list<std::shared_ptr<SchemeObject>> &args) :
        func(func), args(args)
    {}
};


enum class execution_result_t
{
    VALUE, TAIL_CALL, ERROR
};

struct ExecutionResult
{
    execution_result_t type;
    std::shared_ptr<SchemeObject> value;
    std::unique_ptr<TailContext> tail_context;

    ExecutionResult() : type(execution_result_t::VALUE), value(nullptr), tail_context(nullptr)
    {}

    ExecutionResult(const std::shared_ptr<SchemeObject> &value) : type(execution_result_t::VALUE),
                                                                  value(value),
                                                                  tail_context(nullptr)
    {};

    ExecutionResult(const std::shared_ptr<SchemeProcedure> &func,
                    const std::list<std::shared_ptr<SchemeObject>> &args) :
        type(execution_result_t::TAIL_CALL), value(nullptr), tail_context(std::make_unique<TailContext>(func, args))
    {};

    std::shared_ptr<SchemeObject> force_value();

    /* explicit ExecutionResult(const std::string &error) : type(execution_result_t::ERROR),
                                                          value(std::make_shared<SchemeString>(error)),
                                                          tail_context(nullptr)
     {};*/
};


using context_map_t = std::map<std::string, std::shared_ptr<SchemeObject>>;

enum class ast_type_t
{
    LIST, NAME, INT, FLOAT, STRING, BOOL, CHAR, VECTOR
};

struct Context
{
    std::list<std::shared_ptr<context_map_t>> locals;

    Context() : locals()
    {};

    bool get(const std::string &name, std::shared_ptr<SchemeObject> &res) const;

    void set(const std::string &name, std::shared_ptr<SchemeObject> value);

    bool assign(const std::string &name, std::shared_ptr<SchemeObject> value);

    void new_frame();

    void new_frame(const context_map_t &vars);

    void delete_frame();
};

struct ASTNode
{
    ast_type_t type;
    std::string value;
    std::list<std::shared_ptr<ASTNode>> list;

    ASTNode() : type(ast_type_t::LIST), value(), list()
    {};

    ASTNode(ast_type_t type, const std::string &value) : type(type), value(value), list()
    {};

    ExecutionResult evaluate(Context &context) const;

};


struct SchemeObject
{
    virtual ~SchemeObject()
    {};

    virtual std::string external_repr() const = 0;

    virtual std::string printable() const
    {
        return external_repr();
    }

    virtual bool to_bool() const
    {
        return true;
    }

    virtual std::shared_ptr<ASTNode> to_AST() const;

    virtual bool is_eq(const std::shared_ptr<SchemeObject> &) const;

};

struct SchemeInt : public SchemeObject
{
    long long value;

    SchemeInt(long long a) : value(a)
    {};

    std::string external_repr() const override;

    std::shared_ptr<ASTNode> to_AST() const override;

    bool is_eq(const std::shared_ptr<SchemeObject> &) const override;
};

struct SchemeFloat : public SchemeObject
{
    double value;

    SchemeFloat(double a) : value(a)
    {};

    std::string external_repr() const override;

    std::shared_ptr<ASTNode> to_AST() const override;

    bool is_eq(const std::shared_ptr<SchemeObject> &) const override;
};

struct SchemeSpecialForm: public SchemeObject
{
    std::string name;

    explicit SchemeSpecialForm(const std::string &name): name(name)
    {}

    std::string external_repr() const override;

    ExecutionResult execute(std::list<std::shared_ptr<ASTNode>>, Context &);
};

struct SchemeProcedure: public SchemeObject
{
    std::string name{""};
    Arity arity{0, 0};

    ExecutionResult execute(const std::list<std::shared_ptr<SchemeObject>> &) const;

    virtual ExecutionResult _run(const std::list<std::shared_ptr<SchemeObject>> &) const = 0;
};


struct SchemeCompoundProcedure: public SchemeProcedure
{
    std::list<std::string> params;
    std::list<ASTNode> body;
    Context context;

    explicit SchemeCompoundProcedure(const std::string &name_ = "")
    {
        name = name_;
    }

    std::string external_repr() const override;

    ExecutionResult _run(const std::list<std::shared_ptr<SchemeObject>> &) const override;
};

struct SchemePrimitiveProcedure: public SchemeProcedure
{
    SchemePrimitiveProcedure(const std::string &name_, long long minargs, long long maxargs)
    {
        name = name_;
        arity.first = minargs;
        arity.second = maxargs;
    }

    std::string external_repr() const override;

    ExecutionResult _run(const std::list<std::shared_ptr<SchemeObject>> &) const override;
};


struct SchemeBool : public SchemeObject
{
    bool value;

    SchemeBool(bool a) : value(a)
    {};

    std::string external_repr() const override;

    std::shared_ptr<ASTNode> to_AST() const override;

    bool to_bool() const override
    {
        return value;
    }
};

struct SchemeString : public SchemeObject
{
    std::string value;

    SchemeString(std::string s) : value(s)
    {};

    std::string external_repr() const override;

    std::shared_ptr<ASTNode> to_AST() const override;

    std::string printable() const override;
};

struct SchemeChar : public SchemeObject
{
    unsigned char value;

    SchemeChar(unsigned char c) : value(c)
    {};

    std::string external_repr() const override;

    std::shared_ptr<ASTNode> to_AST() const override;

    std::string printable() const override;

    bool is_eq(const std::shared_ptr<SchemeObject> &) const override;
};

struct SchemePair : public SchemeObject
{
    std::shared_ptr<SchemeObject> car, cdr;

    SchemePair(std::shared_ptr<SchemeObject> a, std::shared_ptr<SchemeObject> b) : car(std::move(a)), cdr(std::move(b))
    {};

    std::string external_repr() const override;

    std::shared_ptr<ASTNode> to_AST() const override;

    long long list_length() const;
};

struct SchemeWeakPair : public SchemeObject
{
    std::weak_ptr<SchemeObject> car;
    std::shared_ptr<SchemeObject> cdr;

    SchemeWeakPair(const std::shared_ptr<SchemeObject> &a, std::shared_ptr<SchemeObject> b) : car(a), cdr(std::move(b))
    {};

    std::string external_repr() const override;

};

struct SchemeCell : public SchemeObject
{
    std::shared_ptr<SchemeObject> value;

    SchemeCell(std::shared_ptr<SchemeObject> a) : value(std::move(a))
    {};

    std::string external_repr() const override;
};

struct SchemeVector : public SchemeObject
{
    std::vector<std::shared_ptr<SchemeObject>> vec;

    SchemeVector() : vec()
    {};

    static std::shared_ptr<SchemeVector> from_list(std::shared_ptr<SchemePair> list);

    std::shared_ptr<SchemePair> to_list() const;

    std::string external_repr() const override;

    std::shared_ptr<ASTNode> to_AST() const override;

};

struct SchemeSymbol : public SchemeObject
{
    std::string value;
    bool uninterned{false};
    static long long uninterned_counter;

    SchemeSymbol(std::string s) : value(s)
    {};

    std::string external_repr() const override;

    std::shared_ptr<ASTNode> to_AST() const override;

    bool is_eq(const std::shared_ptr<SchemeObject> &) const override;
};

struct SchemePromise : public SchemeObject
{
    std::shared_ptr<SchemeObject> value;
    std::shared_ptr<SchemeCompoundProcedure> func;

    SchemePromise(std::shared_ptr<SchemeCompoundProcedure> f) : value(nullptr), func(f)
    {};

    std::string external_repr() const override;

    std::shared_ptr<SchemeObject> force();
};

struct SchemeEnvironment : public SchemeObject
{
    Context context;

    SchemeEnvironment(const Context &c) : context(c)
    {};

    std::string external_repr() const override;
};

std::pair<std::shared_ptr<SchemeObject>, bool>
do_quote(std::shared_ptr<ASTNode> node, Context &context, int quasi_level);

inline std::shared_ptr<SchemeObject> to_object(const std::shared_ptr<SchemeObject> &a)
{
    return a;
}

#endif
