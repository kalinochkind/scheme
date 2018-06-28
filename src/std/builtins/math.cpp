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
    auto pi = std::dynamic_pointer_cast<SchemeInt>(p);
    if(pi)
        return pi->value;
    auto pf = std::dynamic_pointer_cast<SchemeFloat>(p);
    if(pf)
        return pf->value;
    throw eval_error(error_msg);
}

static long long get_int(std::shared_ptr<SchemeObject> p, const std::string &error_msg)
{
    auto pi = std::dynamic_pointer_cast<SchemeInt>(p);
    if(pi)
        return pi->value;
    throw eval_error(error_msg);
}

BuiltinFunction math_function(const std::string &name, double (*fun)(double))
{
    return {1, 1, [name, fun](const std::list<std::shared_ptr<SchemeObject>> &l) {
        double arg = get_value(l.front(), name + ": number required");
        return to_object(std::make_shared<SchemeFloat>(fun(arg)));
    }};
}

static ExecutionResult fold(const std::list<std::shared_ptr<SchemeObject>> &l, long long start,
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
            throw eval_error(i->external_repr() + " is not an number");
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
        return to_object(std::make_shared<SchemeFloat>(d));
    else
        return to_object(std::make_shared<SchemeInt>(n));
}


static FunctionPackage package(
    {
        {"+", {0, -1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return fold(l, 0, [](long long a, long long b) { return a + b; },
                        [](double a, double b) { return a + b; });
        }}},
        {"-", {1, -1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.empty())
                throw eval_error("-: at least one argument required");
            return fold(l, 0, [](long long a, long long b) { return a - b; },
                        [](double a, double b) { return a - b; }, l.size() > 1);
        }}},
        {"*", {0, -1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return fold(l, 1, [](long long a, long long b) { return a * b; },
                        [](double a, double b) { return a * b; });
        }}},
        {"/", {1, -1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(l.empty())
                throw eval_error("/: at least one argument required");
            if(l.size() == 1)
            {
                auto val = get_value(l.front(), "/: numbers required");
                if(val == 0)
                    throw eval_error("Division by zero");
                return to_object(std::make_shared<SchemeFloat>(1 / val));
            }
            double val = get_value(l.front(), "/: numbers required");
            for(auto it = next(l.begin()); it != l.end(); ++it)
            {
                auto d = get_value(*it, "/: numbers required");
                if(d == 0)
                    throw eval_error("Division by zero");
                val /= d;
            }
            return to_object(std::make_shared<SchemeFloat>(val));
        }}},
        {"quotient", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto a = std::dynamic_pointer_cast<SchemeInt>(l.front());
            auto b = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!a || !b)
                throw eval_error("quotient: two ints required");
            if(b->value == 0)
                throw eval_error("Division by zero");
            return to_object(std::make_shared<SchemeInt>(a->value / b->value));
        }}},
        {"remainder", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto a = std::dynamic_pointer_cast<SchemeInt>(l.front());
            auto b = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!a || !b)
                throw eval_error("remainder: two ints required");
            if(b->value == 0)
                throw eval_error("Division by zero");
            return to_object(std::make_shared<SchemeInt>(a->value % b->value));
        }}},
        {"modulo", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            auto a = std::dynamic_pointer_cast<SchemeInt>(l.front());
            auto b = std::dynamic_pointer_cast<SchemeInt>(l.back());
            if(!a || !b)
                throw eval_error("modulo: two ints required");
            if(b->value == 0)
                throw eval_error("Division by zero");
            long long res;
            if((a->value > 0) ^ (b->value > 0))
                res = (b->value - (-a->value % b->value)) % b->value;
            else
                res = a->value % b->value;
            return to_object(std::make_shared<SchemeInt>(res));
        }}},
        {"random", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(is_int(l.front()))
            {
                long long max = get_int(l.front(), "");
                long long res = max ? (rand() * 1ll * RAND_MAX + rand()) % max : 0;
                return to_object(std::make_shared<SchemeInt>(res));
            }
            else if(is_float(l.front()))
            {
                double max = get_value(l.front(), "");
                double res = (rand() * 1ll * RAND_MAX + rand()) / (RAND_MAX * 1. * RAND_MAX) * max;
                return to_object(std::make_shared<SchemeFloat>(res));
            }
            else
                throw eval_error("random: a number required");
        }}},
        {"<", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(get_value(l.front(), "<: numbers required") <
               get_value((*next(l.begin())), "<: numbers required"))
                return scheme_true;
            else
                return scheme_false;
        }}},
        {"=", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(get_value(l.front(), "=: numbers required") ==
               get_value((*next(l.begin())), "=: numbers required"))
                return scheme_true;
            else
                return scheme_false;
        }}},
        {"round", math_function("round", round)},
        {"ceiling", math_function("ceiling", ceil)},
        {"truncate", math_function("truncate", trunc)},
        {"floor", math_function("floor", floor)},
        {"sin", math_function("sin", sin)},
        {"cos", math_function("cos", cos)},
        {"exp", math_function("exp", exp)},
        {"log", math_function("log", log)},
        {"tan", math_function("tan", tan)},
        {"asin", math_function("asin", asin)},
        {"acos", math_function("acos", acos)},
        {"sqrt", math_function("sqrt", sqrt)},
        {"atan", {1, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            double res, arg = get_value(l.front(), "atan: number required");
            if(l.size() == 1)
                res = atan(arg);
            else
                res = atan2(arg, get_value(l.back(), "atan: number required"));
            return to_object(std::make_shared<SchemeFloat>(res));
        }}},
        {"exact?", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return is_int(l.front()) ? scheme_true : scheme_false;
        }}},
        {"number->string", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            if(!(is_int(l.front()) || is_float(l.front())))
                throw eval_error("number->string: a number required");
            return to_object(std::make_shared<SchemeString>(l.front()->external_repr()));
        }}},
        {"string->number", {1, 1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::shared_ptr<SchemeString> s;
            if(!(s = std::dynamic_pointer_cast<SchemeString>(l.front())))
                throw eval_error("string->number: a string required");
            ast_type_t type = identifier_type(s->value);
            try
            {
                if(type == ast_type_t::INT)
                    return to_object(std::make_shared<SchemeInt>(stoll(s->value)));
                else if(type == ast_type_t::FLOAT)
                    return to_object(std::make_shared<SchemeFloat>(stod(s->value)));
                else
                    return scheme_false;
            }
            catch(std::out_of_range &)
            {
                return scheme_false;
            }
        }}},
    }
);