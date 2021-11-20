#include <Toten.h>

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
