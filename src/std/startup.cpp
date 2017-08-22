#include <string>

std::string startup = "(define (> x y) (< y x)) "
        "(define true (< 0 1)) "
        "(define false (< 1 0)) "
        "(define (not x) (if x false true)) "
        "(define (abs x) (if (< x 0) (- x) x)) "
        "(define (positive? x) (> x 0)) "
        "(define (negative? x) (< x 0)) "
        "(define nil (list))";
