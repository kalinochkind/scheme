#include <assert.h>
#include "std.h"

template<class T>
void Registry<T>::add(const std::string &name, const T &f)
{
    instance().map[name] = f;
}

template<class T>
T Registry<T>::get(const std::string &name)
{
    if(instance().map.count(name))
        return instance().map[name];
    assert(false);
}

template<class T>
bool Registry<T>::exists(const std::string &name)
{
    return instance().map.count(name) != 0;
}


template<class T>
const std::unordered_map<std::string, T> Registry<T>::all()
{
    return instance().map;
}


template<class T>
Registry<T> &Registry<T>::instance()
{
    static Registry<T> a;
    return a;
}

FunctionPackage::FunctionPackage(const std::map<std::string, BuiltinFunction> &map)
{
    for(auto &&p : map)
    {
        FunctionRegistry::add(p.first, p.second);
    }
}

SpecialFormPackage::SpecialFormPackage(const std::map<std::string, SpecialForm > &map)
{
    for(auto &&p : map)
    {
        SpecialFormRegistry::add(p.first, p.second);
    }
}


template
class Registry<SpecialForm>;

template
class Registry<BuiltinFunction>;
