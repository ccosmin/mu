#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "cell.hpp"
#include "interpreter.hpp"

using namespace mu;

static std::string Eval(Interpreter& i, const std::string& expr)
{
  return i.Eval(expr).ToString();
}

TEST_CASE("Original tests", "[interpreter]")
{
  Interpreter i;
  REQUIRE(Eval(i, "(quote (testing 1 (2.0) -3.14e159))") == "(testing 1 (2.0) -3.14e159)");
  REQUIRE(Eval(i, "(+ 2 2)") == "4");
  REQUIRE(Eval(i, "(+ (* 2 100) (* 1 10))") == "210");
  REQUIRE(Eval(i, "(if (> 6 5) (+ 1 1) (+ 2 2))") == "2");
  REQUIRE(Eval(i, "(if (< 6 5) (+ 1 1) (+ 2 2))") == "4");
  REQUIRE(Eval(i, "(define x 3)") == "3");
  REQUIRE(Eval(i, "x") == "3");
  REQUIRE(Eval(i, "(+ x x)") == "6");
  REQUIRE(Eval(i, "(begin (define x 1) (set! x (+ x 1)) (+ x 1))") == "3");
  REQUIRE(Eval(i, "((lambda (x) (+ x x)) 5)") == "10");
  REQUIRE(Eval(i, "(define twice (lambda (x) (* 2 x)))") == "<Lambda>");
  REQUIRE(Eval(i, "(twice 5)") == "10");
  REQUIRE(Eval(i, "(define compose (lambda (f g) (lambda (x) (f (g x)))))") == "<Lambda>");
  REQUIRE(Eval(i, "((compose list twice) 5)") == "(10)");
  REQUIRE(Eval(i, "(define repeat (lambda (f) (compose f f)))") == "<Lambda>");
  REQUIRE(Eval(i, "((repeat twice) 5)") == "20");
  REQUIRE(Eval(i, "((repeat (repeat twice)) 5)") == "80");
  REQUIRE(Eval(i, "(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))") == "<Lambda>");
  REQUIRE(Eval(i, "(fact 3)") == "6");
  //REQUIRE(Eval(i, "(fact 50)") == "30414093201713378043612608166064768844377641568960512000000000000");
  REQUIRE(Eval(i, "(fact 12)") == "479001600"); // no bignums; this is as far as we go with 32 bits
  REQUIRE(Eval(i, "(define abs (lambda (n) ((if (> n 0) + -) 0 n)))") == "<Lambda>");
  REQUIRE(Eval(i, "(list (abs -3) (abs 0) (abs 3))") == "(3 0 3)");
  REQUIRE(Eval(i, "(define combine (lambda (f)"
           "(lambda (x y)"
              "(if (null? x) (quote ())"
              "(f (list (car x) (car y))"
              "((combine f) (cdr x) (cdr y)))))))") == "<Lambda>");
  REQUIRE(Eval(i, "(define zip (combine cons))") == "<Lambda>");
  REQUIRE(Eval(i, "(zip (list 1 2 3 4) (list 5 6 7 8))") == "((1 5) (2 6) (3 7) (4 8))");
  REQUIRE(Eval(i, "(define riff-shuffle (lambda (deck) (begin"
          "(define take (lambda (n seq) (if (<= n 0) (quote ()) (cons (car seq) (take (- n 1) (cdr seq))))))"
          "(define drop (lambda (n seq) (if (<= n 0) seq (drop (- n 1) (cdr seq)))))"
          "(define mid (lambda (seq) (/ (length seq) 2)))"
          "((combine append) (take (mid deck) deck) (drop (mid deck) deck)))))") == "<Lambda>");
  REQUIRE(Eval(i, "(riff-shuffle (list 1 2 3 4 5 6 7 8))") == "(1 5 2 6 3 7 4 8)");
  REQUIRE(Eval(i, "((repeat riff-shuffle) (list 1 2 3 4 5 6 7 8))") ==  "(1 3 5 7 2 4 6 8)");
  REQUIRE(Eval(i, "(riff-shuffle (riff-shuffle (riff-shuffle (list 1 2 3 4 5 6 7 8))))") == "(1 2 3 4 5 6 7 8)");
}

TEST_CASE("Simple addition", "[interpreter]")
{
  Interpreter i;
  REQUIRE(Eval(i, "(+ 1 1)") == "2");
}

TEST_CASE("Simple mul", "[interpreter]")
{
  Interpreter i;
  REQUIRE(Eval(i, "(* 1 2)") == "2");
}

TEST_CASE("Define and assign", "[interpreter]")
{
  Interpreter i;
  REQUIRE(Eval(i, "(define x 2)") == "2");
}

TEST_CASE("Define repeat and apply", "[interpreter]")
{
  Interpreter i;
  i.Eval("(define compose (lambda (f g) (lambda (x) (f (g x)))))");
  i.Eval("(define repeat (lambda (f) (compose f f)))");
  i.Eval("(define twice (lambda (x) (* 2 x)))");
  REQUIRE(Eval(i, "((repeat twice) 5)") == "20");
}

TEST_CASE("Parse some string", "[strings]")
{
  Interpreter i;
  REQUIRE(Eval(i, "(define myStr \"some string\")") == "some string");
}
