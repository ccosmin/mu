#ifndef __MU_ENV_HPP__
#define __MU_ENV_HPP__

#include "cell.hpp"
#include <iostream>

namespace mu {

class Env 
{
public:
  Env(Env* outer = nullptr) 
  : m_outer(outer) 
  {
  }

  Env(const Cells& parms, const Cells& args, Env* outer)
    : m_outer(outer)
    {
        Cellit a = args.begin();
        for (Cellit p = parms.begin(); p != parms.end(); ++p)
            m_env[p->GetVal()] = *a++;
    }

    // map a variable name onto a Cell
    typedef std::map<std::string, Cell> map;

    // return a reference to the innermost Env where 'var' appears
    map & find(const std::string & var)
    {
        if (m_env.find(var) != m_env.end())
            return m_env; // the symbol exists in this Env
        if (m_outer)
            return m_outer->find(var); // attempt to find the symbol in some "outer" env
        std::cout << "unbound symbol '" << var << "'\n";
        exit(1);
    }

    // return a reference to the Cell associated with the given symbol 'var'
    Cell & operator[] (const std::string & var)
    {
        return m_env[var];
    }
    
private:
    map m_env; // inner symbol->Cell mapping
    Env* m_outer; // next adjacent outer env, or 0 if there are no further Envs
};

}

#endif
