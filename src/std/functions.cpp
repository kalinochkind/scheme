#include <iostream>
#include <cmath>
#include "std.h"
#include "eval.h"

std::chrono::milliseconds get_current_time()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
}

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
                                          long long (*llf)(long long, long long), double (*df)(double, double))
{
    long long n = start;
    double d = start;
    bool is_double = false;
    for(auto i : l)
    {
        if(!is_int(i) && !is_float(i))
        {
            throw eval_error(i->toString() + " is not an number");
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


std::unordered_map<std::string, std::function<std::shared_ptr<SchemeObject>(
        const std::list<std::shared_ptr<SchemeObject>> &)>> functions = {
        {"+",         [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return fold(l, 0, [](long long a, long long b) { return a + b; },
                        [](double a, double b) { return a + b; });
        }
        },
        {"-",         [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() > 2)
                throw eval_error("-: one or two arguments required");
            return fold(l, 0, [](long long a, long long b) { return -a - b; },
                        [](double a, double b) { return -a - b; });
        }
        },
        {"*",         [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return fold(l, 1, [](long long a, long long b) { return a * b; }, [](double a, double b) { return a * b; });
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
                throw eval_error("random: an integer required");
            long long max = get_int(l.front(), "random: an integer required");
            long long res = (rand() * 1ll * RAND_MAX + rand()) % max;
            return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeInt>(res));
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
        {"random",    [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("random: an integer required");
            long long max = get_int(l.front(), "random: an integer required");
            long long res = (rand() * 1ll * RAND_MAX + rand()) % max;
            return std::dynamic_pointer_cast<SchemeObject>(std::make_shared<SchemeInt>(res));
        }
        },
        {"newline",   [](const std::list<std::shared_ptr<SchemeObject>> &) {
            std::cout << std::endl;
            return scheme_empty;
        }
        },
        {"display",   [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.size() != 1)
                throw eval_error("display: one argument required");
            std::cout << l.front()->toString();
            return scheme_empty;
        }
        },
        {"runtime",   [](const std::list<std::shared_ptr<SchemeObject>> &) {
            return std::dynamic_pointer_cast<SchemeObject>(
                    std::make_shared<SchemeInt>((get_current_time() - start_time).count()));
        }
        },
        {"error",     [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::string res = "error: ";
            for(auto i : l)
            {
                res += i->toString() + " ";
            }
            throw eval_error(res);
            return scheme_empty;
        }
        },
        {"sin",       math_function("sin", sin)},
        {"cos",       math_function("cos", cos)},
        {"exp",       math_function("exp", exp)},
        {"log",       math_function("log", log)},
        {"tan",       math_function("tan", tan)},
        {"asin",      math_function("asin", asin)},
        {"acos",      math_function("acos", acos)},
        {"atan",      math_function("atan", atan)},
};