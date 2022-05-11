#include "log_duration.h"
#include "process_queries.h"
#include "search_server.h"
#include "search_server_tests.h"
#include "test_example_functions.h"

#include <execution>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;


template <typename ExecutionPolicy>
void TestMatch(string_view mark, SearchServer search_server, const string& query, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    int word_count = 0;
    for (int id = 0; id < document_count; ++id) {
        const auto [words, status] = search_server.MatchDocument(policy, query, id);
        word_count += static_cast<int>(words.size());
    }
    cout << word_count << endl;
}
#define TEST_MATCH(policy) TestMatch(#policy, search_server, query, execution::policy)

template <typename ExecutionPolicy>
void TestRemove(string_view mark, SearchServer search_server, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    const int document_count = search_server.GetDocumentCount();
    for (int id = 0; id < document_count; ++id) {
        search_server.RemoveDocument(policy, id);
    }
    cout << search_server.GetDocumentCount() << endl;
}
#define TEST_REMOVE(policy) TestRemove(#policy, search_server, execution::policy)

template <typename QueriesProcessor>
void TestProcessQueries(string_view mark, QueriesProcessor processor, const SearchServer& search_server, const vector<string>& queries) {
    LOG_DURATION(mark);
    const auto documents = processor(search_server, queries);
    cout << documents.size() << endl;
}
#define TEST_PQ(processor) TestProcessQueries(#processor, processor, search_server, queries)

template <typename ExecutionPolicy>
void TestFindTopDocuments(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy) {
    LOG_DURATION(mark);
    double total_relevance = 0;
        for (const string_view query : queries) {
            for (const auto& document : search_server.FindTopDocuments(policy, query)) {
                total_relevance += document.relevance;
            }
        }
        cout << total_relevance << endl;
}
#define TEST_FTD(policy) TestFindTopDocuments(#policy, search_server, queries, execution::policy)

int main() {
    TestSearchServer ();
    cout << endl;

    mt19937 generator;
    // TestProcessQueries
    {
        const auto dictionary = GenerateDictionary(generator, 10000, 25);
        const auto documents = GenerateQueries(generator, dictionary, 100'000, 10);
        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(static_cast<int>(i), documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
        }
        const auto queries = GenerateQueries(generator, dictionary, 10'000, 7);
        cout << "Testing ProcessQueries speed: "s << endl;
        TEST_PQ(ProcessQueries);
        TEST_PQ(ProcessQueriesJoined);
    }
    cout << endl;
    // TestRemove
    {
        const auto dictionary = GenerateDictionary(generator, 10000, 25);
        const auto documents = GenerateQueries(generator, dictionary, 10'000, 100);
        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(static_cast<int>(i), documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
        }
        cout << "Testing Remove speed: "s << endl;
        TEST_REMOVE(seq);
        TEST_REMOVE(par);
    }
    cout << endl;
    // Test Match
    {
        const auto dictionary = GenerateDictionary(generator, 1000, 10);
        const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);
        const string query = GenerateQuery(generator, dictionary, 500, 0.1);
        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(static_cast<int>(i), documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
        }
        cout << "Testing Match speed: "s << endl;
        TEST_MATCH(seq);
        TEST_MATCH(par);
    }

    cout << endl;
    // Test Find Top
    {
        const auto dictionary = GenerateDictionary(generator, 1000, 10);
            const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);
            SearchServer search_server(dictionary[0]);
            for (size_t i = 0; i < documents.size(); ++i) {
                search_server.AddDocument(static_cast<int>(i), documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
            }
            const auto queries = GenerateQueries(generator, dictionary, 100, 70);
            cout << "Testing FindTopDocuments speed: "s << endl;
            TEST_FTD(seq);
            TEST_FTD(par);
    }

    return 0;
}
