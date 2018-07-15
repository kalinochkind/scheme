#include <sstream>
#include "parser.h"
#include "std.h"

static auto read_from_string(const std::string &s)
{
    std::istringstream is;
    is.str(s);
    return read_object(is);
}

static void run_test(const std::string &expression, const std::string &expected_value)
{
    static int test_num = 0;
    ++test_num;
    std::cout << "Test " << test_num << ": ";
    init_scheme();
    auto object1 = read_from_string(expression);
    if(object1.result != parse_result_t::OK)
    {
        std::cout << "FAILED\nCould not parse expression " << expression << ": " << object1.error << std::endl;
        exit(1);
    }
    auto object2 = read_from_string(expected_value);
    if(object2.result != parse_result_t::OK)
    {
        std::cout << "FAILED\nCould not parse value " << expected_value << ": " << object2.error << std::endl;
        exit(1);
    }
    std::shared_ptr<SchemeObject> result1, result2;
    try
    {
        result1 = object1.node->evaluate(global_context).force_value();
    }
    catch(eval_error &e)
    {
        std::cout << "FAILED\nEvaluation of " << expression << " failed: " << e.what() << std::endl;
        exit(1);
    }
    try
    {
        result2 = object2.node->evaluate(global_context).force_value();
    }
    catch(eval_error &e)
    {
        std::cout << "FAILED\nEvaluation of " << expected_value << " failed: " << e.what() << std::endl;
        exit(1);
    }
    if(result1 != result2 && !result1->is_eq(result2))
    {
        std::cout << "FAILED\n" << expression << " != " << expected_value << " [[ " << result1->external_repr()
                  << " != " << result2->external_repr() << " ]]" << std::endl;
        exit(1);
    }
    std::cout << "OK" << std::endl;
}

int main()
{
    run_test("(+ 2 2)", "4");
    run_test("(= (* 2 2) 4.)", "#t");
}
