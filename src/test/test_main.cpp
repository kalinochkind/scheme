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
    run_test("(define x (lambda x (cdr x))) (equal? (x 1 2 3) '(2 3))", "#t");
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

static void test_equality()
{
    run_test("(define (f x) (+ x x)) (define l '(1 2 q)) (define p (cons car cdr)) (define v '#(1 2 q)) "
             "(define s \"qwerty\") (define c (make-cell 5)) (define wp (weak-cons 42 37)) "
             "(and (eq? #t #t) (eq? #f #f) (eq? 'QWE 'Qwe) (eq? 7 7) (eq? 4.6 4.6) (eq? #\\o #\\o) (eq? cons cons) "
             "(eq? '() '()) (eq? f f) (eq? l l) (eq? p p) (eq? v v) (eq? s s) (eq? c c) (eq? wp wp))", "#t");
    run_test("(or (eq? '(5 5) 'kek) (eq? #t #f) (eq? 'q 'w) (eq? 4 4.0) (eq? 4 5) (eq? 4.5 5.4) (eq? #\\A #\\B) "
             "(eq? '() '(())) (eq? (lambda x x) (lambda x 0)) (eq? (cons 1 1) (cons 1 1)) "
             "(eq? '#() '#()) (eq? \"qw\" \"qw\") (eq? (make-cell '()) (make-cell '())))", "#f");
    run_test("(define gen-counter (lambda () (let ((n 0)) (lambda () (set! n (+ n 1)) n)))) (let ((g (gen-counter))) "
             "(eqv? g g))", "#t");
    run_test("(define gen-counter (lambda () (let ((n 0)) (lambda () (set! n (+ n 1)) n)))) "
             "(eqv? (gen-counter) (gen-counter))", "#f");
    run_test("(and (equal? 'a 'a) (equal? '(a) '(a)) (equal? '(a (b) \"c\") '(a (b) \"c\")) "
             "(equal? (make-vector 5 'a) (make-vector 5 'a)))", "#t");
    run_test("(or (equal? 'a 'b) (equal? 'a '(a)) (equal? '(a (b) \"c\") '((b) a \"c\")) "
             "(equal? (make-vector 5 'a) (make-vector 6 'a)) (equal? \"a\" \"A\"))", "#f");
}

static void test_math()
{
    run_test("(equal? (list (number? 5) (number? 5.) (number? #\\5) (exact? 5) (exact? 5.) (inexact? 5) (inexact? 5.) "
             "(integer? 5) (integer? 5.) (integer? 5.5)) '(#t #t #f #t #f #f #t #t #t #f))", "#t");
    run_test("(equal? (list (< 1 2) (<= 1 2) (> 1 2) (>= 1 2) (= 1 2) (< 2 1) (<= 2 1) (> 2 1) (>= 2 1) (= 2 1) "
             "(< 2 2.) (<= 2 2.) (> 2 2.) (>= 2 2.) (= 2 2.)) '(#t #t #f #f #f #f #f #t #t #f #f #t #f #t #t))", "#t");
    run_test("(equal? (list (zero? 5) (positive? 5) (negative? 5) (zero? -5) (positive? -5) (negative? -5) (zero? 0)) "
             "'(#f #t #f #f #f #t #t))", "#t");
    run_test("(equal? (list (+ 3 4 5) (+ 3 4) (+ 3) (+) (* 3 4 5) (* 3 4) (* 3) (*) (- 3 4 5) (- 3 4) (- 3) (/ 3 4)) "
             "'(12 7 3 0 60 12 3 1 -6 -1 -3 0.75))", "#t");
    run_test("(1+ (-1+ 5))", "5");
    run_test("(equal? (list (abs 3) (abs -3) (quotient 13 4) (modulo 13 4) (remainder 13 4) "
             "(quotient -13 4) (modulo -13 4) (remainder -13 4) (quotient 13 -4) (modulo 13 -4) (remainder 13 -4)"
             "(quotient -13 -4) (modulo -13 -4) (remainder -13 -4)) '(3 3 3 1 1 -3 3 -1 -3 -3 1 3 -1 -1))", "#t");
    run_test("(string->number \"13.5\")", "13.5");
    run_test("(string->number \"13\")", "13");
    run_test("(equal? (number->string 13) \"13\")", "#t");
    run_test("(rational? -2/3)", "#t");
    run_test("5/2", "2.5");

}

static void test_strings()
{
    run_test("(equal? (string #\\a #\\A #\\spAce #\\LF #\\newline #\\() \"aA \n\\n(\")", "#t");
    run_test("(equal? (char->name #\\Space) \"Space\")", "#t");
    run_test("(equal? (name->char \"Del\") #\\DEL)", "#t");
    run_test("(equal? (list (char=? #\\a #\\A) (char=? #\\a #\\a) (char=? #\\a #\\b) (char-ci=? #\\a #\\A) "
             "(char-ci=? #\\a #\\a) (char-ci=? #\\a #\\b)) '(#f #t #f #t #t #f))", "#t");
    run_test("(equal? (list (char<? #\\a #\\a) (char<? #\\a #\\b) (char-ci<? #\\a #\\A) "
             "(char-ci<? #\\a #\\a) (char-ci<? #\\A #\\b) (char-ci<? #\\a #\\B)) '(#f #t #f #f #t #t))", "#t");
    run_test("(equal? (string (char-upcase #\\a) (char-upcase #\\A) (char-upcase #\\5) (char-downcase #\\a) "
             "(char-downcase #\\A) (char-downcase #\\5)) \"AA5aa5\")", "#t");

    run_test("(equal? (make-string 5 #\\k) \"kkkkk\")", "#t");
    run_test("(equal? (list->string (list #\\q #\\w)) \"qw\")", "#t");
    run_test("(define a \"abc\") (define b a) (define c (string-copy a)) (string-set! a 0 #\\Q) "
             "(equal? (list (eq? a b) (eq? a c) (eq? (string-ref b 0) #\\Q) (eq? (string-ref c 0) #\\Q)) '(#t #f #t #f))",
             "#t");
    run_test("(string-length \"qasw\")", "4");
    run_test("(string=? \"PIE\" \"PIE\") ", "#t");
    run_test("(string=? \"PIE\" \"pie\") ", "#f");
    run_test("(string-ci=? \"PIE\" \"pie\")", "#t");
    run_test("(substring=? \"Alamo\" 1 3 \"cola\" 2 4)", "#t");
    run_test("(string<? \"cat\" \"dog\")", "#t");
    run_test("(string<? \"cat\" \"DOG\")", "#f");
    run_test("(string-ci<? \"cat\" \"DOG\")", "#t");
    run_test("(string>? \"catkin\" \"cat\")", "#t");

    run_test("(equal? (map string-upper-case?  '(\"\"    \"A\"    \"art\"  \"Art\"  \"ART\")) '(#f #t #f #f #t))", "#t");
    run_test("(define str \"ABCDEFG\") (substring-downcase! str 3 5) (equal? str \"ABCdeFG\")", "#t");

    run_test("(equal? (list (string-append) (string-append \"*\" \"ace\" \"*\") (string-append \"\" \"\" \"\")) "
             "'(\"\" \"*ace*\" \"\"))", "#t");
    run_test("(equal? (substring \"arduous\" 2 5) \"duo\")", "#t");
    run_test("(equal? (string-tail \"uncommon\" 2) \"common\")", "#t");
    run_test("(equal? (list (string-pad-left \"hello\" 4) (string-pad-left \"hello\" 8) (string-pad-left \"hello\" 8 #\\*) "
             "(string-pad-right \"hello\" 4) (string-pad-right \"hello\" 8)) '(\"ello\" \"   hello\" \"***hello\" \"hell\" \"hello   \"))", "#t");

    run_test("(string-search-forward \"rat\" \"pirate rating\")", "2");
    run_test("(substring-search-forward \"rat\" \"pirate rating\" 4 13)", "7");
    run_test("(substring-search-forward \"rat\" \"pirate rating\" 9 13)", "#f");
    run_test("(string-search-backward \"rat\" \"pirate rating\")", "10");
    run_test("(substring-search-backward \"rat\" \"pirate rating\" 1 8)", "5");
    run_test("(substring-search-backward \"rat\" \"pirate rating\" 9 13)", "#f");
    run_test("(equal? (list (string-search-all \"rat\" \"pirate\") (string-search-all \"rat\" \"pirate rating\") "
             "(substring-search-all \"rat\" \"pirate rating\" 4 13) (substring-search-all \"rat\" \"pirate rating\" 9 13)) "
             "'((2) (2 7) (7) ()))", "#t");
    run_test("(substring? \"rat\" \"pirate\")", "#t");
    run_test("(substring? \"rat\" \"outrage\")", "#f");
    run_test("(substring? \"\" \"outrage\")", "#t");

    run_test("(string-find-next-char \"Adam\" #\\A)", "0");
    run_test("(substring-find-next-char \"Adam\" 1 4 #\\A)", "#f");
    run_test("(substring-find-next-char-ci \"Adam\" 1 4 #\\A)", "2");

    run_test("(string-match-forward \"mirror\" \"micro\")", "2");
    run_test("(string-match-forward \"a\" \"b\")", "0");
    run_test("(string-match-backward-ci \"BULBOUS\" \"fractious\")", "3");
    run_test("(string-prefix? \"abc\" \"abcdef\")", "#t");
    run_test("(string-prefix? \"\" \"afhkm\")", "#t");
    run_test("(string-suffix? \"ous\" \"bulbous\")", "#t");
    run_test("(string-suffix? \"\" \"bulbous\")", "#t");

    run_test("(define str \"a few words\") (equal? (list (string-replace str #\\space #\\-) (substring-replace str 2 9 #\\space #\\-) str) "
             "'(\"a-few-words\" \"a few-words\" \"a few words\"))", "#t");
    run_test("(define str \"a few words\") (string-replace! str #\\space #\\-) (equal? str \"a-few-words\")", "#t");
    run_test("(define s (make-string 10 #\\space)) (substring-fill! s 2 8 #\\*) (equal? s \"  ******  \")", "#t");
    run_test("(define answer (make-string 9 #\\*)) (substring-move-left! \"start\" 0 5 answer 0) (equal? answer \"start****\")", "#t");
    run_test("(define answer \"start****\") (substring-move-right! \"-end\" 0 4 answer 5) (equal? answer \"start-end\")", "#t");
    run_test("(equal? (reverse-string \"foo bar baz\") \"zab rab oof\")", "#t");
    run_test("(equal? (reverse-substring \"foo bar baz\" 4 7) \"rab\")", "#t");
    run_test("(equal? (let ((foo \"foo bar baz\")) (reverse-string! foo) foo) \"zab rab oof\")", "#t");
    run_test("(equal? (let ((foo \"foo bar baz\")) (reverse-substring! foo 4 7) foo) \"foo rab baz\")", "#t");

    run_test("(define s \"qwertyuiop\") (set-string-length! s 5) (equal? s \"qwert\")", "#t");
    run_test("(define s \"qwerty\") (set-string-length! s 10) (string-length s)", "10");
}

static void test_lists()
{
    run_test("(define x (list 'a 'b 'c)) (define y x) (define r1 (list? y)) (set-cdr! x 4) (define r2 (list? y)) "
             "(set-cdr! x x) (define r3 (list? y)) (equal? (list r1 r2 r3) '(#t #f #f))", "#t");
    run_test("(pair? '())", "#f");
    run_test("(pair? '#(1 2))", "#f");
    run_test("(pair? '(1 2))", "#t");
    run_test("(pair? '(1 . 2))", "#t");
    run_test("(equal? (car '((a) b c d)) '(a))", "#t");
    run_test("(equal? (cdr '((a) b c d)) '(b c d))", "#t");
    run_test("(equal? (vector->list '#(dah dah didah)) '(dah dah didah))", "#t");
    run_test("(equal? (string->list \"abcd\") '(#\\a #\\b #\\c #\\d))", "#t");
    run_test("(equal? (substring->list \"abcdef\" 1 3) '(#\\b #\\c))", "#t");
    run_test("(length '(a (b) (c d e)))", "3");
    run_test("(null? '())", "#t");
    run_test("(null? '(()))", "#f");
    run_test("(list-ref '(a b c d) 2)", "'c");
    run_test("(equal? (sublist '(a b c d e f) 1 4) '(b c d))", "#t");
    run_test("(equal? (list-head '(a b c d e f) 2) '(a b))", "#t");
    run_test("(define l '(a b c d e f)) (eqv? (list-tail l 2) (cddr l))", "#t");
    run_test("(equal? (append '(a (b)) '((c))) '(a (b) (c)))", "#t");
    run_test("(append)", "nil");
    run_test("(equal? (append '(a b) '(c . d)) '(a b c . d))", "#t");
    run_test("(append '() 'a)", "'a");
    run_test("(define x '(a b c)) (define y '(d e f)) (define z '(g h)) (equal? (list (append! x y z) x y z)"
             "'((a b c d e f g h) (a b c d e f g h) (d e f g h) (g h)))", "#t");
    run_test("(equal? (filter odd? '(1 2 3 4 5)) '(1 3 5))", "#t");
    run_test("(equal? (remove odd? '(1 2 3 4 5)) '(2 4))", "#t");

    run_test("(equal? (memq 'a '(a b c)) '(a b c))", "#t");
    run_test("(equal? (memq 'b '(a b c)) '(b c))", "#t");
    run_test("(memq 'd '(a b c))", "#f");
    run_test("(memq (list 'a) '(b (a) c))", "#f");
    run_test("(equal? (member (list 'a) '(b (a) c)) '((a) c))", "#t");
    run_test("(equal? (memv 101 '(100 101 102)) '(101 102))", "#t");

    run_test("(equal? (map cadr '((a b) (d e) (g h))) '(b e h))", "#t");
    run_test("(equal? (map + '(1 2 3) '(4 5 6)) '(5 7 9))", "#t");
    run_test("(equal? (let ((v (make-vector 5))) (for-each (lambda (i) (vector-set! v i (* i i))) '(0 1 2 3 4))v) "
             "'#(0 1 4 9 16))", "#t");

    run_test("(reduce-left + 0 '(1 2 3 4))", "10");
    run_test("(reduce-left + 0 '(foo))", "'foo");
    run_test("(reduce-left + 0 '())", "0");
    run_test("(equal? (reduce-left list '() '(1 2 3 4)) '(((1 2) 3) 4))", "#t");
    run_test("(equal? (reduce-right list '() '(1 2 3 4)) '(1 (2 (3 4))))", "#t");
    run_test("(equal? (fold-right list '() '(1 2 3 4)) '(1 (2 (3 (4 ())))))", "#t");
    run_test("(equal? (fold-left list '() '(1 2 3 4)) '((((() 1) 2) 3) 4))", "#t");

    run_test("(any integer? '(a 3 b 2.7))", "#t");
    run_test("(every integer? '(a 3 b 2.7))", "#f");
    run_test("(any integer? '(a 3.1 b 2.7))", "#f");
    run_test("(every integer? '(1 2 3. 4))", "#t");
    run_test("(any < '(3 1 4 1 5) '(2 7 1 8 2))", "#t");
    run_test("(every < '(3 1 4 1 5) '(2 7 1 8 2))", "#f");

    run_test("(equal? (reverse '(a (b c) d (e (f)))) '((e (f)) d (b c) a))", "#t");
}

static void test_vectors()
{
    run_test("(equal? (make-vector 5 'a) '#(a a a a a))", "#t");
    run_test("(equal? (vector 5 'a) '#(5 a))", "#t");
    run_test("(define a (list->vector '(1 2 3))) (define b a) (define c (vector-copy a)) (vector-set! a 0 0) "
             "(equal? (list (eq? a b) (eq? a c) (vector-ref b 0) (vector-ref c 0) (vector-length b)) '(#t #f 0 1 3))", "#t");
    run_test("(equal? (make-initialized-vector 5 (lambda (x) (* x x))) '#(0 1 4 9 16))", "#t");
    run_test("(vector-length (vector-grow '#(1 2 3) 5))", "5");
    run_test("(equal? (vector-map cadr '#((a b) (d e) (g h))) '#(b e h))", "#t");
    run_test("(vector-ref '#(1 1 2 3 5 8 13 21) 5)", "8");
    run_test("(equal? (let ((vec (vector 0 '(2 2 2 2) \"Anna\"))) (vector-set! vec 1 '(\"Sue\" \"Sue\")) vec) "
             "'#(0 (\"Sue\" \"Sue\") \"Anna\"))", "#t");
    run_test("(equal? (subvector '#(0 1 2 3 4 5) 2 4) '#(2 3))", "#t");
    run_test("(equal? (vector-head '#(0 1 2 3 4 5) 2) '#(0 1))", "#t");
    run_test("(equal? (vector-tail '#(0 1 2 3 4 5) 2) '#(2 3 4 5))", "#t");
    run_test("(define v '#(0 1 2 3 4)) (subvector-fill! v 1 3 'q) (equal? v '#(0 q q 3 4))", "#t");
}

static void test_misc()
{
    run_test("(not 5)", "#f");
    run_test("(not #f)", "#t");
    run_test("(boolean/and #f #t #t)", "#f");
    run_test("(boolean/and 'f #t #t)", "#t");
    run_test("(boolean/or #f)", "#f");
    run_test("(boolean/or #f #t #t)", "#t");

    run_test("(symbol? 'car)", "#t");
    run_test("(symbol? \"car\")", "#f");
    run_test("(symbol? (string->uninterned-symbol \"car\"))", "#t");
    run_test("(equal? \"Kek\" (symbol->string (string->symbol \"Kek\")))", "#t");
    run_test("(equal? 'Kek (string->symbol \"Kek\"))", "#f");
    run_test("(equal? 'Kek (intern \"Kek\"))", "#t");
    run_test("'Kek", "'kek");
    run_test("(eq? (string->uninterned-symbol \"car\") (string->uninterned-symbol \"car\"))", "#f");
    run_test("(symbol-append 'foo- 'bar)", "'foo-bar");
    run_test("(symbol-append 'foo- (string->uninterned-symbol \"baz\"))", "'foo-baz");
    run_test("(symbol-append 'foo- (string->symbol \"BAZ\"))", "(string->symbol \"foo-BAZ\")");
    run_test("(symbol<? 'a 'b)", "#t");
    run_test("(symbol<? 'B 'a)", "#f");

    run_test("(cell? (make-cell make-cell))", "#t");
    run_test("(cell-contents (make-cell 42))", "42");
    run_test("(define c (make-cell 42)) (set-cell-contents! c 37) (cell-contents c)", "37");

    run_test("(force (delay (+ 1 2)))", "3");
    run_test("(equal? (let ((p (delay (+ 1 2)))) (list (force p) (force p))) '(3 3))", "#t");
    run_test("(define p (delay 5)) (promise-forced? p)", "#f");
    run_test("(define p (delay 5)) (force p) (promise-forced? p)", "#t");
    run_test("(define p (delay 5)) (force p) (promise-value p)", "5");
    run_test("(define count 0) (define p (delay (begin (set! count (+ count 1)) (* x 3)))) (define x 5) "
             "(force p) (force p) count", "1");

    run_test("(define p (weak-cons 37 42)) (weak-pair/car? p)", "#f");
    run_test("(define a 37) (define p (weak-cons a 42)) (weak-pair/car? p)", "#t");
    run_test("(define a 37) (define p (weak-cons a 42)) (weak-car p)", "37");
    run_test("(define a 37) (define p (weak-cons a 42)) (define a) (weak-pair/car? p)", "#f");
    run_test("(define a 37) (define p (weak-cons a 42)) (define a) (weak-car p)", "#f");
    run_test("(define p (weak-cons 37 42)) (weak-set-car! p car) (weak-car p)", "car");
    run_test("(define p (weak-cons 37 42)) (weak-cdr p)", "42");
    run_test("(define p (weak-cons 37 42)) (weak-set-cdr! p 337) (weak-cdr p)", "337");
}

static void test_procedures()
{
    run_test("(apply + 3 4 '(5 6))", "18");
    run_test("(procedure? car)", "#t");
    run_test("(primitive-procedure? car)", "#t");
    run_test("(compound-procedure? car)", "#f");
    run_test("(procedure? (lambda x x))", "#t");
    run_test("(primitive-procedure? (lambda x x))", "#f");
    run_test("(compound-procedure? (lambda x x))", "#t");
    run_test("(equal? (procedure-arity (lambda () 3)) '(0 . 0))", "#t");
    run_test("(equal? (procedure-arity (lambda (x) 3)) '(1 . 1))", "#t");
    run_test("(equal? (procedure-arity car) '(1 . 1))", "#t");
    run_test("(equal? (procedure-arity /) '(1 . #f))", "#t");
    run_test("(equal? (procedure-arity (lambda (x #!optional y) x)) '(1 . 2))", "#t");
    run_test("(equal? (procedure-arity (lambda (x #!optional y q #!rest z) x)) '(1 . #f))", "#t");
    run_test("(equal? (procedure-arity make-vector) '(1 . 2))", "#t");
    run_test("((make-primitive-procedure 'car) '(1 2))", "1");
    run_test("(primitive-procedure-name car)", "'car");
    run_test("(define (f x #!optional y) (cons x y)) (equal? (f 1 2) '(1 . 2))", "#t");
    run_test("(define (f x #!optional y) (cons x y)) (default-object? (cdr (f 1 2)))", "#f");
    run_test("(define (f x #!optional y) (cons x y)) (default-object? (cdr (f 1)))", "#t");

    run_test("(environment? (the-environment))", "#t");
    run_test("(environment-has-parent? (the-environment))", "#t");
    run_test("(environment-has-parent? (environment-parent (the-environment)))", "#f");
    run_test("(define x) (equal? (environment-bound-names (the-environment)) '(x))", "#t");
    run_test("(define x) (define y 5) (equal? (environment-bindings (the-environment)) '((x) (y 5)))", "#t");
    run_test("(environment-reference-type (the-environment) 'map)", "'normal");
    run_test("(environment-reference-type (the-environment) 'kek)", "'unbound");
    run_test("(environment-reference-type (the-environment) 'define)", "'macro");
    run_test("(define define) (environment-reference-type (the-environment) 'define)", "'unassigned");
    run_test("(define define) (environment-reference-type (environment-parent (the-environment)) 'define)", "'macro");
    run_test("(define define) (environment-bound? (the-environment) 'define)", "#t");
    run_test("(define define) (environment-assigned? (the-environment) 'define)", "#f");
    run_test("(define x 5) (environment-lookup (the-environment) 'x)", "5");
    run_test("(define x 5) (environment-assign! (the-environment) 'x 6) x", "6");
    run_test("(environment-define (the-environment) 'x 6) x", "6");
    run_test("(define x 5) (eval '(+ x x) (the-environment))", "10");
    run_test("(environment-has-parent? user-initial-environment)", "#t");
    run_test("(environment-has-parent? system-global-environment)", "#f");
    run_test("(define x 5) (environment-lookup (nearest-repl/environment) 'x)", "5");
    run_test("(define map 5) (ge system-global-environment) (procedure? map)", "#t");
    run_test("(define map 5) (ge system-global-environment) (ge user-initial-environment) (procedure? map)", "#f");
    run_test("(define env (make-root-top-level-environment '(x) '(5))) (ge env) x", "5");
    run_test("(define env (make-top-level-environment '(x) '(5))) (ge env) (procedure? map)", "#t");
    run_test("(define env (make-top-level-environment '(x) '(5))) (ge env) x", "5");
}

static void test_io()
{
    run_test("(and (port? console-i/o-port) (i/o-port? console-i/o-port) (input-port? (current-input-port)) "
             "(output-port? (current-output-port)))", "#t");
    run_test("(define s (open-input-string \"qwe\\nrty\\nuio\")) (close-all-open-files) (read-line s) "
             "(equal? (read-line s) \"rty\")", "#t");
    run_test("(define s (open-output-string)) (close-all-open-files) (display 'qwe s) (newline s) "
             "(equal? (get-output-string s) \"qwe\\n\")", "#t");
    run_test("(equal? (with-input-from-string \"(a b c) (d e f)\" read) '(a b c))", "#t");
    run_test("(equal? (call-with-output-string (lambda (p) (display 42 p))) \"42\")", "#t");
    run_test("(define i (open-input-string \"qwe rty\")) (define o (open-output-string)) (set-current-input-port! i) "
             "(set-current-output-port! o) (write (read-char)) (equal? (get-output-string o) \"#\\\\q\")", "#t");
    run_test("(equal? (with-output-to-string (lambda () (write 'abc))) \"abc\")", "#t");
    run_test("(equal? (write-to-string \"1 2 3\") \"\\\"1 2 3\\\"\")", "#t");
    run_test("(equal? (write-to-string ''(a b c)) \"'(a b c)\")", "#t");
    run_test("(equal? (write-to-string '(quote . 1)) \"(quote . 1)\")", "#t");
    run_test("(equal? (with-input-from-string \"qw\" (lambda () "
             "(list (peek-char) (read-char) (read-char) (peek-char) (read-char)))) '(#\\q #\\q #\\w eof eof))", "#t");
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
    test_equality();
    test_math();
    test_strings();
    test_lists();
    test_vectors();
    test_misc();
    test_procedures();
    test_io();
}
