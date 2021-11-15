/*
Ввод осуществляется из потока cin ; вывод - в поток cout.

Грамматика для ввода:

Вычисление:
    Инструкция
    Помощь
    Вывод
    Выход

Инструкция:
    Объявление
    Константа
    Переменная
    Выражение


Объявление:
    "let" Имя "=" Выражение

Константа:
    "const" Имя "=" Выражение


Переменная:
    Изменение переменной
    Выражение


Изменения переменной:
    Имя "="  Выражение


Помощь:
    help
Вывод:
    ;
Выход:
    quit


Выражение:
    Терм
    Выражение "+" Терм
    Выражение "-" Терм

Терм:
    Первичное_выражение
    Терм "*" Первичное_выражение
    Терм "/" Первичное_выражение
    Терм "%" Первичное_выражение

Первичное_выражение:
    Число
    ( Выражение )
    | Выражение |            // Модуль
    - Первичное_выражение
    + Первичное_выражение
    Первичное_выражение "^" Первичное_выражение     // Степень

Число:
    Литерал с плавающей точкой

Ввод из потока cin через Token stream с именем ts.

*/

#include <std_lib_facilities.h>


// Константы для программы
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


// Структура лексемы: её вид, значение у чисел, имя у переменных
struct Token {
    char kind;
    double value;
    string name;

    Token (char ch):           // Конструктор для символа
        kind{ ch }, value{ 0 } { }

    Token (char ch, double val):      // Конструктор для числа
        kind{ ch }, value{ val } { }

    Token ( char ch , string n):      // Конструктор для символа
        kind { ch } , name { n } { }
};

// Класс потока лексем: считывает лексемы; сохраняет в буфер и достает из него; игнорирует лексемы
class Token_stream {
    bool full{false};
    Token buffer{'\0'};

public:
    Token_stream ()  { } // Пустой конструктор

    Token get();

    void putback(Token t);

    void ignore(char);
};

// Сохраняет лексему
void Token_stream::putback (Token t)
{
    if (full)
        error("буфер уже заполнен!");

    buffer = t;
    full = true;
}

// Получает лексему и возращает её (работает с потоком cin и с буфером buffer)
Token Token_stream::get () {
    if (full) {
    full = false;
    return buffer;
    }

    char ch;
    while (cin.get(ch) && isspace(ch) && ch != '\n') {}  // игнорируем все пробельные символы, кроме "\n"

    switch (ch) {
    case '(':
    case ')':
    case '|':
    case '^':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
    case '=':
        return Token{ ch };  // Возвращаем лексему - симбол

    case print:
    case '\n':
        return Token{ print }; // Возвращаем лексему - " ; "

    case '.':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': {
        cin.putback(ch);
        double val;
        cin >> val;
        return Token{ number, val }; // Возвращаем лексему - число
    }

    default:
        if (isalpha(ch)) {
          string s;
          s += ch;
          while (cin.get(ch) &&
                 (isalpha(ch) || isdigit(ch) || ch == '_')) // Считываем слово
            s += ch;
          cin.putback(ch);

          if (s == declkey) return Token{ let }; // Если слово " let "
          if (s == const_decley) return Token{ constanta };  // Если слово " const "
          if (s == "help") return Token{ help };  // Если слово " help "
          if (s == "quit") return Token{ quit };  // Если слово " quit "

          return Token{ name, s };  // Возвращаем лексему - переменную
        }
        error("Неверная лексема!");
    }
}

// функция игнорирования лексем (нужна после обнаружения ошибки)
void Token_stream::ignore (char c) {
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;

    char ch;
    while (cin >> ch)
        if (ch == c) return;
}

// Структура переменной: хранит имя, значение и константа ли она или нет
struct Variable {
    string name;
    double value;
    bool constanta; // переменная константа или нет

    Variable (string n, double v, bool c):
        name{ n }, value{ v }, constanta{ c } { }
};

// Класс для сохранения переменных, нахождения перерменных и их значений, для изменения переменных
class Symbol_table {
    vector<Variable> var_table; // вектор с переменными (изначально пуст)

public:
    Symbol_table () {}  // Пустой конструктор

    double get (string s);
    void set (string s, double d);
    bool is_declared (string s);
    double define_name (string var, double val, bool c);
};

// Для нахождения значения переменной
double Symbol_table:: get (string s) {
    for (int i = 0; i < var_table.size(); ++i)
        if (var_table[i].name == s)
            return var_table[i].value;

    error("не найдена переменная " + s);
}

// Для изменения значения переменной
void Symbol_table:: set (string s, double d) {
    for (int i = 0; i <= var_table.size(); ++i) {
        if (var_table[i].name == s) {
            if (var_table[i].constanta)  // не можем изменить константу
                error(s + " не может быть изменена!");
            var_table[i].value = d;
            return;
        }
    }
    error("не найдена переменная с именем: " + s);
}

// Существует ли такая переменная
bool Symbol_table:: is_declared (string s) {
    for (int i = 0; i < var_table.size(); ++i)
        if (var_table[i].name == s)
            return true;

    return false;
}

// Объявляет переменную
double Symbol_table:: define_name (string var, double val, bool c) {
    if (is_declared(var))
        error(var + " уже объявлена!");

    var_table.push_back (Variable{ var, val, c });

    return val;
}


// Следующие функции работают с переменными (нужен объект класса Symbol_table) и с лексемами (нужен объект класса Token_stream)


// Объявление функции "Выражение"
double expression (Symbol_table& sym_tab, Token_stream& ts);

// Определение функции "Первичное выражение"
double primary (Symbol_table& sym_tab, Token_stream& ts) {
    Token t = ts.get();
    double left{}; // Нужна для степени
    switch (t.kind) {

    case '(':
    {
        double d = expression(sym_tab, ts);
        t = ts.get();
        if (t.kind != ')')
          error("требуется '(' ");
        left = d;
        break;
    }

    case '|':
    {
        double d = expression(sym_tab, ts);
        t = ts.get();
        if (t.kind != '|')
          error("требуется '|' ");
        left =  abs(d);
        break;
    }
    case '-': {
        left =  -primary(sym_tab, ts);
        break;
    }
    case '+': {
        left =  +primary(sym_tab, ts);
    break;
    }

    case number:
    {
        left =  t.value;
        break;
    }

    case name:
    {
        left =  sym_tab.get(t.name);
        break;
    }

    default:
        error("требуется первичное выражение!");
    }
    t = ts.get();
    if (t.kind == '^')
        return pow(left, primary(sym_tab, ts));  // возведение в степень по правилам математики 3^2^4 = 3^16
    ts.putback(t);
    return left;
}

// Определение функции "Терм"
double term (Symbol_table& sym_tab, Token_stream& ts) {
    double left = primary(sym_tab, ts);

    while (true) {
        Token t = ts.get();

        switch (t.kind) {

        case '*':
        {
            left *= primary(sym_tab, ts);
            break;
        }
        case '/':
        {
            double d = primary(sym_tab, ts);
            if (d == 0) error(" / : деление на нуль ");
            left /= d;
            break;
        }
        case '%':
        {
            double d = primary(sym_tab, ts);
            if (d == 0) error(" % : деление на нуль ");
            left = fmod(left, d);
            break;
        }
        default:
            ts.putback(t);
            return left;
        }
    }
}

// Определение функции "Выражение"
double expression (Symbol_table& sym_tab, Token_stream& ts) {
    double left = term(sym_tab, ts);

    while (true) {
        Token t = ts.get();

        switch (t.kind) {

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

// Объявляем константу
double const_declaration (Symbol_table& sym_tab, Token_stream& ts) {
    Token t = ts.get();
    if (t.kind != name)
        error("имя ожидается в объявлении");

    string var = t.name;

    if (sym_tab.is_declared(var))
        error(var + " уже объявлена!");

    t = ts.get();
    if (t.kind != '=')
        error("пропущен символ '=' в объявлении" + var);

    return sym_tab.define_name (var, expression(sym_tab, ts), true);
}

// Объявляем переменную
double declaration (Symbol_table& sym_tab, Token_stream& ts) {
    Token t = ts.get();
    if (t.kind != name)
        error("имя ожидается в объявлении");

    string var = t.name;

    if (sym_tab.is_declared(var))
        error(var + " уже объявлена!");

    t = ts.get();
    if (t.kind != '=')
        error("пропущен символ '=' в объявлении" + var);

    return sym_tab.define_name (var, expression(sym_tab, ts), false);
}

// Функция, связанная с переменной (либо изменяет переменную, либо вызывает функцию expression)
double variable(Symbol_table& sym_tab, Token_stream& ts) {
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

// "Инструкция" в грамматике
// вызывает функции для объявления или изменения переменной, объявления константы, вычисления выражения)
double statement (Symbol_table& sym_tab, Token_stream& ts) {
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

// "Вычисление" в грамматике (вызывает statement, вывод, выход, помощь)
void calculate (Symbol_table& sym_tab, Token_stream& ts) {
    while (true)
        try {
        cout << prompt;
        Token t = ts.get();
        while (t.kind == print)
          t = ts.get();
        if (t.kind == 'h'){
            cout << "это калькулятор. Примеры использования: 2+4%|3*(3-1)-3|^(3*8-2*11)/5;" << endl
                   << "можно объявлять переменные: let x = 8; " << endl
                   << "можно изменять переменные: x = 5*(2+x); " << endl
                   << "можно объявлять константы: const zero = 0; " << endl
                   << "можно использовать их в выражениях: 6-x*(2-zero)"<< endl
                   << "константы по умолчанию: pi, e, k (k = 1000)"<< endl
                   << "для выхода из калькулятора напишите quit"<< endl;
            continue;
        }
        if (t.kind == quit) return;

        ts.putback(t);
        cout << result << statement(sym_tab, ts) << endl;
        }
    catch (runtime_error& e) {
        cerr << e.what() << endl;
        ts.ignore (print);
    }
}


int main ()
try {
    Token_stream ts;  // создаем локально объект класса Token_stream
    Symbol_table sym_tab;  // создаем локально объект класса Symbol_table

    sym_tab.define_name ("pi", 3.141592653589793, true); // true = константа
    sym_tab.define_name ("e",  2.718281828459045, true);
    sym_tab.define_name ("k",  1000.0, true);

    cout << "команда help выдаст полную информацию по использованию калькулятора" << endl;
    calculate(sym_tab, ts);  // Вычисления
}
catch (exception& e) {
    cerr << "Исключение: " << e.what() << endl;
    return 1;
}
catch (...) {
    cerr << "Неясное исключение" << endl;
    return 2;
}
