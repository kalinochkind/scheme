#include "std.h"

std::chrono::milliseconds get_current_time()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
}

static FunctionPackage package(
    {
        {"runtime", {0, 0, [](const std::list<std::shared_ptr<SchemeObject>> &) {
            return to_object(std::make_shared<SchemeInt>((get_current_time() - start_time).count()));
        }}},
        {"error", {0, -1, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            std::string res = "error: ";
            for(auto i : l)
            {
                res += i->external_repr() + " ";
            }
            throw eval_error(res);
            return scheme_empty;
        }}},
        {"eq?", {2, 2, [](const std::list<std::shared_ptr<SchemeObject>> &l) {
            return eq_test(l.front(), l.back()) ? scheme_true : scheme_false;
        }}},
    }
);
