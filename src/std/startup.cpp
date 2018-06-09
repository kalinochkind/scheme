#include <string>
#include <schemeobject.h>
#include <sstream>
#include <parser.h>

const std::string startup = "(define (> x y) (< y x)) "
        "(define (>= x y) (not (< x y))) "
        "(define (<= x y) (not (< y x))) "
        "(define true #t) "
        "(define false #f) "
        "(define (not x) (if x #f #t)) "
        "(define (abs x) (if (< x 0) (- x) x)) "
        "(define (positive? x) (> x 0)) "
        "(define (negative? x) (< x 0)) "
        "(define (zero? x) (= x 0)) "
        "(define (list . x) x) "
        "(define nil (list)) "
        "(define (newline) (display \"\\n\")) "
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
        "(define (append . x)"
        "  (define (f x y) (if (null? x) y (cons (car x) (f (cdr x) y))))"
        "  (fold-right f nil x)) "
        "(define (even? x) (= 0 (remainder x 2))) "
        "(define (odd? x) (not (even? x))) "
        "(define (filter predicate sequence)"
        "  (cond ((null? sequence) nil) ((predicate (car sequence)) (cons (car sequence)"
        "  (filter predicate (cdr sequence)))) (else (filter predicate (cdr sequence))))) "
        "(define (fold-right op initial sequence)"
        "  (if (null? sequence) initial (op (car sequence) (fold-right op initial (cdr sequence))))) "
        "(define (fold-left op initial sequence)"
        "  (define (iter result rest) (if (null? rest) result (iter (op result (car rest))"
        "  (cdr rest)))) (iter initial sequence)) "
        "(define (reverse items)"
        "  (fold-left (lambda (x y) (cons y x)) nil items)) "
        "(define (null? x) (eq? x nil)) "
        "(define (equal? a b)"
        "  (if (and (pair? a) (pair? b)) (and (equal? (car a) (car b)) (equal? (cdr a) (cdr b))) (eq? a b))) "
        "(define quotient /) "
        "(define (gcd a b) (if (= b 0) a (gcd b (remainder a b)))) "
        "(define (last-pair x) (if (pair? (cdr x)) (last-pair (cdr x)) x)) "
        "(define (append! x . y)"
        "  (define (f x y) (set-cdr! (last-pair x) y) x)"
        "  (if (null? y) x (f x (apply append! y)))) "
        "(define stream-car car) "
        "(define (stream-cdr x) (force (cdr x))) "
        "(define stream-null? null?) "
        "(define the-empty-stream nil) "
        "(define (stream-map fun . args)"
        "  (define (cars l) (if (null? l) nil (cons (stream-car (car l)) (cars (cdr l)))))"
        "  (define (cdrs l) (if (null? l) nil (cons (stream-cdr (car l)) (cdrs (cdr l)))))"
        "  (define (nils l) (and (not (null? l)) (or (stream-null? (car l)) (nils (cdr l)))))"
        "  (if (or (null? args) (nils args)) the-empty-stream (cons-stream (apply fun (cars args)) (apply stream-map fun (cdrs args))))) "
        "(define (stream-ref stream k) (if (= k 0) (stream-car stream) (stream-ref (stream-cdr stream) (- k 1)))) "
        "(define (stream-head stream k) (if (= k 0) nil (cons (stream-car stream) (stream-head (stream-cdr stream) (- k 1))))) "
        "(define system-global-environment (the-environment)) "
        "(define (member-procedure eq) (define (fun object list) "
        "  (cond ((null? list) false) ((eq object (car list)) list) (else (fun object (cdr list))))) fun) "
        "(define memq (member-procedure eq?)) "
        "(define member (member-procedure equal?)) "
        "(define eqv? eq?) "
        "(define real? number?) "
        "(define (integer? x) (and (number? x) (= x (round x)))) "
        "(define (inexact? x) (and (number? x) (not (exact? x)))) "
        "(define exact-integer? exact?)"
        "(define (exact-nonnegative-integer? x) (and (exact? x) (>= x 0))) "
        "(define (1+ x) (+ 1 x)) "
        "(define (-1+ x) (- x 1)) "
        "(define (expt x y) (exp (* y (log x)))) "
        "(define char=? eq?) "
        "(define (char<? a b) (< (char->integer a) (char->integer b))) "
        "(define (char>? a b) (char<? b a)) "
        "(define (char<=? a b) (or (char<? a b) (char=? a b))) "
        "(define (char>=? a b) (or (char>? a b) (char=? a b))) "
        "(define (char-ci=? a b) (char=? (char-downcase a) (char-downcase b))) "
        "(define (char-ci<? a b) (char<? (char-downcase a) (char-downcase b))) "
        "(define (char-ci>? a b) (char-ci<? b a)) "
        "(define (char-ci<=? a b) (or (char-ci<? a b) (char-ci=? a b))) "
        "(define (char-ci>=? a b) (or (char-ci>? a b) (char-ci=? a b))) "
        "(define char-code char->integer) "
        "(define char-ascii? char?) "
        "(define char->ascii char->integer) "
        "(define ascii->char integer->char) "
        "(define (list->string l) (apply string l)) "
        "(define (string-null? s) (eq? s \"\")) "
        "(define string=? eq?) "
        ;


Context initGlobalContext()
{
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
    return global_context;
}
