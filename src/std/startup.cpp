#include <string>
#include <schemeobject.h>
#include <sstream>
#include <parser.h>
#include <std.h>

const std::string startup =
    "(define (> x y) (< y x)) "
    "(define (>= x y) (not (< x y))) "
    "(define (<= x y) (not (< y x))) "
    "(define true #t) "
    "(define false #f) "
    "(define (not x) (if x #f #t)) "
    "(define false? not) "
    "(define boolean=? eq?) "
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
    "(define (map* init fun . args)"
    "  (define (cars l) (if (null? l) nil (cons (car (car l)) (cars (cdr l)))))"
    "  (define (cdrs l) (if (null? l) nil (cons (cdr (car l)) (cdrs (cdr l)))))"
    "  (define (nils l) (and (not (null? l)) (or (null? (car l)) (nils (cdr l)))))"
    "  (if (or (null? args) (nils args)) init (cons (apply fun (cars args)) (apply map* init fun (cdrs args))))) "
    "(define (map . args) (apply map* (cons '() args))) "
    "(define (for-each fun . args)"
    "  (define (cars l) (if (null? l) nil (cons (car (car l)) (cars (cdr l)))))"
    "  (define (cdrs l) (if (null? l) nil (cons (cdr (car l)) (cdrs (cdr l)))))"
    "  (define (nils l) (and (not (null? l)) (or (null? (car l)) (nils (cdr l)))))"
    "  (cond ((or (null? args) (nils args)) \"\") (else (apply fun (cars args)) (apply for-each fun (cdrs args))))) "
    "(define (append . x)"
    "  (define (f x y) (if (null? x) y (cons (car x) (f (cdr x) y))))"
    "  (define len (-1+ (length x))) "
    "  (if (null? x) nil (fold-right f (car (list-tail x len)) (list-head x len)))) "
    "(define (even? x) (= 0 (remainder x 2))) "
    "(define (odd? x) (not (even? x))) "
    "(define (filter predicate sequence)"
    "  (cond ((null? sequence) nil) ((predicate (car sequence)) (cons (car sequence)"
    "  (filter predicate (cdr sequence)))) (else (filter predicate (cdr sequence))))) "
    "(define (remove predicate sequence) (filter (lambda (x) (not (predicate x))) sequence)) "
    "(define (fold-right op initial sequence)"
    "  (if (null? sequence) initial (op (car sequence) (fold-right op initial (cdr sequence))))) "
    "(define (fold-left op initial sequence)"
    "  (define (iter result rest) (if (null? rest) result (iter (op result (car rest))"
    "  (cdr rest)))) (iter initial sequence)) "
    "(define (reduce-left op initial sequence) "
    "  (if (null? sequence) initial (fold-left op (first sequence) (list-tail sequence 1)))) "
    "(define (reduce-right op initial sequence) "
    "  (if (null? sequence) initial (let ((len (-1+ (length sequence)))) "
    "    (fold-right op (car (list-tail sequence len)) (list-head sequence len))))) "
    "(define (reverse items)"
    "  (fold-left (lambda (x y) (cons y x)) nil items)) "
    "(define (null? x) (eq? x nil)) "
    "(define (equal? a b)"
    "  (cond ((and (pair? a) (pair? b)) (and (equal? (car a) (car b)) (equal? (cdr a) (cdr b))))"
    "    ((and (string? a) (string? b)) (string=? a b))"
    "    ((and (vector? a) (vector? b)) (equal? (vector->list a)  (vector->list b)))"
    "    (else (eq? a b)))) "
    "(define (gcd a b) (if (= b 0) a (gcd b (remainder a b)))) "
    "(define (last-pair x) (if (pair? (cdr x)) (last-pair (cdr x)) x)) "
    "(define (append! x . y)"
    "  (define (f x y) (set-cdr! (last-pair x) y) x)"
    "  (if (null? y) x (f x (apply append! y)))) "
    "(define (stream . l) l) "
    "(define (list->stream l) l) "
    "(define (stream->list stream)"
    "  (if (stream-null? stream) '() (cons (stream-car stream) (stream->list (stream-cdr stream))))) "
    "(define stream-car car) "
    "(define stream-first car) "
    "(define (stream-cdr x) (force (cdr x))) "
    "(define stream-rest stream-cdr) "
    "(define stream-null? null?) "
    "(define (stream-length l) (length (stream->list l))) "
    "(define (stream-tail s k) (if (= k 0) s (stream-tail (stream-cdr s) (-1+ k)))) "
    "(define (stream-ref s k) (stream-car (stream-tail s k))) "
    "(define (stream-head s k) (if (= k 0) '() (cons (stream-car s) (stream-head (stream-cdr s) (-1+ k))))) "
    "(define the-empty-stream nil) "
    "(define (stream-pair? object) (and (pair? object) (promise? (cdr object)))) "
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
    "(define eqv? eq?) "
    "(define memq (member-procedure eq?)) "
    "(define memv (member-procedure eqv?)) "
    "(define member (member-procedure equal?)) "
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
    "(define (string-copy s) (string-append s)) "
    "(define (string-head s e) (substring s 0 e)) "
    "(define (string-tail s b) (substring s b (string-length s))) "
    "(define (substring->list s a b) (string->list (substring s a b))) "
    "(define (substring-downcase! s a b) "
    "  (do ((i a (1+ i))) ((= i b)) (string-set! s i (char-downcase (string-ref s i)))) s) "
    "(define (substring-upcase! s a b) "
    "  (do ((i a (1+ i))) ((= i b)) (string-set! s i (char-upcase (string-ref s i)))) s) "
    "(define (string-downcase! s) (substring-downcase! s 0 (string-length s))) "
    "(define (string-upcase! s) (substring-upcase! s 0 (string-length s))) "
    "(define (string-downcase s) (string-downcase! (string-copy s))) "
    "(define (string-upcase s) (string-upcase! (string-copy s))) "
    "(define (string-upper-case? s) (and (> (string-length s) 0) (string=? s (string-upcase s)))) "
    "(define (string-lower-case? s) (and (> (string-length s) 0) (string=? s (string-downcase s))))"
    "(define (substring-upper-case? s a b) (string-upper-case? (substring s a b))) "
    "(define (substring-lower-case? s a b) (string-lower-case? (substring s a b))) "
    "(define (substring=? s1 a1 b1 s2 a2 b2) (string=? (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (string-ci=? s t) (string=? (string-downcase s) (string-downcase t))) "
    "(define (substring-ci=? s1 a1 b1 s2 a2 b2) (string-ci=? (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (substring<? s1 a1 b1 s2 a2 b2) (string<? (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (string>? a b) (string<? b a)) "
    "(define (string<=? a b) (or (string<? a b) (string=? a b))) "
    "(define (string>=? a b) (or (string>? a b) (string=? a b))) "
    "(define (string-ci<? a b) (string<? (string-downcase a) (string-downcase b)))"
    "(define (substring-ci<? s1 a1 b1 s2 a2 b2) (string-ci<? (substring s1 a1 b1) (substring s2 a2 b2)))"
    "(define (string-ci>? a b) (string-ci<? b a)) "
    "(define (string-ci<=? a b) (or (string-ci<? a b) (string-ci=? a b))) "
    "(define (string-ci>=? a b) (or (string-ci>? a b) (string-ci=? a b))) "
    "(define (substring-search-forward p s a b) "
    "  (let ((res (string-search-forward p (substring s a b)))) (if res (+ res a) res))) "
    "(define (substring-search-backward p s a b) "
    "  (let ((res (string-search-backward p (substring s a b)))) (if res (+ res a) res))) "
    "(define (substring-search-all p s a b) "
    "  (map (lambda (x) (+ x a)) (string-search-all p (substring s a b)))) "
    "(define (substring? p s) (number? (string-search-forward p s))) "
    "(define (string-find-next-char s c) (string-search-forward (string c) s)) "
    "(define (substring-find-next-char s a b c) (substring-search-forward (string c) s a b)) "
    "(define (string-find-next-char-ci s c) "
    "  (string-find-next-char (string-downcase s) (char-downcase c))) "
    "(define (substring-find-next-char-ci s a b c) "
    "  (substring-find-next-char (string-downcase s) a b (char-downcase c))) "
    "(define (string-find-previous-char s c) "
    "  (let ((res (string-search-backward (string c) s))) (if res (- res 1) res))) "
    "(define (substring-find-previous-char s a b c) "
    "  (let ((res (substring-search-backward (string c) s a b))) (if res (- res 1) res))) "
    "(define (string-find-previous-char-ci s c) "
    "  (string-find-previous-char (string-downcase s) (char-downcase c))) "
    "(define (substring-find-previous-char-ci s a b c) "
    "  (substring-find-previous-char (string-downcase s) a b (char-downcase c))) "
    "(define (substring-match-forward s1 a1 b1 s2 a2 b2) "
    "  (string-match-forward (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (string-match-forward-ci s t) (string-match-forward (string-downcase s) (string-downcase t))) "
    "(define (substring-match-forward-ci s1 a1 b1 s2 a2 b2) "
    "  (string-match-forward-ci (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (substring-match-backward s1 a1 b1 s2 a2 b2) "
    "  (string-match-backkward (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (string-match-backward-ci s t) (string-match-backward (string-downcase s) (string-downcase t))) "
    "(define (substring-match-backward-ci s1 a1 b1 s2 a2 b2) "
    "  (string-match-backward-ci (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (string-prefix? s t) (= (string-match-forward s t) (string-length s))) "
    "(define (substring-prefix? s1 a1 b1 s2 a2 b2) "
    "  (string-prefix? (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (string-prefix-ci? s t) (string-prefix? (string-downcase s) (string-downcase t))) "
    "(define (substring-prefix-ci? s1 a1 b1 s2 a2 b2) "
    "  (string-prefix-ci? (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (string-suffix? s t) (= (string-match-backward s t) (string-length s))) "
    "(define (substring-suffix? s1 a1 b1 s2 a2 b2) "
    "  (string-suffix? (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (string-suffix-ci? s t) (string-suffix? (string-downcase s) (string-downcase t))) "
    "(define (substring-suffix-ci? s1 a1 b1 s2 a2 b2) "
    "  (string-suffix-ci? (substring s1 a1 b1) (substring s2 a2 b2))) "
    "(define (substring-replace! s a b c1 c2) "
    "  (do ((i a (1+ i))) ((= i b)) (if (eq? c1 (string-ref s i)) (string-set! s i c2))) s)"
    "(define (string-replace! s c1 c2) (substring-replace! s 0 (string-length s) c1 c2))"
    "(define (substring-replace s a b c1 c2) (substring-replace! (string-copy s) a b c1 c2))"
    "(define (string-replace s c1 c2) (string-replace! (string-copy s) c1 c2)) "
    "(define (substring-fill! s a b c) (do ((i a (1+ i))) ((= i b)) (string-set! s i c)) s)"
    "(define (string-fill! s c) (substring-fill! s 0 (string-length s) c)) "
    "(define (substring-move-left! s1 a b s2 d) "
    "  (do ((i a (1+ i)) (j d (1+ j))) ((= i b)) (string-set! s2 j (string-ref s1 i))) s2) "
    "(define (substring-move-right! s1 a b s2 d) "
    "  (do ((i (-1+ b) (-1+ i)) (j (- d 1 (- a b)) (-1+ j))) ((< i a)) (string-set! s2 j (string-ref s1 i))) s2) "
    "(define (reverse-string! s) (reverse-substring! s 0 (string-length s))) "
    "(define (reverse-string s) (reverse-string! (string-copy s))) "
    "(define (reverse-substring s a b) (reverse-string (substring s a b))) "
    "(define (general-car-cdr l x) "
    "  (cond ((< x 2) l) ((= 0 (remainder x 2)) (general-car-cdr (cdr l) (quotient x 2))) "
    "    (else (general-car-cdr (car l) (quotient x 2))) )) "
    "(define (make-initialized-list k proc) "
    "  (define (iter i l) (if (< i 0) l (iter (-1+ i) (cons (proc i) l)))) (iter (-1+ k) '())) "
    "(define (list-tail l k) (if (= k 0) l (list-tail (cdr l) (-1+ k)))) "
    "(define (list-ref l k) (car (list-tail l k))) "
    "(define (first l) (list-ref l 0)) (define (second l) (list-ref l 1)) (define (third l) (list-ref l 2)) "
    "(define (fourth l) (list-ref l 3)) (define (fifth l) (list-ref l 4)) (define (sixth l) (list-ref l 5)) "
    "(define (sublist l a b) (list-head (list-tail l a) (- b a))) "
    "(define (list-deletor predicate) (lambda (list) (remove predicate list))) "
    "(define (delete-member-procedure del predicate) "
    "  (lambda (elem list) ((del (lambda (e) (predicate e elem))) list))) "
    "(define delq (delete-member-procedure list-deletor eq?)) "
    "(define delv (delete-member-procedure list-deletor eqv?)) "
    "(define delete (delete-member-procedure list-deletor equal?)) "
    "(define (append-map . args) (apply append (apply map args))) "
    "(define (append-map! . args) (apply append! (apply map args))) "
    "(define (any . args) (fold-left (lambda (x y) (or x y)) #f (apply map args))) "
    "(define (every . args) (fold-left (lambda (x y) (and x y)) #t (apply map args))) "
    "(define (circular-list . objects) (append! objects objects)) "
    "(define (vector . args) (list->vector args)) "
    "(define (vector-copy v) (list->vector (vector->list v))) "
    "(define (make-initialized-vector k proc) (list->vector (make-initialized-list k proc))) "
    "(define (vector-map proc v) (list->vector (map proc (vector->list v)))) "
    "(define (vector-first v) (vector-ref v 0)) (define (vector-second v) (vector-ref v 1)) "
    "(define (vector-third v) (vector-ref v 2)) (define (vector-fourth v) (vector-ref v 3)) "
    "(define (vector-binary-search vec cmp unwrap key) "
    "  (if (= 0 (vector-length vec)) #f (begin "
    "  (define (iter l r) (if (>= (+ l 1) r) l (begin (define m (quotient (+ l r) 2)) "
    "    (define val (vector-ref vec m)) (if (cmp key (unwrap val)) (iter l m) (iter m r))))) "
    "  (define found (vector-ref vec (iter 0 (vector-length vec)))) "
    "  (if (or (cmp key (unwrap found)) (cmp (unwrap found) key)) #f found)))) "
    "(define (vector-head v e) (subvector v 0 e)) "
    "(define (vector-tail v s) (subvector v s (vector-length v))) "
    "(define (subvector-fill! vec a b obj) (do ((i a (1+ i))) ((= i b)) (vector-set! vec i obj)) vec)"
    "(define (vector-fill! vec obj) (subvector-fill! vec 0 (vector-length vec) obj)) "
    "(define (subvector-move-left! v1 a b v2 d) "
    "  (do ((i a (1+ i)) (j d (1+ j))) ((= i b)) (vector-set! v2 j (vector-ref v1 i))) v2) "
    "(define (subvector-move-right! v1 a b v2 d) "
    "  (do ((i (-1+ b) (-1+ i)) (j (- d 1 (- a b)) (-1+ j))) ((< i a)) (vector-set! v2 j (vector-ref v1 i))) v2) "
    "(define sort! merge-sort!) "
    "(define (boolean/and . args) (if (fold-left (lambda (a b) (and a b)) #t args) #t #f)) "
    "(define (boolean/or . args) (if (fold-left (lambda (a b) (or a b)) #f args) #t #f)) "
    "(define (intern s) (string->symbol (string-downcase s))) "
    "(define intern-soft intern) "
    "(define (symbol-append a b) (string->symbol (string-append (symbol->string a) (symbol->string b)))) "
    "(define (symbol<? a b) (string<? (symbol->string a) (symbol->string b))) "
    "(define (default-object? o) (eq? o (string->symbol \"\"))) "
    "(define (make-procedure-arity min #!optional max s) (if (default-object? max) (set! max #f)) "
    "  (if (eq? max min) (if (eq? s #t) min (cons min min)) (cons min max))) "
    "(define (procedure-arity? x) (cond ((exact? x) (>= x 0)) ((pair? x) "
    "  (or (and (exact? (car x)) (exact? (cdr x)) (<= (car x) (cdr x)) (>= (car x) 0)) "
    "  (and (exact? (car x)) (>= (car x) 0) (eq? (cdr x) #f)))) (else #f))) "
    "(define (procedure-arity-min a) (if (exact? a) a (car a))) "
    "(define (procedure-arity-max a) (if (exact? a) a (cdr a)))"
    "";


Context init_global_context()
{
    Context global_context;
    global_context.new_frame();
    for(auto p : SpecialFormRegistry::all())
    {
        global_context.set(p.first, std::make_shared<SchemeSpecialForm>(p.first));
    }
    for(auto p : FunctionRegistry::all())
    {
        global_context.set(p.first, std::make_shared<SchemePrimitiveProcedure>(p.first,
                                                                               std::get<0>(p.second),
                                                                               std::get<1>(p.second)));
    }
    std::istringstream st;
    st.str(startup);
    while(1)
    {
        auto x = read_object(st);
        if(x.result == parse_result_t::END)
            break;
        if(x.result == parse_result_t::ERROR)
        {
            std::cout << "Error during initialization: " << x.error;
            exit(1);
        }
        x.node->evaluate(global_context);
    }
    global_context.new_frame();
    global_context.set("user-initial-environment", std::make_shared<SchemeEnvironment>(global_context));
    return global_context;
}
