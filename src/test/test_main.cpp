#include <sstream>
#include "parser.h"
#include "std.h"


static std::shared_ptr<SchemeObject> evaluate_string_in_global_context(const std::string &s)
{
    std::istringstream is;
    is.str(s);
    ParseResult parsed;
    std::shared_ptr<SchemeObject> object{nullptr};
    int expression_index = 0;
    while(true)
    {
        ++expression_index;
        parsed = read_object(is);
        if(parsed.result == parse_result_t::END)
        {
            if(!object)
            {
                std::cout << "FAILED\nNothing to execute" << std::endl;
                exit(1);
            }
            break;
        }
        if(parsed.result == parse_result_t::ERROR)
        {
            std::cout << "FAILED\nCould not parse expression " << expression_index << " in " << s << ": "
                      << parsed.error << std::endl;
            exit(1);
        }
        try
        {
            object = parsed.node->evaluate(global_context).force_value();
        }
        catch(eval_error &e)
        {
            std::cout << "FAILED\nCould not evaluate expression " << expression_index << " in " << s << ": "
                      << e.what() << std::endl;
            exit(1);
        }
    }
    return object;
}

static void run_test(const std::string &expression, const std::string &expected_value)
{
    static int test_num = 0;
    ++test_num;
    std::cout << "Test " << test_num << ": ";
    init_scheme();
    std::istringstream in1, in2;
    auto expected_result = evaluate_string_in_global_context(expected_value);
    auto actual_result = evaluate_string_in_global_context(expression);
    if(expected_result != actual_result && !expected_result->is_eq(actual_result))
    {
        std::cout << "FAILED\n" << expression << " != " << expected_value << " [[ " << actual_result->external_repr()
                  << " != " << expected_result->external_repr() << " ]]" << std::endl;
        exit(1);
    }
    std::cout << "OK" << std::endl;
}

static void test_basic()
{
    run_test("(define x 1) (define (f x) (g 2)) (define (g y) (+ x y)) (f 5)", "3");
    run_test("(equal? (list #t #f true false (if #t 1 2) (if #f 1 2) (if 0 1 2)) '(#t #f #t #f 1 2 1))", "#t");
    run_test("(equal? '(8 . (13 . ())) (list 8 13))", "#t");
    run_test("\n(define\tx 5 )(+\tx  2)  ", "\n7\n");
    run_test("'()'()\"q\"`();", "nil;t");
    run_test("(define name \"max\")(equal? (list\"Hi\"name(+ 1 2)) '(\"Hi\"\"max\"3))", "#t");
    run_test("(define lambda) (define + 5) (define <=?) (define a-b) +", "5");
    run_test("(define fOO 5) Foo", "5");
    run_test("(equal? #\\a #\\A)", "#f");
    run_test("(define x ; 5 \n 6) x", "6");
    run_test("((if #f = *) 3 4)", "12");
}

static void test_special_forms()
{
    run_test("((lambda (x) (+ x x)) 5)", "10");
    run_test("(define reverse-subtract (lambda (x y)(- y x)))(reverse-subtract 7 10)", "3");
    run_test("(define foo (let ((x 4)) (lambda (y) (+ x y)))) (foo 6)", "10");
    run_test("((named-lambda (f x) (+ x x)) 4)", "8");

    run_test("(let ((x 2) (y 3))(* x y))", "6");
    run_test("(let ((x 2) (y 3))(let ((foo (lambda (z) (+ x y z)))(x 7))(foo 4)))", "9");
    run_test("(let ((x 2) (y 3))(let* ((x 7)(z (+ x y)))(* z x))) ", "70");
    run_test("(letrec ((even?(lambda (n)(if (zero? n)#t(odd? (- n 1)))))"
             "(odd?(lambda (n)(if (zero? n)#f(even? (- n 1))))))(even? 88)) ", "#t");

    run_test("(define variable #t)(define (access-variable) variable)(let ((variable #f))(access-variable))", "#t");
    run_test("(define variable #t)(define (access-variable) variable)(let ((variable #f))(access-variable)) variable",
             "#t");
    run_test("(define variable #t)(define (access-variable) variable)(fluid-let ((variable #f))(access-variable))",
             "#f");
    run_test(
        "(define variable #t)(define (access-variable) variable)(fluid-let ((variable #f))(access-variable)) variable",
        "#t");

    run_test(
        "(let ((x 5)) (define foo (lambda (y) (bar x y))) (define bar (lambda (a b) (+ (* a b) a))) (foo (+ x 3)))",
        "45");
    run_test("(define x 2) (set! x 4) (+ x 1)", "5");
    run_test("(define x 2) ((lambda () (set! x 4))) (+ x 1)", "5");
    run_test("(define x 2) ((lambda () (define x 4))) (+ x 1)", "3");
    run_test("(equal? (list (quote a) (quote #(a b c)) (quote (+ 1 2))) '(a #(a b c) (+ 1 2)))", "#t");
    run_test("(equal? ''a '(quote a))", "#t");
    run_test("(equal? `(list ,(+ 1 2) 4) '(list 3 4))", "#t");
    run_test("(equal? (let ((name 'a)) `(list ,name ',name)) '(list a 'a))", "#t");
    run_test("(equal? `(a ,(+ 1 2) ,@(map abs '(4 -5 6)) b) '(a 3 4 5 6 b))", "#t");
    run_test("(equal? `((foo ,(- 10 3)) ,@(cdr '(c)) . ,(car '(cons))) '((foo 7) . cons))", "#t");
    run_test("(equal? `(a `(b ,(+ 1 2) ,(foo ,(+ 1 3) d) e) f) '(a `(b ,(+ 1 2) ,(foo 4 d) e) f))", "#t");
    run_test("(equal? (quasiquote (list (unquote (+ 1 2)) 4)) '(list 3 4))", "#t");
    run_test("(equal? '(quasiquote (list (unquote (+ 1 2)) 4)) ``(list ,(+ 1 2) 4))", "#t");
    run_test("(equal? '(quasiquote (list (unquote (+ 1 2)) 4)) ''(list ,(+ 1 2) 4))", "#f");
    run_test("(equal?  (let ((name1 'x)(name2 'y)) `(a `(b ,,name1 ,',name2 d) e)) "
             "'(a `(b ,x ,'y d) e))", "#t");
    run_test("`,(+ 2 3)", "5");

    run_test("(if (> 3 2) 'yes 'no)", "'yes");
    run_test("(if (< 3 2) 'yes 'no)", "'no");
    run_test("(cond ((> 3 2) 'greater)((< 3 2) 'less))", "'greater");
    run_test("(cond ((> 3 3) 'greater)((< 3 3) 'less)(else 'equal))", "'equal");
    run_test("(cond (#f => car) ((cons 1 2) => cdr) (else #f))", "2");

    run_test("(case (* 2 3)((2 3 5 7) 'prime)((1 4 6 8 9) 'composite))", "'composite");
    run_test("(case (car '(c d))((a e i o u) 'vowel)((w y) 'semivowel)(else 'consonant))", "'consonant");

    run_test("(equal? (list (and (= 2 2) (> 2 1)) (and (= 2 2) (< 2 1)) (and 1 2 'c '(f g)) (and)) "
             "'(#t #f (f g) #t))", "#t");
    run_test("(equal? (list (or (= 2 2) (> 2 1)) (or (= 2 2) (< 2 1)) (or #f #f #f) (or (memq 'b '(a b c)) (/ 3 0))) "
             "'(#t #t #f (b c)))", "#t");

    run_test("(define x 0) (begin (set! x 5)(+ x 1))", "6");
    run_test("(equal? (let loop((numbers '(3 -2 1 6 -5))(nonneg '())(neg '()))(cond ((null? numbers)(list nonneg neg))"
             "((>= (car numbers) 0)(loop (cdr numbers)(cons (car numbers) nonneg) neg)) (else"
             "(loop (cdr numbers) nonneg (cons (car numbers) neg))))) '((6 1 3) (-5 -2)))", "#t");
    run_test("(equal? (do ((vec (make-vector 5))(i 0 (+ i 1)))((= i 5) vec)(vector-set! vec i i)) '#(0 1 2 3 4))",
             "#t");
    run_test("(let ((x '(1 3 5 7 9)))(do ((x x (cdr x))(sum 0 (+ sum (car x))))((null? x) sum)))", "25");
}

int main()
{
    if(scheme_true == scheme_false || scheme_true->is_eq(scheme_false) || scheme_false->is_eq(scheme_true) ||
       evaluate_string_in_global_context("#t") != scheme_true ||
       evaluate_string_in_global_context("#f") != scheme_false ||
       std::make_shared<SchemeInt>(5)->is_eq(std::make_shared<SchemeInt>(6)) ||
       !evaluate_string_in_global_context("8")->is_eq(std::make_shared<SchemeInt>(8)))
    {
        std::cout << "Something weird happened" << std::endl;
        exit(1);
    }
    test_basic();
    test_special_forms();
}
