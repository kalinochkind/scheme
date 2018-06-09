#include <cmath>
#include "std.h"
#include "parser.h"

static bool is_int(std::shared_ptr<SchemeObject> p)
{
    return bool(std::dynamic_pointer_cast<SchemeInt>(p));
}

static bool is_float(std::shared_ptr<SchemeObject> p)
{
    return bool(std::dynamic_pointer_cast<SchemeFloat>(p));
}

static double get_value(std::shared_ptr<SchemeObject> p, const std::string &error_msg)
{
    if(is_int(p))
        return std::dynamic_pointer_cast<SchemeInt>(p)->value;
    if(is_float(p))
        return std::dynamic_pointer_cast<SchemeFloat>(p)->value;
    throw eval_error(error_msg);
}

static long long get_int(std::shared_ptr<SchemeObject> p, const std::string &error_msg)
{
    if(is_int(p))
        return std::dynamic_pointer_cast<SchemeInt>(p)->value;
    throw eval_error(error_msg);
}

std::function<std::shared_ptr<SchemeObject>(const std::list<std::shared_ptr<SchemeObject>> &)>
math_function(const std::string &name, double (*fun)(double))
{
    return [name, fun](const std::list<std::shared_ptr<SchemeObject>> &l) {
        if(l.size() != 1)
            throw eval_error(name + ": number required");
        double arg = get_value(l.front(), name + ": number required");
        return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeFloat>(fun(arg)));
    };
}

static std::shared_ptr<SchemeObject> fold(const std::list<std::shared_ptr<SchemeObject>> &l, long long start,
                                          long long (*llf)(long long, long long), double (*df)(double, double),
                                          bool start_with_first = false)
{
    long long n = start;
    double d = start;
    bool is_double = false;
    for(auto i : l)
    {
        if(!is_int(i) && !is_float(i))
        {
            throw eval_error(i->externalRepr() + " is not an number");
        }
        if(start_with_first)
        {
            if(is_float(i))
            {
                is_double = true;
                d = std::dynamic_pointer_cast<SchemeFloat>(i)->value;
            }
            else
            {
                d = n = std::dynamic_pointer_cast<SchemeInt>(i)->value;
            }
            start_with_first = false;
            continue;
        }
        if(is_float(i))
        {
            is_double = true;
            d = df(d, std::dynamic_pointer_cast<SchemeFloat>(i)->value);
        }
        else if(is_double)
        {
            d = df(d, std::dynamic_pointer_cast<SchemeInt>(i)->value);
        }
        else
        {
            n = llf(n, std::dynamic_pointer_cast<SchemeInt>(i)->value);
            d = df(d, std::dynamic_pointer_cast<SchemeInt>(i)->value);
        }
    }
    if(is_double)
        return std::make_shared<SchemeFloat>(d);
    else
        return std::make_shared<SchemeInt>(n);
}


static Package package(
    {
        {"+",         [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return fold(l, 0, [](long long a, long long b) { return a + b; },
                        [](double a, double b) { return a + b; });
        }
        },
        {"-",         [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return fold(l, 0, [](long long a, long long b) { return a - b; },
                        [](double a, double b) { return a - b; }, l.size() > 1);
        }
        },
        {"*",         [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return fold(l, 1, [](long long a, long long b) { return a * b; },
                        [](double a, double b) { return a * b; });
        }
        },
        {"/",         [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("/: two arguments required");
            auto ap = l.front();
            auto bp = (*next(l.begin()));
            if(is_int(ap) && is_int(bp))
            {
                long long a = std::dynamic_pointer_cast<SchemeInt>(ap)->value;
                long long b = std::dynamic_pointer_cast<SchemeInt>(bp)->value;
                if(b == 0)
                    throw eval_error("Integer division by zero");
                return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeInt>(a / b));
            }
            double a = get_value(ap, "/: numbers required");
            double b = get_value(bp, "/: numbers required");
            if(b == 0)
                throw eval_error("Division by zero");
            return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeFloat>(a / b));
        }
        },
        {"remainder", [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("remainder: two arguments required");
            auto a = l.front();
            auto b = (*next(l.begin()));
            if(!is_int(a) || !is_int(b))
                throw eval_error("remainder: two ints required");
            long long x = std::dynamic_pointer_cast<SchemeInt>(a)->value;
            long long y = std::dynamic_pointer_cast<SchemeInt>(b)->value;
            if(y == 0)
                throw eval_error("Division by zero");
            return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeInt>(x % y));
        }
        },
        {"random",    [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("random: a number required");
            if(is_int(l.front()))
            {
                long long max = get_int(l.front(), "");
                long long res = max ? (rand() * 1ll * RAND_MAX + rand()) % max : 0;
                return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeInt>(res));
            }
            else if(is_float(l.front()))
            {
                double max = get_value(l.front(), "");
                double res = (rand() * 1ll * RAND_MAX + rand()) / (RAND_MAX * 1. * RAND_MAX) * max;
                return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeFloat>(res));
            }
            else
                throw eval_error("random: a number required");
        }
        },
        {"<",         [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("<: two arguments required");
            if(get_value(l.front(), "<: numbers required") <
               get_value((*next(l.begin())), "<: numbers required"))
                return scheme_true;
            else
                return scheme_false;
        }
        },
        {"=",         [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 2)
                throw eval_error("=: two arguments required");
            if(get_value(l.front(), "=: numbers required") ==
               get_value((*next(l.begin())), "=: numbers required"))
                return scheme_true;
            else
                return scheme_false;
        }
        },
        {"round",     math_function("round", round)},
        {"ceiling",   math_function("ceiling", ceil)},
        {"truncate",  math_function("truncate", trunc)},
        {"floor",     math_function("floor", floor)},
        {"sin",       math_function("sin", sin)},
        {"cos",       math_function("cos", cos)},
        {"exp",       math_function("exp", exp)},
        {"log",       math_function("log", log)},
        {"tan",       math_function("tan", tan)},
        {"asin",      math_function("asin", asin)},
        {"acos",      math_function("acos", acos)},
        {"sqrt",      math_function("sqrt", sqrt)},
        {"atan",      [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() < 1 || l.size() > 2)
                throw eval_error("atan: one or two numbers required");
            double res, arg = get_value(l.front(), "atan: number required");
            if(l.size() == 1)
                res = atan(arg);
            else
                res = atan2(arg, get_value(l.back(), "atan: number required"));
            return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeFloat>(res));
        }
        },
        {"exact?",    [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("exact?: one argument required");
            return is_int(l.front()) ? scheme_true : scheme_false;
        }
        },
        {"number->string",    [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1 || !(is_int(l.front()) || is_float(l.front())))
                throw eval_error("number->string: a number required");
            return std::make_shared<SchemeString>(l.front()->externalRepr());
        }
        },
        {"string->number",    [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::shared_ptr<SchemeString> s;
            if(l.size() != 1 || !(s = std::dynamic_pointer_cast<SchemeString>(l.front())))
                throw eval_error("string->number: a string required");
            ast_type_t type = identifier_type(s->value);
            try
            {
                if(type == ast_type_t::INT)
                    return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeInt>(stoll(s->value)));
                else if(type == ast_type_t::FLOAT)
                    return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeFloat>(stod(s->value)));
                else
                    return scheme_false;
            }
            catch(std::out_of_range &)
            {
                return scheme_false;
            }
        }
        },
    }
);