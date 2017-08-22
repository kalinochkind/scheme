#include <string>

std::string startup = "(define (> x y) (< y x)) "
        "(define true (< 0 1)) "
        "(define false (< 1 0)) "
        "(define (not x) (if x false true)) "
        "(define (abs x) (if (< x 0) (- x) x)) "
        "(define (positive? x) (> x 0)) "
        "(define (negative? x) (< x 0)) "
        "(define (list . x) x) "
        "(define nil (list)) "
        "(define (newline) (display \"\n\")) "
        "(define (min a . l)"
        "  (define (proc l) (if (or (null? (cdr l)) (< (car l) (proc (cdr l)))) (car l) (proc (cdr l))))"
        "  (proc (cons a l))) "
        "(define (max a . l)"
        "  (define (proc l) (if (or (null? (cdr l)) (< (proc (cdr l)) (car l))) (car l) (proc (cdr l))))"
        "  (proc (cons a l))) ";
