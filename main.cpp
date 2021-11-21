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

#include <Token.h>
#include <Variable.h>


// Следующие функции работают с переменными (глобальный объект класса Symbol_table) и с лексемами (нужен объект класса Token_stream)


// Объявление функции "Выражение"
double expression (Token_stream&);

// Определение функции "Первичное выражение"
double primary (Token_stream& ts) {
    Token t = ts.get();
    double left{}; // Нужна для степени
    switch (t.kind) {

    case '(':
    {
        double d = expression(ts);
        t = ts.get();
        if (t.kind != ')')
          error("требуется ')' ");
        left = d;
        break;
    }

    case '|':
    {
        double d = expression(ts);
        t = ts.get();
        if (t.kind != '|')
          error("требуется '|' ");
        left =  abs(d);
        break;
    }
    case '-': {
        left =  -primary(ts);
        break;
    }
    case '+': {
        left =  +primary(ts);
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
    if (t.kind == '^'){
        double pr = primary(ts);
        if (pr == 0 && left == 0)
            error("Ноль в степени ноль не определен!");
        return pow(left, pr);  // возведение в степень по правилам математики 3^2^4 = 3^16
    }
    ts.putback(t);
    return left;
}

// Определение функции "Терм"
double term (Token_stream& ts) {
    double left = primary(ts);

    while (true) {
        Token t = ts.get();

        switch (t.kind) {

        case '*':
        {
            left *= primary(ts);
            break;
        }
        case '/':
        {
            double d = primary(ts);
            if (d == 0) error(" / : деление на нуль ");
            left /= d;
            break;
        }
        case '%':
        {
            double d = primary(ts);
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
double expression (Token_stream& ts) {
    double left = term(ts);

    while (true) {
        Token t = ts.get();

        switch (t.kind) {

        case '+':
            left += term(ts);
            break;

        case '-':
            left -= term(ts);
            break;

        default:
            ts.putback(t);
            return left;
        }
    }
}

// Объявляем константу
double const_declaration (Token_stream& ts) {
    Token t = ts.get();
    if (t.kind != name)
        error("имя ожидается в объявлении");

    string var = t.name;

    if (sym_tab.is_declared(var))
        error(var + " уже объявлена!");

    t = ts.get();
    if (t.kind != '=')
        error("пропущен символ '=' в объявлении" + var);

    return sym_tab.define_name (var, expression(ts), true);
}

// Объявляем переменную
double declaration (Token_stream& ts) {
    Token t = ts.get();
    if (t.kind != name)
        error("имя ожидается в объявлении");

    string var = t.name;

    if (sym_tab.is_declared(var))
        error(var + " уже объявлена!");

    t = ts.get();
    if (t.kind != '=')
        error("пропущен символ '=' в объявлении" + var);

    return sym_tab.define_name (var, expression(ts), false);
}

// Функция, связанная с переменной (либо изменяет переменную, либо вызывает функцию expression)
double variable(Token_stream& ts) {
    char ch;
    cin >> ch;
    if (ch == '='){
        Token t = ts.get();
        double result = expression(ts);
        sym_tab.set (t.name, result);
        return result;
    }
    else {
        cin.putback(ch);
        return expression(ts);
    }
}

// "Инструкция" в грамматике
// вызывает функции для объявления или изменения переменной, объявления константы, вычисления выражения)
double statement (Token_stream& ts) {
    Token t = ts.get();
    switch (t.kind)
    {
    case let:
        return declaration(ts);
    case constant:
        return const_declaration(ts);
    case name:
        ts.putback(t);
        return variable(ts);
    default:
        ts.putback(t);
        return expression(ts);
    }
}

// "Вычисление" в грамматике (вызывает statement, вывод, выход, помощь)
void calculate (Token_stream& ts) {
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
        cout << result << statement(ts) << endl;
        }
    catch (runtime_error& e) {
        cerr << e.what() << endl;
        ts.ignore (print);
    }
}


int main ()
try {
    Token_stream ts;  // создаем локально объект класса Token_stream

    sym_tab.define_name ("pi", 3.141592653589793, true); // true = константа
    sym_tab.define_name ("e",  2.718281828459045, true);
    sym_tab.define_name ("k",  1000.0, true);

    cout << "команда help выдаст полную информацию по использованию калькулятора" << endl;
    calculate(ts);  // Вычисления
}
catch (exception& e) {
    cerr << "Исключение: " << e.what() << endl;
    return 1;
}
catch (...) {
    cerr << "Неясное исключение" << endl;
    return 2;
}
