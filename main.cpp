/*
  calculator08buggy.cpp

  Helpful comments removed.

  We have inserted 3 bugs that the compiler will catch and 3 that it won't.
*/

#include <std_lib_facilities.h>


struct Token
{
  char kind;
  double value;
  string name;

  Token (char ch)
    : kind{ ch }, value{ 0 }
  { }

  Token (char ch, double val)
    : kind{ ch }, value{ val }
  { }
  Token ( char ch , string n) :
      kind { ch } , name { n }
  { }
};


class Token_stream
{
  bool full{ false };
  Token buffer{ '\0' };

public:
  Token_stream ()  { }

  Token get ();
  void putback (Token t);

  void ignore (char);
};


void Token_stream::putback (Token t)
{
  if (full)
    error("putback() into a full buffer");

  buffer = t;
  full = true;
}


constexpr char quit = 'q';
constexpr char print = ';';
constexpr char help = 'h';
constexpr char number = '8';
constexpr char name = 'a';
constexpr char let = 'L';
constexpr char constanta = 'c';

const string prompt = "> ";
const string result = "= ";
const string declkey = "let";
const string const_decley = "const";


Token Token_stream::get ()
{
  if (full)
  {
    full = false;
    return buffer;
  }

  char ch;
  while (cin.get(ch) && isspace(ch) && ch != '\n') {}


  switch (ch)
  {
  case print:
  case '(':
  case ')':
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case '=':
    return Token{ ch };

  case '\n':
    return Token{ print };

  case '.':
  case '0': case '1': case '2': case '3': case '4':
  case '5': case '6': case '7': case '8': case '9':
  {
    cin.putback(ch);
    double val;
    cin >> val;
    return Token{ number, val };
  }

  default:
    if (isalpha(ch))
    {
      string s;
      s += ch;
      while (cin.get(ch) &&
             (isalpha(ch) || isdigit(ch) || ch == '_'))
        s += ch;
      cin.putback(ch);

      if (s == declkey) return Token{ let };
      if (s == const_decley) return Token{ constanta };
      if (s == "help") return Token{ help };
      if (s == "quit") return Token{ quit };

      return Token{ name, s };
    }
    error("Bad token");
  }
}


void Token_stream::ignore (char c)
{
  if (full && c == buffer.kind)
  {
    full = false;
    return;
  }
  full = false;

  char ch;
  while (cin >> ch)
    if (ch == c) return;
}


struct Variable
{
  string name;
  double value;
  bool constanta; // это константа или нет

  Variable (string n, double v, bool c)
      : name{ n }, value{ v }, constanta{ c }
  { }
};

class Symbol_table {
    vector<Variable> var_table;

public:
    Symbol_table () {}

    double get (string s);
    void set (string s, double d);
    bool is_declared (string s);
    double define_name (string var, double val, bool c);
};


double Symbol_table:: get (string s)
{
  for (int i = 0; i < var_table.size(); ++i)
    if (var_table[i].name == s)
      return var_table[i].value;

  error("get: undefined name ", s);
}

void Symbol_table:: set (string s, double d)
{
  for (int i = 0; i <= var_table.size(); ++i)
  {
    if (var_table[i].name == s)
    {
      if (var_table[i].constanta) // не можем изменить константу
          error(s + " cannot be changed");
      var_table[i].value = d;
      return;
    }
  }

  error("set: undefined name ", s);
}

bool Symbol_table:: is_declared (string s)
{
  for (int i = 0; i < var_table.size(); ++i)
    if (var_table[i].name == s) return true;

  return false;
}

double Symbol_table:: define_name (string var, double val, bool c)
{
  if (is_declared(var))
    error(var, " declared twice");

  var_table.push_back (Variable{ var, val, c });

  return val;
}



double expression (Symbol_table& sym_tab, Token_stream& ts);

double primary (Symbol_table& sym_tab, Token_stream& ts)
{
  Token t = ts.get();
  switch (t.kind)
  {
  case '(':
  {
    double d = expression(sym_tab, ts);
    t = ts.get();
    if (t.kind != ')')
      error("'(' expected");
    return d;
  }

  case '-':
    return -primary(sym_tab, ts);
  case '+':
    return +primary(sym_tab, ts);

  case number:
    return t.value;

  case name:
    return sym_tab.get(t.name);

  default:
    error("primary expected");
  }
}


double term (Symbol_table& sym_tab, Token_stream& ts)
{
  double left = primary(sym_tab, ts);

  while (true)
  {
    Token t = ts.get();

    switch (t.kind)
    {
    case '*':
      left *= primary(sym_tab, ts);
      break;

    case '/':
    {
      double d = primary(sym_tab, ts);
      if (d == 0) error("divide by zero");
      left /= d;
      break;
    }

    default:
      ts.putback(t);
      return left;
    }
  }
}


double expression (Symbol_table& sym_tab, Token_stream& ts)
{
  double left = term(sym_tab, ts);

  while (true)
  {
    Token t = ts.get();

    switch (t.kind)
    {
    case '+':
      left += term(sym_tab, ts);
      break;

    case '-':
      left -= term(sym_tab, ts);
      break;

    default:
      ts.putback(t);
      return left;
    }
  }
}

double const_declaration (Symbol_table& sym_tab, Token_stream& ts) {
    Token t = ts.get();
    if (t.kind != name)
      error("name expected in declaration");

    string var = t.name;

    if (sym_tab.is_declared(var))
      error(var, " declared twice");

    t = ts.get();
    if (t.kind != '=')
      error("'=' missing in declaration of ", var);

    return sym_tab.define_name (var, expression(sym_tab, ts), true);
}


double declaration (Symbol_table& sym_tab, Token_stream& ts)
{
  Token t = ts.get();
  if (t.kind != name)
    error("name expected in declaration");

  string var = t.name;

  if (sym_tab.is_declared(var))
    error(var, " declared twice");

  t = ts.get();
  if (t.kind != '=')
    error("'=' missing in declaration of ", var);

  return sym_tab.define_name (var, expression(sym_tab, ts), false);
}

double variable(Symbol_table& sym_tab, Token_stream& ts) {  // при обнаружении переменной
    char ch;
    cin >> ch;
    if (ch == '='){
        Token t = ts.get();
        double result = expression(sym_tab, ts);
        sym_tab.set (t.name, result);
        return result;
    }
    else {
        cin.putback(ch);
        return expression(sym_tab, ts);
    }
}

double statement (Symbol_table& sym_tab, Token_stream& ts)
{
  Token t = ts.get();
  switch (t.kind)
  {
  case let:
    return declaration(sym_tab, ts);
  case constanta:
      return const_declaration(sym_tab, ts);
  case name:
      ts.putback(t);
      return variable(sym_tab, ts);
  default:
    ts.putback(t);
    return expression(sym_tab, ts);
  }
}


void clean_up_mess (Token_stream& ts)
{
  ts.ignore (print);
}


void calculate (Symbol_table& sym_tab, Token_stream& ts)
{
  while (true)
  try
  {
    cout << prompt;
    Token t = ts.get();
    while (t.kind == print)
      t = ts.get();
    if (t.kind == 'h'){
        cout << "это калькулятор. Примеры использования: 2+3*(3-1)-3;" << endl
               << "можно объявлять переменные: let x = 8; " << endl
               << "можно изменять переменные: x = 5; " << endl
               << "можно объявлять константы: const zero = 0; " << endl
               << "можно использовать их в выражениях: 6-x*(2-zero)"<< endl
               << "константы по умолчанию: pi, e"<< endl;
        continue;
    }
    if (t.kind == quit) return;

    ts.putback(t);
    cout << result << statement(sym_tab, ts) << endl;
  }
  catch (runtime_error& e)
  {
    cerr << e.what() << endl;
    clean_up_mess(ts);
  }
}


int main ()
try
{
  Token_stream ts;
  Symbol_table sym_tab;
  sym_tab.define_name ("pi", 3.141592653589793, true); // true = константа
  sym_tab.define_name ("e",  2.718281828459045, true);

  calculate(sym_tab, ts);
}
catch (exception& e)
{
  cerr << "exception: " << e.what() << endl;
  return 1;
}
catch (...)
{
  cerr << "Oops, unknown exception" << endl;
  return 2;
}
