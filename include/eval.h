#ifndef SCHEME_EVAL_H
#define SCHEME_EVAL_H

#include <chrono>
#include <stdexcept>

class eval_error: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

extern std::chrono::milliseconds start_time;

#endif //SCHEME_EVAL_H
