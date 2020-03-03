#ifndef __MU_CELL_HPP__
#define __MU_CELL_HPP__

#include <vector>
#include <string>
#include <map>

namespace mu {

enum CellType 
{ 
  Symbol, 
  Number, 
  List, 
  Proc, 
  Lambda 
};

struct Env; // forward declaration; Cell and Env reference each other

// a variant that can hold any kind of lisp value
class Cell 
{
public:
  typedef Cell (*ProcType)(const std::vector<Cell> &);
  typedef std::vector<Cell>::const_iterator iter;
  typedef std::map<std::string, Cell> map;

  Cell(CellType type = Symbol) 
  : m_type(type), m_env(nullptr) 
  {
  }

  Cell(CellType type, const std::string& val) 
  : m_type(type), m_val(val), m_env(nullptr) 
  {
  }

  Cell(ProcType proc) 
  : m_type(Proc), m_proc(proc), m_env(nullptr) 
  {
  }

  std::string GetVal() const
  {
    return m_val;
  }

  CellType GetType() const
  {
    return m_type;
  }

  void SetType(CellType type)
  {
    m_type = type;
  }

  const std::vector<Cell>& GetList() const
  {
    return m_list;
  }

  std::vector<Cell>& GetList()
  {
    return m_list;
  }

  ProcType GetProc() const
  {
    return m_proc;
  }

  Env* GetEnv() const
  {
    return m_env;
  }

  void SetEnv(Env* env)
  {
    m_env = env;
  }

  std::string ToString() const;
  
private:
  CellType m_type;
  std::string m_val;
  std::vector<Cell> m_list;
  ProcType m_proc;
  Env* m_env;
};

typedef std::vector<Cell> Cells;
typedef Cells::const_iterator Cellit;

const Cell FalseSym(Symbol, "#f");
const Cell TrueSym(Symbol, "#t"); // anything that isn't false_sym is true
const Cell Nil(Symbol, "nil");

}

#endif
