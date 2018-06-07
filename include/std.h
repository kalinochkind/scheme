#ifndef SCHEME_STD_H
#define SCHEME_STD_H

#include <unordered_map>
#include <chrono>
#include <functional>
#include "schemeobject.h"

extern std::shared_ptr<SchemeObject> scheme_true, scheme_false, scheme_empty, scheme_nil;

extern std::unordered_map<std::string, std::function<std::shared_ptr<SchemeObject>(
        const std::list<std::shared_ptr<ASTNode>> &,
        Context &context,
        std::shared_ptr<SchemeFunc> tail_func)>> special_forms;

bool eq_test(std::shared_ptr<SchemeObject> a, std::shared_ptr<SchemeObject> b);

std::chrono::milliseconds get_current_time();

Context initGlobalContext();


using BuiltinFunction = std::function<std::shared_ptr<SchemeObject>(
        const std::list<std::shared_ptr<SchemeObject>> &)>;

class FunctionRegistry
{
    std::unordered_map<std::string, BuiltinFunction> map;

    static FunctionRegistry &instance();

public:

    static void add(const std::string &name, const BuiltinFunction &);

    static bool exists(const std::string &);

    static BuiltinFunction get(const std::string &);
};


struct Package
{
    Package(const std::map<std::string, BuiltinFunction> &);
};

#endif
