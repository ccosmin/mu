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
  Lambda,
  String,
  Boolean
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

  Cell(bool boolVal)
  : m_type(Boolean), m_boolVal(boolVal)
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

  bool GetBoolVal() const
  {
    return m_boolVal;
  }

  void SetBoolVal(bool boolVal)
  {
    m_boolVal = boolVal;
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
  bool m_boolVal;
  std::vector<Cell> m_list;
  ProcType m_proc;
  Env* m_env;
};

typedef std::vector<Cell> Cells;
typedef Cells::const_iterator Cellit;

inline Cell MakeBool(bool val) 
{
  Cell c(Boolean);
  c.SetBoolVal(val);
  return c;
}

const Cell FalseBool = MakeBool(false);
const Cell TrueBool = MakeBool(true);
const Cell Nil(Symbol, "nil");

}

#endif
