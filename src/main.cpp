#include <iostream>
#include <chrono>
#include <sstream>
#include "eval.h"
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
    std::istringstream st;
    st.str(startup);
    std::shared_ptr<ASTNode> x;
    while(1)
    {
        try
        {
            x = readObject(st);
            x->evaluate(global_context);
        }
        catch(end_of_input)
        {
            break;
        }
    }
    global_context.newFrame();
    global_context.set("user-initial-environment", std::make_shared<SchemeEnvironment>(global_context));
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