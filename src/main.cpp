#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>

#include "catch.hpp"

// return given mumber as a string
std::string str(long n) { std::ostringstream os; os << n; return os.str(); }

// return true iff given character is '0'..'9'
bool isdig(char c) { return isdigit(static_cast<unsigned char>(c)) != 0; }


////////////////////// Cell

enum CellType { Symbol, Number, List, Proc, Lambda };

struct Env; // forward declaration; Cell and Env reference each other

// a variant that can hold any kind of lisp value
struct Cell {
    typedef Cell (*proc_type)(const std::vector<Cell> &);
    typedef std::vector<Cell>::const_iterator iter;
    typedef std::map<std::string, Cell> map;
    CellType type; std::string val; std::vector<Cell> list; proc_type proc; Env * env;
    Cell(CellType type = Symbol) : type(type), env(0) {}
    Cell(CellType type, const std::string & val) : type(type), val(val), env(0) {}
    Cell(proc_type proc) : type(Proc), proc(proc), env(0) {}
};

typedef std::vector<Cell> Cells;
typedef Cells::const_iterator Cellit;

const Cell false_sym(Symbol, "#f");
const Cell true_sym(Symbol, "#t"); // anything that isn't false_sym is true
const Cell nil(Symbol, "nil");


////////////////////// Env

// a dictionary that (a) associates symbols with Cells, and
// (b) can chain to an "outer" dictionary
struct Env {
    Env(Env * outer = 0) : outer_(outer) {}

    Env(const Cells & parms, const Cells & args, Env * outer)
    : outer_(outer)
    {
        Cellit a = args.begin();
        for (Cellit p = parms.begin(); p != parms.end(); ++p)
            env_[p->val] = *a++;
    }

    // map a variable name onto a Cell
    typedef std::map<std::string, Cell> map;

    // return a reference to the innermost Env where 'var' appears
    map & find(const std::string & var)
    {
        if (env_.find(var) != env_.end())
            return env_; // the symbol exists in this Env
        if (outer_)
            return outer_->find(var); // attempt to find the symbol in some "outer" env
        std::cout << "unbound symbol '" << var << "'\n";
        exit(1);
    }

    // return a reference to the Cell associated with the given symbol 'var'
    Cell & operator[] (const std::string & var)
    {
        return env_[var];
    }
    
private:
    map env_; // inner symbol->Cell mapping
    Env * outer_; // next adjacent outer env, or 0 if there are no further Envs
};


////////////////////// built-in primitive procedures

Cell proc_add(const Cells & c)
{
    long n(atol(c[0].val.c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i) n += atol(i->val.c_str());
    return Cell(Number, str(n));
}

Cell proc_sub(const Cells & c)
{
    long n(atol(c[0].val.c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i) n -= atol(i->val.c_str());
    return Cell(Number, str(n));
}

Cell proc_mul(const Cells & c)
{
    long n(1);
    for (Cellit i = c.begin(); i != c.end(); ++i) n *= atol(i->val.c_str());
    return Cell(Number, str(n));
}

Cell proc_div(const Cells & c)
{
    long n(atol(c[0].val.c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i) n /= atol(i->val.c_str());
    return Cell(Number, str(n));
}

Cell proc_greater(const Cells & c)
{
    long n(atol(c[0].val.c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i)
        if (n <= atol(i->val.c_str()))
            return false_sym;
    return true_sym;
}

Cell proc_less(const Cells & c)
{
    long n(atol(c[0].val.c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i)
        if (n >= atol(i->val.c_str()))
            return false_sym;
    return true_sym;
}

Cell proc_less_equal(const Cells & c)
{
    long n(atol(c[0].val.c_str()));
    for (Cellit i = c.begin()+1; i != c.end(); ++i)
        if (n > atol(i->val.c_str()))
            return false_sym;
    return true_sym;
}

Cell proc_length(const Cells & c) { return Cell(Number, str(c[0].list.size())); }
Cell proc_nullp(const Cells & c)  { return c[0].list.empty() ? true_sym : false_sym; }
Cell proc_car(const Cells & c)    { return c[0].list[0]; }

Cell proc_cdr(const Cells & c)
{
    if (c[0].list.size() < 2)
        return nil;
    Cell result(c[0]);
    result.list.erase(result.list.begin());
    return result;
}

Cell proc_append(const Cells & c)
{
    Cell result(List);
    result.list = c[0].list;
    for (Cellit i = c[1].list.begin(); i != c[1].list.end(); ++i) result.list.push_back(*i);
    return result;
}

Cell proc_cons(const Cells & c)
{
    Cell result(List);
    result.list.push_back(c[0]);
    for (Cellit i = c[1].list.begin(); i != c[1].list.end(); ++i) result.list.push_back(*i);
    return result;
}

Cell proc_list(const Cells & c)
{
    Cell result(List); result.list = c;
    return result;
}

// define the bare minimum set of primintives necessary to pass the unit tests
void add_globals(Env & env)
{
    env["nil"] = nil;   env["#f"] = false_sym;  env["#t"] = true_sym;
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
    if (x.type == Symbol)
        return env->find(x.val)[x.val];
    if (x.type == Number)
        return x;
    if (x.list.empty())
        return nil;
    if (x.list[0].type == Symbol) {
        if (x.list[0].val == "quote")       // (quote exp)
            return x.list[1];
        if (x.list[0].val == "if")          // (if test conseq [alt])
            return eval(eval(x.list[1], env).val == "#f" ? (x.list.size() < 4 ? nil : x.list[3]) : x.list[2], env);
        if (x.list[0].val == "set!")        // (set! var exp)
            return env->find(x.list[1].val)[x.list[1].val] = eval(x.list[2], env);
        if (x.list[0].val == "define")      // (define var exp)
            return (*env)[x.list[1].val] = eval(x.list[2], env);
        if (x.list[0].val == "lambda") {    // (lambda (var*) exp)
            x.type = Lambda;
            // keep a reference to the Env that exists now (when the
            // lambda is being defined) because that's the outer Env
            // we'll need to use when the lambda is executed
            x.env = env;
            return x;
        }
        if (x.list[0].val == "begin") {     // (begin exp*)
            for (size_t i = 1; i < x.list.size() - 1; ++i)
                eval(x.list[i], env);
            return eval(x.list[x.list.size() - 1], env);
        }
    }
                                            // (proc exp*)
    Cell proc(eval(x.list[0], env));
    Cells exps;
    for (Cell::iter exp = x.list.begin() + 1; exp != x.list.end(); ++exp)
        exps.push_back(eval(*exp, env));
    if (proc.type == Lambda) {
        // Create an Env for the execution of this lambda function
        // where the outer Env is the one that existed* at the time
        // the lambda was defined and the new inner associations are the
        // parameter names with the given arguments.
        // *Although the environmet existed at the time the lambda was defined
        // it wasn't necessarily complete - it may have subsequently had
        // more symbols defined in that Env.
        return eval(/*body*/proc.list[2], new Env(/*parms*/proc.list[1].list, /*args*/exps, proc.env));
    }
    else if (proc.type == Proc)
        return proc.proc(exps);

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
            c.list.push_back(read_from(tokens));
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
std::string to_string(const Cell & exp)
{
    if (exp.type == List) {
        std::string s("(");
        for (Cell::iter e = exp.list.begin(); e != exp.list.end(); ++e)
            s += to_string(*e) + ' ';
        if (s[s.size() - 1] == ' ')
            s.erase(s.size() - 1);
        return s + ')';
    }
    else if (exp.type == Lambda)
        return "<Lambda>";
    else if (exp.type == Proc)
        return "<Proc>";
    return exp.val;
}

// the default read-eval-print-loop
void repl(const std::string & prompt, Env * env)
{
    for (;;) {
        std::cout << prompt;
        std::string line; std::getline(std::cin, line);
        std::cout << to_string(eval(read(line), env)) << '\n';
    }
}

int main ()
{
    Env global_env; add_globals(global_env);
    repl("90> ", &global_env);
}

