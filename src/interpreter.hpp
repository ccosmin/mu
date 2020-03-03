#ifndef __MU_INTERPRETER_HPP__
#define __MU_INTERPRETER_HPP__

#include "cell.hpp"
#include "env.hpp"

namespace mu {

class Interpreter
{
public:
  Interpreter();

  Cell Eval(const std::string& str);
  void Repl();

private:
  Env m_env;
};

}

#endif
