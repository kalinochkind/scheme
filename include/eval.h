#ifndef SCHEME_EVAL_H
#define SCHEME_EVAL_H

#include <chrono>
#include <stdexcept>
#include <memory>
#include "schemeobject.h"

class eval_error: public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

extern std::chrono::milliseconds start_time;

std::shared_ptr<SchemeObject>
execute_function(std::shared_ptr<SchemeFunc> f, const std::list<std::shared_ptr<SchemeObject>> &val_list);

#endif //SCHEME_EVAL_H
