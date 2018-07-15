#ifndef SCHEME_STD_H
#define SCHEME_STD_H

#include <unordered_map>
#include <chrono>
#include <functional>
#include "schemeobject.h"

extern std::shared_ptr<SchemeObject> scheme_true, scheme_false, scheme_empty, scheme_nil;

inline bool eq_test(const std::shared_ptr<SchemeObject> &a, const std::shared_ptr<SchemeObject> &b)
{
    return a == b || a->is_eq(b);
}

std::chrono::milliseconds get_current_time();

void init_scheme();


template<class T>
class Registry
{
    std::unordered_map<std::string, T> map;

    static Registry &instance();

public:

    static void add(const std::string &name, const T &);

    static bool exists(const std::string &);

    static T get(const std::string &);

    static const std::unordered_map<std::string, T> all();
};

using BuiltinFunction = std::tuple<long long, long long, std::function<ExecutionResult(
    const std::list<std::shared_ptr<SchemeObject>> &)>>;
using FunctionRegistry = Registry<BuiltinFunction>;

using SpecialForm = std::function<ExecutionResult(const std::list<std::shared_ptr<ASTNode>> &,
                                                  const Context &context)>;
using SpecialFormRegistry = Registry<SpecialForm>;


struct FunctionPackage
{
    FunctionPackage(const std::map<std::string, BuiltinFunction> &);
};

struct SpecialFormPackage
{
    SpecialFormPackage(const std::map<std::string, SpecialForm> &);
};


#endif
