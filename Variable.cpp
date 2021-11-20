#include <Variable.h>

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
