#ifndef SCHEME_SCHEMEOBJECT_H
#define SCHEME_SCHEMEOBJECT_H

#include <string>
#include <list>
#include <memory>
#include <map>
#include <chrono>
#include <vector>


struct SchemeObject;
struct SchemeFunc;

class eval_error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

extern std::chrono::milliseconds start_time;


struct TailContext
{
    std::shared_ptr<SchemeFunc> func;
    std::list<std::shared_ptr<SchemeObject>> args;

    TailContext(const std::shared_ptr<SchemeFunc> func, const std::list<std::shared_ptr<SchemeObject>> args) :
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

    explicit ExecutionResult(const std::shared_ptr<SchemeObject> &value) : type(execution_result_t::VALUE),
                                                                           value(value),
                                                                           tail_context(nullptr)
    {};

    explicit ExecutionResult(const std::shared_ptr<SchemeFunc> &func,
                             const std::list<std::shared_ptr<SchemeObject>> &args) :
        type(execution_result_t::TAIL_CALL), value(nullptr), tail_context(std::make_unique<TailContext>(func, args))
    {};

    std::shared_ptr<SchemeObject> force_value();

    /* explicit ExecutionResult(const std::string &error) : type(execution_result_t::ERROR),
                                                          value(std::make_shared<SchemeString>(error)),
                                                          tail_context(nullptr)
     {};*/
};

ExecutionResult
execute_function(std::shared_ptr<SchemeFunc> f, const std::list<std::shared_ptr<SchemeObject>> &val_list);


using context_map_t = std::map<std::string, std::shared_ptr<SchemeObject>>;

enum class ast_type_t
{
    LIST, NAME, INT, FLOAT, STRING, BOOL, CHAR, VECTOR
};

struct Context
{
    std::list<std::shared_ptr<context_map_t>> locals;

    Context(): locals() {};

    std::shared_ptr<SchemeObject> get(const std::string &name) const;

    void set(const std::string &name, std::shared_ptr<SchemeObject> value);

    void assign(const std::string &name, std::shared_ptr<SchemeObject> value);

    void newFrame();

    void newFrame(const context_map_t &vars);

    void delFrame();
};

struct ASTNode
{
    ast_type_t type;
    std::string value;
    std::list<std::shared_ptr<ASTNode>> list;

    ASTNode(): type(ast_type_t::LIST), value(), list() {};
    ASTNode(ast_type_t type, const std::string &value): type(type), value(value), list() {};

    ExecutionResult evaluate(Context &context);

};


struct SchemeObject
{
    virtual ~SchemeObject() {};

    virtual std::string externalRepr() const = 0;

    virtual std::string printable() const
    {
        return externalRepr();
    }

    virtual bool toBool() const
    {
        return true;
    }

    virtual std::shared_ptr<ASTNode> toAST() const;

};

struct SchemeInt: public SchemeObject
{
    long long value;

    SchemeInt(long long a): value(a) {};

    std::string externalRepr() const override;
    std::shared_ptr<ASTNode> toAST() const override;
};

struct SchemeFloat: public SchemeObject
{
    double value;

    SchemeFloat(double a): value(a) {};

    std::string externalRepr() const override;
    std::shared_ptr<ASTNode> toAST() const override;
};

struct SchemeFunc: public SchemeObject
{
    std::string name;
    std::list<std::string> params;
    std::list<ASTNode> body;
    Context context;
    bool arglist;

    SchemeFunc(std::string name = ""): name(name), params(), body(), context(), arglist(false) {};

    std::string externalRepr() const override;
};

struct SchemeBuiltinFunc: public SchemeFunc
{
    using SchemeFunc::SchemeFunc;

    std::string externalRepr() const override;
};

struct SchemeBool: public SchemeObject
{
    bool value;

    SchemeBool(bool a): value(a) {};

    std::string externalRepr() const override;
    std::shared_ptr<ASTNode> toAST() const override;

    bool toBool() const override
    {
        return value;
    }
};

struct SchemeString: public SchemeObject
{
    std::string value;

    SchemeString(std::string s): value(s) {};

    std::string externalRepr() const override;
    std::shared_ptr<ASTNode> toAST() const override;
    std::string printable() const override;
};

struct SchemeChar: public SchemeObject
{
    unsigned char value;

    SchemeChar(unsigned char c): value(c) {};

    std::string externalRepr() const override;
    std::shared_ptr<ASTNode> toAST() const override;
    std::string printable() const override;
};

struct SchemePair: public SchemeObject
{
    std::shared_ptr<SchemeObject> car, cdr;

    SchemePair(std::shared_ptr<SchemeObject> a, std::shared_ptr<SchemeObject> b): car(std::move(a)), cdr(std::move(b))
    {};

    std::string externalRepr() const override;
    std::shared_ptr<ASTNode> toAST() const override;

    long long listLength() const;
};

struct SchemeCell: public SchemeObject
{
    std::shared_ptr<SchemeObject> value;

    SchemeCell(std::shared_ptr<SchemeObject> a): value(std::move(a)) {};

    std::string externalRepr() const override;
};

struct SchemeVector: public SchemeObject
{
    std::vector<std::shared_ptr<SchemeObject>> vec;

    SchemeVector(): vec() {};
    static std::shared_ptr<SchemeVector> fromList(std::shared_ptr<SchemePair> list);

    std::shared_ptr<SchemePair> toList() const;

    std::string externalRepr() const override;
    std::shared_ptr<ASTNode> toAST() const override;

};

struct SchemeSymbol: public SchemeObject
{
    std::string value;
    bool uninterned{false};
    static long long uninterned_counter;

    SchemeSymbol(std::string s): value(s) {};

    std::string externalRepr() const override;
    std::shared_ptr<ASTNode> toAST() const override;
};

struct SchemePromise: public SchemeObject
{
    std::shared_ptr<SchemeObject> value;
    std::shared_ptr<SchemeFunc> func;

    SchemePromise(std::shared_ptr<SchemeFunc> f): value(nullptr), func(f) {};

    std::string externalRepr() const override;

    std::shared_ptr<SchemeObject> force();
};

struct SchemeEnvironment: public SchemeObject
{
    Context context;

    SchemeEnvironment(const Context &c): context(c) {};

    std::string externalRepr() const override;
};

std::pair<std::shared_ptr<SchemeObject>, bool>
do_quote(std::shared_ptr<ASTNode> node, Context &context, int quasi_level);

#endif
