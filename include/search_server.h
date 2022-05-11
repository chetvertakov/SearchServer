#pragma once

#include <algorithm>
#include <cmath>
#include <execution>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "concurrent_map.h"
#include "document.h"
#include "string_processing.h"

const int MAX_RESULT_DOCUMENT_COUNT = 5; // кол-во выводимых документов в запросе
const int MAX_COUNTS = 16; // максимальное кол-во потоков выполенения


class SearchServer {
private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    struct Query {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };

    std::set<std::string, std::less<>> stop_words_; // множество стоп слов
    std::map<std::string, std::map<int, double>, std::less<>> word_to_document_; // словарь слов : <слово, <document_id, term_freq>>
    std::map<int, std::map<std::string_view, double>> document_to_word_;
    std::map<int, DocumentData> documents_; // словарь документов <document_id, данные>
    std::set<int> document_ids_; // множество ids документов на сервере

    // разбивает строку на слова, разделенные пробелами за вычетом стоп-слов
    std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;
    // выбрасывает исключение если в слове есть сервисные символы
    static bool IsNotValidWord(std::string_view word);
    // считает средний рейтинг по вектору рейтингов
    static int ComputeAverageRating(const std::vector<int> &ratings);
    // разделяет строку запроса на плюс и минус слова, выбрасывет исключение если есть некорректные минус-слова
    Query SplitQueryWords(std::string_view raw_query) const;
    // находит и возвращает все документы по запросу, соответствующие предикату
    template <class KeyMapper, class ExecutionPolicy>
    std::vector<Document> FindAllDocuments(ExecutionPolicy&& policy,
                                           std::string_view raw_query,
                                           KeyMapper key_mapper) const;

public:
    // конструкторы класса SearchServer
    template <class Сontainer>
    explicit SearchServer(const Сontainer &stop_words);
    explicit SearchServer(const std::string &stop_words);
    explicit SearchServer(std::string_view stop_words);

    // добавляет документ на сервер
    void AddDocument (int document_id, std::string_view document,
                      DocumentStatus status = DocumentStatus::ACTUAL, const std::vector<int> &ratings = {});

    // возвращает слова документа и их term-frequency
    std::map<std::string_view, double> GetWordFrequencies(int document_id) const;

    // удаляет документ с заданным id
    template<class ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id);
    void RemoveDocument(int document_id);

    // возвращает кол-во документов на сервере
    int GetDocumentCount() const;

    // итераторы по id-s документов в сервере
    std::set<int>::iterator begin() const;
    std::set<int>::iterator end() const;

    // возвращает отсортированный вектор документов по запросу
    template <class KeyMapper, class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query,
                                           KeyMapper key_mapper) const;
    template <class KeyMapper>
    std::vector<Document> FindTopDocuments(std::string_view raw_query, KeyMapper key_mapper) const;

    // перегружает FindTopDocuments для поиска по статусу
    template <class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query,
                                            DocumentStatus document_status) const;
    std::vector<Document> FindTopDocuments(std::string_view raw_query,
                                            DocumentStatus document_status) const;

    // перегружает FindTopDocuments для поиска по статусу по умолчанию (ACTUAL)
    template <class ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query) const;
    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    // возвращает кортеж из общих слов и статуса документа по запросу
    template<class ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy,
                                                std::string_view raw_query, int document_id) const;
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query,
                                                                            int document_id) const;
};

void PrintMatchDocumentResult(int document_id, const std::vector<std::string> &words, DocumentStatus status);
void PrintDocument(const Document &document);

template <class KeyMapper, class ExecutionPolicy>
std::vector<Document> SearchServer::FindAllDocuments(ExecutionPolicy&& policy,
                                                     std::string_view raw_query,
                                                     KeyMapper key_mapper) const {
    using namespace std;
    ConcurrentMap<int, double> document_to_relevance_par(MAX_COUNTS);
    Query query = SplitQueryWords(raw_query);
    const double documents_count = GetDocumentCount();

    // собираем вектор плюс слов
    std::vector<std::string_view> plus_words(query.plus_words.size());
    std::transform(query.plus_words.begin(),query.plus_words.end(), plus_words.begin(), [](auto &word) {
        return word;
    });

    for_each(policy, plus_words.begin(), plus_words.end(),
             [this, &document_to_relevance_par, documents_count, key_mapper](auto &word_view){
        if (word_to_document_.count(word_view) != 0) { // проходим по всем документам содержащим плюс слова
            // считаем IDF для слова из запроса
            const auto &record = word_to_document_.find(word_view)->second;
            const double inverse_document_freq = log(documents_count /
                static_cast<double>(record.size()));
            for (const auto& [document_id, term_freq] : record) { // считаем IDF-TF для документа
                // добавляем только документы удовлетворяющие предикату
                const auto &document = documents_.at(document_id);
                if (key_mapper(document_id, document.status, document.rating)) {
                    document_to_relevance_par[document_id].ref_to_value += term_freq * inverse_document_freq;
                }
            }
        }
    });

    map<int, double> document_to_relevance = document_to_relevance_par.BuildOrdinaryMap();

    // здесь не параллелим, чтобы не было гонки
    for_each(query.minus_words.begin(), query.minus_words.end(),
             [this, &document_to_relevance](auto &word_view){
        if (word_to_document_.count(word_view) != 0) {
            // проходим по всем документам содержащим минус-слово
            for (const auto& [document_id, term_freq] : word_to_document_.find(word_view)->second) {
                if (document_to_relevance.count(document_id)) {
                    document_to_relevance.erase(document_id); // если докумет с таким id есть в выдаче - удаляем
                }
            }
        }
    });

    vector<Document> matched_documents;
    matched_documents.reserve(document_to_relevance.size());
    for (const auto& [document_id, relevance] : document_to_relevance) {
        // формируем вектор документов на выдачу
        matched_documents.push_back(Document{document_id, relevance, documents_.at(document_id).rating});
    }
    return matched_documents;
}

template <class Сontainer>
SearchServer::SearchServer(const Сontainer &stop_words) {
    using namespace std;
    for (string_view stop_word: stop_words) {
        if (IsNotValidWord(stop_word)) {
            throw invalid_argument("Document contain service symbols");
        }
        if (!stop_word.empty()) {
            stop_words_.insert({stop_word.data(), stop_word.size()});
        }
    }
}

template <class KeyMapper, class ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, std::string_view raw_query,
                                                     KeyMapper key_mapper) const {
    using namespace std;
    vector<Document> matched_documents = FindAllDocuments(policy, raw_query, key_mapper);

    sort(policy, matched_documents.begin(), matched_documents.end(),
         [](const Document &lhs, const Document &rhs) {
        return rhs < lhs;});
    //оставляем только MAX_RESULT_DOCUMENT_COUNT первых результатов
    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }
    return matched_documents;
}

template <class KeyMapper>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
                                                     KeyMapper key_mapper) const {
    return FindTopDocuments(std::execution::seq, raw_query, key_mapper);
}

template<class ExecutionPolicy>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy,
                                                        std::string_view raw_query, int document_id) const {
    using namespace std;
    if (documents_.count(document_id) == 0) {
        throw out_of_range("No document with id "s + to_string(document_id));
    }

    Query query = SplitQueryWords(raw_query);

    tuple<vector<string_view>, DocumentStatus> result;
    get<1>(result) = documents_.at(document_id).status;

    // если в документе есть минус слово возвращаем пустой список
    if (any_of(policy, query.minus_words.begin(), query.minus_words.end(), [this, document_id] (auto &word){
        return document_to_word_.at(document_id).count(word) != 0;})) {

        return result;
    }

    // если в документе есть плюс-слово добавляем его (слово) в выдачу
    // еще раз проверил, распараллеливание с локом не даёт выигрыша в скорости,
    // поэтому оставил последовательную версию
    get<0>(result).reserve(query.plus_words.size());
    for_each(query.plus_words.begin(), query.plus_words.end(),
    [this, document_id, &result](auto &word){
        if (document_to_word_.at(document_id).count(word) != 0) {
            get<0>(result).push_back(word_to_document_.find(word)->first);
        }
    });
    return result;
}

template<class ExecutionPolicy>
void SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id) {
    if (documents_.count(document_id) != 0) {

        // собираем вектор слов документа
        std::vector<std::string_view> words(document_to_word_.at(document_id).size());
        std::transform(document_to_word_.at(document_id).begin(),
                       document_to_word_.at(document_id).end(), words.begin(), [](auto &word) {
            return word.first;
        });

        // удаляем слова (можно распараллелить потому что из каждого словаря удалится максимум одна запись)
        std::for_each(policy, words.begin(), words.end(), [this, document_id](auto &word_view) {
            word_to_document_.find(word_view)->second.erase(document_id);});

        // подчищаем в словаре слова, которые остались без документов
        // (нельзя распараллелить, потому что удаляются записи в одном словаре)
        std::for_each(words.begin(), words.end(), [this, document_id](auto &word_view) {
            auto it = word_to_document_.find(word_view);
            if (it->second.empty()) {
                word_to_document_.erase(it);
            }
        });

        documents_.erase(document_id);
        document_ids_.erase(document_id);
        document_to_word_.erase(document_id);
    }
}

template<class ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, std::string_view query,
                                                     DocumentStatus document_status) const {
    return FindTopDocuments(policy, query, [document_status](int, DocumentStatus status, int) {
        return status == document_status;
    });
}

template<class ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, std::string_view query) const {
    return FindTopDocuments(policy, query, DocumentStatus::ACTUAL);
}
