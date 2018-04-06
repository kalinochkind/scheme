#include <iostream>
#include <chrono>
#include <sstream>
#include <fstream>
#include "eval.h"
#include "parser.h"
#include "std.h"

std::chrono::milliseconds start_time;


static int runREPL(Context &global_context)
{
    std::shared_ptr<ASTNode> x;
    while(true)
    {
        try
        {
            std::cout << ">> ";
            x = readObject(std::cin);
        }
        catch(end_of_input)
        {
            break;
        }
        catch(parse_error &e)
        {
            std::cerr << "Parse error: " << e.what() << std::endl;
            continue;
        }
        try
        {
            std::cout << x->evaluate(global_context)->externalRepr() << '\n';
        }
        catch(eval_error &e)
        {
            std::cerr << "Eval error: " << e.what() << std::endl;
        }
    }
    return 0;
}

static int runFile(Context &global_context, std::istream &file)
{
    std::shared_ptr<ASTNode> x;
    while(true)
    {
        try
        {
            x = readObject(file);
        }
        catch(end_of_input)
        {
            return 0;
        }
        catch(parse_error &e)
        {
            std::cerr << "Parse error: " << e.what() << std::endl;
            return 1;
        }
        try
        {
            x->evaluate(global_context);
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
    Context global_context = initGlobalContext();
    if (argc <= 1)
    {
        return runREPL(global_context);
    }
    else
    {
        std::ifstream infile(argv[1]);
        if(!infile.is_open())
        {
            std::cerr << "Error: no such file\n";
            return 1;
        }
        return runFile(global_context, infile);
    }
}
