#include "log_duration.h"
#include "test_example_functions.h"

#include <vector>


using namespace std;

void AddDocument(SearchServer &search_server, int document_id, const std::string &document,
                 DocumentStatus status, const std::vector<int> &ratings) {
    search_server.AddDocument (document_id, document, status, ratings);
}

void FindTopDocuments(const SearchServer &search_server, const std::string &raw_query)
{
    {
        LOG_DURATION_STREAM("Operation time"s, cout);
        cout << "Результаты поиска по запросу: "s << raw_query << endl;
        auto result = search_server.FindTopDocuments(raw_query);
        for (auto &document : result) {
            PrintDocument(document);
        }
    }
    cout << endl;
}

void ProfileGetWordFrequencies() {

    auto test = [](int docs) {
        vector<string> temp_docs;
        temp_docs.reserve(static_cast<size_t>(docs));
        for (int i = 0; i < docs; ++i) {
            string doc = to_string(i) + "_word1 " +
                         to_string(i) + "_word2 " +
                         to_string(i) + "_word3 ";
            temp_docs.push_back(doc);
        }
        SearchServer server(""s);
        for (int i = 0; i < docs; ++i) {
            server.AddDocument(i, temp_docs[static_cast<size_t>(i)]);
        }
        LOG_DURATION_STREAM("GetWordFrequencies operation time for "s
                            + to_string(docs) + " documents"s, cout);
        // проверяем в цикле 10000 раз, иначе будет слишком быстро
        for (int i = 0; i < 1000; ++i) {
            server.GetWordFrequencies(50);
        }
    };
    // тестируем для 100 документов
    {
        test(100);
    }
    // тестируем для 1000 документов
    {
        test(1000);
    }
    // тестируем для 10000 документов
    {
       test(10000);
    }
    // тестируем для 100000 документов
    {
       test(100000);
    }
}

void ProfileRemoveDocument() {

    auto test = [] (int words, int docs) {
        vector<string> temp_docs;
        temp_docs.reserve(static_cast<size_t>(docs));
        for (int i = 0; i < docs; ++i) {
            string doc;
            doc.clear();
            for (int j = 1 ; j <= words; ++j) {
                doc += to_string(i) + "_word"s + to_string(j) + " "s;
            }
            temp_docs.push_back(doc);
        }
        SearchServer server(""s);
        for (int i = 0; i < docs; ++i) {
            server.AddDocument(i, temp_docs[static_cast<size_t>(i)]);
        }
        LOG_DURATION_STREAM("RemoveDocument operation time for "s + to_string(docs) +
                            " documents with "s + to_string(words) + " words"s, cout);
        for (int i = 0; i < 10; ++i) {
            server.RemoveDocument(i);
        }
    };

    //// тестируем для 100 документов в 100 слов
    {
        test(100, 100);
    }

    //// тестируем для 1000 документов в 100 слов
    {
        test(1000, 100);
    }
    //// тестируем для 100 документов в 500 слов
    {
        test(100, 500);
    }

    //// тестируем для 1000 документов в 500 слов
    {
        test(1000, 500);
    }

    //// тестируем для 100 документов в 1000 слов
    {
        test(100, 1000);
    }

    //// тестируем для 1000 документов в 1000 слов
    {
        test(1000, 1000);
    }
}



string GenerateWord(std::mt19937 &generator, int max_length) {
    const int length = std::uniform_int_distribution(1, max_length)(generator);
    std::string word;
    word.reserve(static_cast<size_t>(length));
    for (int i = 0; i < length; ++i) {
        word.push_back(std::uniform_int_distribution('a', 'z')(generator));
    }
    return word;
}

std::vector<string> GenerateDictionary(std::mt19937 &generator, int word_count, int max_length) {
    std::vector<std::string> words;
    words.reserve(static_cast<size_t>(word_count));
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    std::sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
            query.push_back('-');
        }
        query += dictionary[static_cast<size_t>(uniform_int_distribution<int>(0, static_cast<int>(dictionary.size()) - 1)(generator))];
    }
    return query;
}

std::vector<string> GenerateQueries(std::mt19937 &generator, const std::vector<std::string> &dictionary, int query_count, int max_word_count) {
    std::vector<std::string> queries;
    queries.reserve(static_cast<size_t>(query_count));
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}
