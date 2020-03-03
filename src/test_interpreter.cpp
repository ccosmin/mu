#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "cell.hpp"
#include "interpreter.hpp"

using namespace mu;

TEST_CASE("Simple addition", "[interpreter]")
{
  Interpreter i;
  REQUIRE(i.Eval("(+ 1 1)").ToString() == "2");
}

TEST_CASE("Simple mul", "[interpreter]")
{
  Interpreter i;
  REQUIRE(i.Eval("(* 1 2)").ToString() == "2");
}
