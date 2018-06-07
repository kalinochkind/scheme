#include "std.h"


void FunctionRegistry::add(const std::string &name, const BuiltinFunction &f)
{
    instance().map[name] = f;
}

BuiltinFunction FunctionRegistry::get(const std::string &name)
{
    if(instance().map.count(name))
        return instance().map[name];
    return nullptr;
}

bool FunctionRegistry::exists(const std::string &name)
{
    return instance().map.count(name) != 0;
}

FunctionRegistry &FunctionRegistry::instance()
{
    static FunctionRegistry a;
    return a;
}

Package::Package(const std::map<std::string, BuiltinFunction> &map)
{
    for(auto &&p : map)
    {
        FunctionRegistry::add(p.first, p.second);
    }
}