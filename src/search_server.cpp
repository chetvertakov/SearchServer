#include "search_server.h"
#include "string_processing.h"

#include <stdexcept>
#include <numeric>

using namespace std;

SearchServer::SearchServer(const std::string &stop_words) : SearchServer(SplitIntoWordsView(stop_words)) {}
SearchServer::SearchServer(std::string_view stop_words) : SearchServer(SplitIntoWordsView(stop_words)) {}

void SearchServer::AddDocument(int document_id, string_view document,
                               DocumentStatus status, const vector<int> &ratings) {

    // если документ пустой или документ с таким id уже есть на сервере или id отрицательный - ничего не добавляем
    if (document.empty()) {
        throw invalid_argument("Can't add empty document"s);
    }
    if (documents_.count(document_id) != 0) {
        throw invalid_argument("Document with this id already exist"s);
    }
    if (document_id < 0) {
        throw invalid_argument("Can't add document with negative id"s);
    }

    vector<string_view> words = SplitIntoWordsNoStop(document);
    const double document_size = static_cast<double>(words.size());

    // считаем term_freq для каждого слова в документе и записываем их вместе со словом
    for (string_view &word_view : words) {
        string word{word_view.data(), word_view.size()};
        word_to_document_[word][document_id] += 1.0/document_size;
        document_to_word_[document_id][word_to_document_.find(word)->first] += 1.0/document_size;
    }

    documents_[document_id] = DocumentData{ComputeAverageRating(ratings), status};
    document_ids_.insert(document_id);
}

map<string_view, double> SearchServer::GetWordFrequencies(int document_id) const {
    map<string_view, double> map = {};
    if (document_ids_.count(document_id) == 0) {
        return map;
    }
    return document_to_word_.at(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
    RemoveDocument(std::execution::seq, document_id);
}

int SearchServer::GetDocumentCount() const {
    return static_cast<int>(document_ids_.size());
}

set<int>::iterator SearchServer::begin() const {
    return document_ids_.begin(); // сложность О(1)
}

set<int>::iterator SearchServer::end() const {
    return document_ids_.end(); // сложность О(1)
}

int SearchServer::ComputeAverageRating(const vector<int> &ratings) {
    if (ratings.empty()) {
        return 0;
    }
    return accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(string_view text) const{
    using namespace std;
    vector<string_view> words = SplitIntoWordsView(text);
    vector<string_view> words_no_stop;
    words_no_stop.reserve(words.size());

    // здесь распараллеливание с локом words_no_stop преимущества в скорости не даёт
    for_each(words.begin(), words.end(),
    [this, &words_no_stop](string_view &word) {
        if (stop_words_.count(word) == 0) {
            if (IsNotValidWord(word)) {
                throw invalid_argument("Document contain service symbols"s);
            }
            words_no_stop.push_back(word);
        }
    });
    return words_no_stop;
}

bool SearchServer::IsNotValidWord(string_view word) {
    // A valid word must not contain special characters
    // здесь для не очень длинных слов распараллеливать смысла нет
    for (const char c : word) {
        if (c >= '\0' && c < ' ') {
            return true;
        }
    }
    return false;
}

SearchServer::Query SearchServer::SplitQueryWords(string_view raw_query) const {
    using namespace std;
    Query query;
    vector<string_view> words = SplitIntoWordsNoStop(raw_query);

    // здесь не параллелится (можно обрабатывать в 2 потока плюс и минус слова, или лочить контейнеры на запись,
    // но выигрыша по скорости не будет, я проверил)
    for_each(words.begin(), words.end (), [this, &query](auto &word) {
        if (word.at(0) == '-') {
            string_view result_word = word.substr(1);
            // не обрабатываем запросы со словами состоящими только из минуса или с двумя минусами вначале подряд
            if (result_word.empty() || result_word[0] == '-') {
                throw invalid_argument("Document contain invalid minus words"s);
            }
            if (stop_words_.count(result_word) == 0) {
                query.minus_words.insert(result_word);
            }
        } else {
            if (stop_words_.count(word) == 0) {
                //если слова нет в стоп-словах добавляем его в plus_words
                query.plus_words.insert(word);
            }
        }
    });
    return query;
}

vector<Document> SearchServer::FindTopDocuments(string_view query, DocumentStatus document_status) const {
    return FindTopDocuments(query, [document_status](int, DocumentStatus status, int) {
        return status == document_status;
    });
}

vector<Document> SearchServer::FindTopDocuments(string_view query) const {
    return FindTopDocuments(query, DocumentStatus::ACTUAL);
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(string_view raw_query,
                                                                            int document_id) const {
    return MatchDocument(std::execution::seq, raw_query, document_id);
}

void PrintMatchDocumentResult(int document_id, vector<string_view> words, DocumentStatus status) {
    cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (string_view word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}

void PrintDocument(const Document &document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
}


