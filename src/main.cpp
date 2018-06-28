#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include "parser.h"
#include "std.h"

std::chrono::milliseconds start_time;


static int run_REPL(Context &global_context)
{
    ParseResult x;
    while(true)
    {
        std::cout << ">> ";
        x = read_object(std::cin);
        if(x.result == parse_result_t::END)
            break;
        if(x.result == parse_result_t::ERROR)
        {
            std::cerr << "Parse error: " << x.error << std::endl;
            continue;
        }
        try
        {
            std::cout << x.node->evaluate(global_context).force_value()->external_repr() << '\n';
        }
        catch(eval_error &e)
        {
            std::cerr << "Eval error: " << e.what() << std::endl;
        }
    }
    return 0;
}

static int run_file(Context &global_context, std::istream &file)
{
    ParseResult x;
    while(true)
    {
        x = read_object(file);
        if(x.result == parse_result_t::END)
            return 0;
        if(x.result == parse_result_t::ERROR)
        {
            std::cerr << "Parse error: " << x.error << std::endl;
            return 1;
        }
        try
        {
            x.node->evaluate(global_context).force_value();
        }
        catch(eval_error &e)
        {
            std::cerr << "Eval error: " << e.what() << std::endl;
            return 1;
        }
    }
}

int main(int argc, char **argv)
{
    start_time = get_current_time();
    srand(time(0));
    Context global_context = init_global_context();
    if (argc <= 1)
    {
        return run_REPL(global_context);
    }
    else
    {
        std::ifstream infile(argv[1]);
        if(!infile.is_open())
        {
            std::cerr << "Error: no such file\n";
            return 1;
        }
        return run_file(global_context, infile);
    }
}
