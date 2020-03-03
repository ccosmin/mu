#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>

#include "catch.hpp"
#include "cell.hpp"
#include "env.hpp"
#include "interpreter.hpp"

using namespace mu;

// return given mumber as a string
std::string str(long n) { std::ostringstream os; os << n; return os.str(); }

// return true iff given character is '0'..'9'
bool isdig(char c) { return isdigit(static_cast<unsigned char>(c)) != 0; }


Cell proc_add(const Cells & c)
{
    long n(atol(c[0].GetVal().c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i) n += atol(i->GetVal().c_str());
    return Cell(Number, str(n));
}

Cell proc_sub(const Cells & c)
{
    long n(atol(c[0].GetVal().c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i) n -= atol(i->GetVal().c_str());
    return Cell(Number, str(n));
}

Cell proc_mul(const Cells & c)
{
    long n(1);
    for (Cellit i = c.begin(); i != c.end(); ++i) n *= atol(i->GetVal().c_str());
    return Cell(Number, str(n));
}

Cell proc_div(const Cells & c)
{
    long n(atol(c[0].GetVal().c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i) n /= atol(i->GetVal().c_str());
    return Cell(Number, str(n));
}

Cell proc_greater(const Cells & c)
{
    long n(atol(c[0].GetVal().c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i)
        if (n <= atol(i->GetVal().c_str()))
            return FalseSym;
    return TrueSym;
}

Cell proc_less(const Cells & c)
{
    long n(atol(c[0].GetVal().c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i)
        if (n >= atol(i->GetVal().c_str()))
            return FalseSym;
    return TrueSym;
}

Cell proc_less_equal(const Cells & c)
{
    long n(atol(c[0].GetVal().c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i)
        if (n > atol(i->GetVal().c_str()))
            return FalseSym;
    return TrueSym;
}

Cell proc_length(const Cells & c) { return Cell(Number, str(c[0].GetList().size())); }
Cell proc_nullp(const Cells & c)  { return c[0].GetList().empty() ? TrueSym : FalseSym; }
Cell proc_car(const Cells & c)    { return c[0].GetList()[0]; }

Cell proc_cdr(const Cells & c)
{
    if (c[0].GetList().size() < 2)
        return Nil;
    Cell result(c[0]);
    result.GetList().erase(result.GetList().begin());
    return result;
}

Cell proc_append(const Cells & c)
{
    Cell result(List);
    result.GetList() = c[0].GetList();
    for (Cellit i = c[1].GetList().begin(); i != c[1].GetList().end(); ++i) result.GetList().push_back(*i);
    return result;
}

Cell proc_cons(const Cells & c)
{
    Cell result(List);
    result.GetList().push_back(c[0]);
    for (Cellit i = c[1].GetList().begin(); i != c[1].GetList().end(); ++i) result.GetList().push_back(*i);
    return result;
}

Cell proc_list(const Cells & c)
{
    Cell result(List); result.GetList() = c;
    return result;
}

// define the bare minimum set of primintives necessary to pass the unit tests
void add_globals(Env & env)
{
    env["Nil"] = Nil;   env["#f"] = FalseSym;  env["#t"] = TrueSym;
    env["append"] = Cell(&proc_append);   env["car"]  = Cell(&proc_car);
    env["cdr"]    = Cell(&proc_cdr);      env["cons"] = Cell(&proc_cons);
    env["length"] = Cell(&proc_length);   env["list"] = Cell(&proc_list);
    env["null?"]  = Cell(&proc_nullp);    env["+"]    = Cell(&proc_add);
    env["-"]      = Cell(&proc_sub);      env["*"]    = Cell(&proc_mul);
    env["/"]      = Cell(&proc_div);      env[">"]    = Cell(&proc_greater);
    env["<"]      = Cell(&proc_less);     env["<="]   = Cell(&proc_less_equal);
}


////////////////////// eval

Cell eval(Cell x, Env * env)
{
    if (x.GetType() == Symbol)
        return env->find(x.GetVal())[x.GetVal()];
    if (x.GetType() == Number)
        return x;
    if (x.GetList().empty())
        return Nil;
    if (x.GetList()[0].GetType() == Symbol) {
        if (x.GetList()[0].GetVal() == "quote")       // (quote exp)
            return x.GetList()[1];
        if (x.GetList()[0].GetVal() == "if")          // (if test conseq [alt])
            return eval(eval(x.GetList()[1], env).GetVal() == "#f" ? (x.GetList().size() < 4 ? Nil : x.GetList()[3]) : x.GetList()[2], env);
        if (x.GetList()[0].GetVal() == "set!")        // (set! var exp)
            return env->find(x.GetList()[1].GetVal())[x.GetList()[1].GetVal()] = eval(x.GetList()[2], env);
        if (x.GetList()[0].GetVal() == "define")      // (define var exp)
            return (*env)[x.GetList()[1].GetVal()] = eval(x.GetList()[2], env);
        if (x.GetList()[0].GetVal() == "lambda") {    // (lambda (var*) exp)
            x.SetType(Lambda);
            // keep a reference to the Env that exists now (when the
            // lambda is being defined) because that's the outer Env
            // we'll need to use when the lambda is executed
            x.SetEnv(env);
            return x;
        }
        if (x.GetList()[0].GetVal() == "begin") {     // (begin exp*)
            for (size_t i = 1; i < x.GetList().size() - 1; ++i)
                eval(x.GetList()[i], env);
            return eval(x.GetList()[x.GetList().size() - 1], env);
        }
    }
                                            // (proc exp*)
    Cell proc(eval(x.GetList()[0], env));
    Cells exps;
    for (Cell::iter exp = x.GetList().begin() + 1; exp != x.GetList().end(); ++exp)
        exps.push_back(eval(*exp, env));
    if (proc.GetType() == Lambda) {
        // Create an Env for the execution of this lambda function
        // where the outer Env is the one that existed* at the time
        // the lambda was defined and the new inner associations are the
        // parameter names with the given arguments.
        // *Although the environmet existed at the time the lambda was defined
        // it wasn't necessarily complete - it may have subsequently had
        // more symbols defined in that Env.
        return eval(/*body*/proc.GetList()[2], new Env(/*parms*/proc.GetList()[1].GetList(), /*args*/exps, proc.GetEnv()));
    }
    else if (proc.GetType() == Proc)
        return proc.GetProc()(exps);

    std::cout << "not a function\n";
    exit(1);
}


////////////////////// parse, read and user interaction

// convert given string to list of tokens
std::list<std::string> tokenize(const std::string & str)
{
    std::list<std::string> tokens;
    const char * s = str.c_str();
    while (*s) {
        while (*s == ' ')
            ++s;
        if (*s == '(' || *s == ')')
            tokens.push_back(*s++ == '(' ? "(" : ")");
        else {
            const char * t = s;
            while (*t && *t != ' ' && *t != '(' && *t != ')')
                ++t;
            tokens.push_back(std::string(s, t));
            s = t;
        }
    }
    return tokens;
}

// numbers become Numbers; every other token is a Symbol
Cell atom(const std::string & token)
{
    if (isdig(token[0]) || (token[0] == '-' && isdig(token[1])))
        return Cell(Number, token);
    return Cell(Symbol, token);
}

// return the Lisp expression in the given tokens
Cell read_from(std::list<std::string> & tokens)
{
    const std::string token(tokens.front());
    tokens.pop_front();
    if (token == "(") {
        Cell c(List);
        while (tokens.front() != ")")
            c.GetList().push_back(read_from(tokens));
        tokens.pop_front();
        return c;
    }
    else
        return atom(token);
}

// return the Lisp expression represented by the given string
Cell read(const std::string & s)
{
    std::list<std::string> tokens(tokenize(s));
    return read_from(tokens);
}

// convert given Cell to a Lisp-readable string

// the default read-eval-print-loop
void repl(const std::string & prompt, Env * env)
{
    for (;;) {
        std::cout << prompt;
        std::string line; std::getline(std::cin, line);
        std::cout << eval(read(line), env).ToString() << '\n';
    }
}

Interpreter::Interpreter()
{
  add_globals(m_env);
}

Cell Interpreter::Eval(const std::string& str)
{
  return eval(read(str), &m_env);
}

void Interpreter::Repl()
{
  repl(">>", &m_env);
}
