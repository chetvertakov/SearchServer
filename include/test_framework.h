#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

// ----------- Реализация тестового фреймворка -----------------

// перегрузка оператора << для пары
template <typename T1, typename T2>
std::ostream& operator << (std::ostream &out, const std::pair<T1, T2> &pair ) {
    using namespace std;
    out << pair.first << ": "s << pair.second;
    return out;
}

// шаблонная функция Print для контейнеров vector, set, map
template <typename T>
std::ostream& Print(std::ostream &out, const T &container) {
    using namespace std;
    if (container.begin() == container.end()) {
        return out;
    }
    bool first_element = true;
    for (auto &element: container) {
        if (first_element) {
            out << element;
        } else {
            out << ", "s << element;
        }
        first_element = false;
    }
    return out;
}

// перегрузка оператора << для контейнера vector
template <typename T>
std::ostream& operator << (std::ostream &out, const std::vector<T> &container ) {
    using namespace std;
    out << "["s;
    Print(out, container) << "]"s;
    return out;
}

// перегрузка оператора << для контейнера set
template <typename T>
std::ostream& operator << (std::ostream &out, const std::set<T> &container ) {
    using namespace std;
    out << "{"s;
    Print(out, container) << "}"s;
    return out;
}

// перегрузка оператора << для контейнера map
template <typename T1, typename T2>
std::ostream& operator << (std::ostream &out, const std::map<T1, T2> &container ) {
    using namespace std;
    out << "{"s;
    Print(out, container) << "}"s;
    return out;
}

// шаблонная функция запуска теста и вывода результата в cerr
template <typename T>
void RunTestImpl(T func, const std::string &name) {
    using namespace std;
    func();
    cerr << name << " OK"s << endl;
}
#define RUN_TEST(func) RunTestImpl((func), #func)

// шаблонная функция, сравнивающая два значения и выводящая ошибку, если они не равны
template <typename T, typename U>
void AssertEqualImpl(const T &t, const U &u, const std::string &t_str,
                     const std::string &u_str, const std::string &file,
                     const std::string &func, unsigned line, const std::string &hint) {
    using namespace std;
    if (t != u) {
        cerr << boolalpha;
        cerr << file << "("s << line << "): "s << func << ": "s;
        cerr << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            cerr << " Hint: "s << hint;
        }
        cerr << endl;
        abort();
    }
}
#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

// функция проверяющая условное выражение и выводящая ошибку если оно не истинно
void AssertImpl(bool value, const std::string &expr_str, const std::string &file,
                const std::string &func, unsigned line, const std::string &hint);
#define ASSERT(expr) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)
#define ASSERT_HINT(expr, hint) AssertImpl((expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))
