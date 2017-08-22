#ifndef SCHEME_STD_H
#define SCHEME_STD_H

#include <unordered_map>
#include <chrono>
#include <functional>
#include "schemeobject.h"

extern std::shared_ptr<SchemeObject> scheme_true, scheme_false, scheme_empty;

extern std::unordered_map<std::string, std::function<std::shared_ptr<SchemeObject>(
        const std::list<std::shared_ptr<ASTNode>> &,
        Context &context,
        std::shared_ptr<SchemeFunc> tail_func)>> special_forms;

extern std::unordered_map<std::string, std::function<std::shared_ptr<SchemeObject>(
        const std::list<std::shared_ptr<SchemeObject>> &)>> functions;

std::chrono::milliseconds get_current_time();

#endif //SCHEME_STD_H
