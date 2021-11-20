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

#include <Toten.h>
#include <Variable.h>


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
          error("требуется ')' ");
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
