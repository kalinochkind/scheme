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
        "  (define (min-iter a l)"
        "    (cond ((null? l) a) ((< a (car l)) (min-iter a (cdr l))) (else (min-iter (car l) (cdr l)))))"
        "  (min-iter a l)) "
        "(define (max a . l)"
        "  (define (max-iter a l)"
        "    (cond ((null? l) a) ((< (car l) a) (max-iter a (cdr l))) (else (max-iter (car l) (cdr l)))))"
        "  (max-iter a l)) "
        "(define (map fun . args)"
        "  (define (cars l) (if (null? l) nil (cons (car (car l)) (cars (cdr l)))))"
        "  (define (cdrs l) (if (null? l) nil (cons (cdr (car l)) (cdrs (cdr l)))))"
        "  (define (nils l) (and (not (null? l)) (or (null? (car l)) (nils (cdr l)))))"
        "  (if (or (null? args) (nils args)) nil (cons (apply fun (cars args)) (apply map fun (cdrs args))))) "
        "(define (for-each fun . args)"
        "  (define (cars l) (if (null? l) nil (cons (car (car l)) (cars (cdr l)))))"
        "  (define (cdrs l) (if (null? l) nil (cons (cdr (car l)) (cdrs (cdr l)))))"
        "  (define (nils l) (and (not (null? l)) (or (null? (car l)) (nils (cdr l)))))"
        "  (cond ((or (null? args) (nils args)) \"\") (else (apply fun (cars args)) (apply for-each fun (cdrs args))))) "
        "(define (length l)"
        "  (define (length-iter l c) (if (null? l) c (length-iter (cdr l) (+ c 1))))"
        "  (length-iter l 0)) "
        "(define (append list1 list2)"
        "  (if (null? list1) list2 (cons (car list1) (append (cdr list1) list2)))) ";
