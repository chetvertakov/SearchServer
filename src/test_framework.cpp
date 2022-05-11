#include "test_framework.h"

using namespace std;
// ----------- Реализация тестового фреймворка -----------------

// функция проверяющая условное выражение и выводящая ошибку если оно не истинно
void AssertImpl(bool value, const string &expr_str, const string &file,
                const string &func, unsigned line, const string &hint) {

    if (!value) {
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT("s << expr_str << ") failed."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}
