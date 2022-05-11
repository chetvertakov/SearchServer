#include "document.h"

using namespace std;

// перегрузка оператора == для типа Document
bool operator == (const Document &lhs, const Document &rhs) {
    return (lhs.id == rhs.id &&
            lhs.rating == rhs.rating &&
            abs(lhs.relevance - rhs.relevance) < 1e-6);
}
bool operator != (const Document &lhs, const Document &rhs) {
    return !(lhs==rhs);
}
// перегрузка оператора < для типа Document
bool operator < (const Document &lhs, const Document &rhs) {
    //сравниваем документы по убыванию релевантности и рейтинга
    if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
        return lhs.rating < rhs.rating;
    }
    return lhs.relevance < rhs.relevance;
}
// перегрузка оператора << для типа Document
ostream& operator << (ostream &os, const Document &rhs) {
    os << "{ "s
         << "document_id = "s << rhs.id << ", "s
         << "relevance = "s << rhs.relevance << ", "s
         << "rating = "s << rhs.rating
         << " }"s;
    return os;
}
