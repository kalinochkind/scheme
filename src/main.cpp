#include <iostream>
#include <chrono>
#include <eval.h>
#include "parser.h"
#include "std.h"

extern std::string startup;
std::chrono::milliseconds start_time;

int main()
{
    start_time = get_current_time();
    srand(time(0));
    std::string s;
    Context global_context;
    global_context.newFrame();
    std::vector<ASTNode> x = parseString(startup);
    for(auto &&i : x)
    {
        i.evaluate(global_context);
    }
    while(true)
    {
        std::getline(std::cin, s);
        if(std::cin.eof())
            break;
        try
        {
            while(true)
            {
                try
                {
                    x = parseString(s);
                    break;
                }
                catch(unexpected_eol_error &e)
                {
                    std::string t;
                    std::getline(std::cin, t);
                    if(std::cin.eof())
                        break;
                    s += "\n" + t;
                }
            }
        }
        catch(parse_error &e)
        {
            std::cerr << "Parse error: " << e.what() << std::endl;
            continue;
        }
        for(auto &&i : x)
        {
            try
            {
                std::cout << i.evaluate(global_context)->toString() << '\n';
            }
            catch(eval_error &e)
            {
                std::cerr << "Eval error: " << e.what() << std::endl;
            }
        }
    }
    return 0;
}