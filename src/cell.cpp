#include "cell.hpp"

using namespace mu;

std::string Cell::ToString() const
{
    if (GetType() == List) {
        std::string s("(");
        for (Cell::iter e = GetList().begin(); e != GetList().end(); ++e)
            s += e->ToString() + ' ';
        if (s[s.size() - 1] == ' ')
            s.erase(s.size() - 1);
        return s + ')';
    }
    else if (GetType() == Lambda)
        return "<Lambda>";
    else if (GetType() == Proc)
        return "<Proc>";
    return GetVal();
}

