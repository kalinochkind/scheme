#ifndef SCHEME_STD_H
#define SCHEME_STD_H

#include <unordered_map>
#include <chrono>
#include <functional>
#include "schemeobject.h"

extern std::shared_ptr<SchemeObject> scheme_true, scheme_false, scheme_empty, scheme_nil;


bool eq_test(std::shared_ptr<SchemeObject> a, std::shared_ptr<SchemeObject> b);

std::chrono::milliseconds get_current_time();

Context initGlobalContext();




template<class T>
class Registry
{
    std::unordered_map<std::string, T> map;

    static Registry &instance();

public:

    static void add(const std::string &name, const T &);

    static bool exists(const std::string &);

    static T get(const std::string &);
};

using BuiltinFunction = std::function<std::shared_ptr<SchemeObject>(
    const std::list<std::shared_ptr<SchemeObject>> &)>;
using FunctionRegistry = Registry<BuiltinFunction>;

using SpecialForm = std::function<std::shared_ptr<SchemeObject>(const std::list<std::shared_ptr<ASTNode>> &,
                                                                Context &context,
                                                                std::shared_ptr<SchemeFunc> tail_func)>;
using SpecialFormRegistry = Registry<SpecialForm>;



struct Package
{
    Package(const std::map<std::string, BuiltinFunction> &);
    Package(const std::map<std::string, SpecialForm> &);
};

#endif
