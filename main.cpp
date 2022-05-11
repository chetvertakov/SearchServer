#include "paginator.h"
#include "process_queries.h"
#include "search_server.h"
#include "remove_duplicates.h"
#include "request_queue.h"

#include <string>
#include <vector>

using namespace std;

int main() {
    // основные методы поискового сервера
    {
        vector<string> stop_words{"и"s, "но"s, "или"s};
        // создаём экземпляр поискового сервера со списком стоп слов
        SearchServer server(stop_words);
        // добавляем документы в сервер
        server.AddDocument(0, "белый кот и пушистый хвост"sv);
        server.AddDocument(1, "черный пёс но желтый хвост"sv);
        server.AddDocument(3, "черный жираф или белый дракон"sv);
        cout << "Кол-во документов : "sv << server.GetDocumentCount() << endl;
        // ищем документы с ключевыми словами
        auto result = server.FindTopDocuments("черный дракон"sv);
        cout << "Документы с ключевыми словами \"черный дракон\" : "sv << endl;
        for (auto &doc : result) {
            cout << doc << endl;
        }

        // добавляем копию другого документа
        server.AddDocument(4, "черный жираф или белый дракон"sv);
        result = server.FindTopDocuments("черный дракон"sv);
        cout << "Документы с ключевыми словами \"черный дракон\" : "sv << endl;
        for (auto &doc : result) {
            cout << doc << endl;
        }
        // проверяем сервер на копии и удаляем их
        RemoveDuplicates(server);
        result = server.FindTopDocuments("черный дракон"sv);
        cout << "Документы с ключевыми словами \"черный дракон\" после удаления копий : "sv << endl;
        for (auto &doc : result) {
            cout << doc << endl;
        }
    }
    cout << endl << endl;

    // постраничный вывод
    {
        vector<string> stop_words{"и"s, "но"s, "или"s};
        // создаём экземпляр поискового сервера со списком стоп слов
        SearchServer server(stop_words);
        // добавляем документы в сервер
        server.AddDocument(0, "первый документ"sv);
        server.AddDocument(1, "второй документ"sv);
        server.AddDocument(2, "третий документ"sv);
        server.AddDocument(3, "четвертый документ"sv);
        server.AddDocument(4, "пятый документ"sv);

        const auto search_results = server.FindTopDocuments("документ");

        // разбиваем результат поиска на страницы
        size_t page_size = 2;
        const auto pages = Paginate(search_results, page_size);
        for (auto page : pages) {
                cout << page << endl;
                cout << "Page break"s << endl;
        }

    }
    cout << endl << endl;

    // очередь запросов
    {
        SearchServer search_server("and in at"s);
        RequestQueue request_queue(search_server);

        search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, {7, 2, 7});
        search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, {1, 2, 3});
        search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, {1, 2, 8});
        search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, {1, 3, 2});
        search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, {1, 1, 1});

        // 1439 запросов с нулевым результатом
        for (int i = 0; i < 1439; ++i) {
            request_queue.AddFindRequest("empty request"s);
        }
        // все еще 1439 запросов с нулевым результатом
        request_queue.AddFindRequest("curly dog"s);
        // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
        request_queue.AddFindRequest("big collar"s);
        // первый запрос удален, 1437 запросов с нулевым результатом
        request_queue.AddFindRequest("sparrow"s);
        cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
    }
    cout << endl << endl;

    // параллельная обработка нескольких запросов
    {
        SearchServer search_server("and with"s);

            int id = 0;
            for (
                const string& text : {
                    "funny pet and nasty rat"s,
                    "funny pet with curly hair"s,
                    "funny pet and not very nasty rat"s,
                    "pet with rat and rat and rat"s,
                    "nasty rat with curly hair"s,
                }
            ) {
                search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, {1, 2});
            }

            const vector<string> queries = {
                "nasty rat -not"s,
                "not very funny nasty pet"s,
                "curly hair"s
            };
            id = 0;
            for (
                const auto& documents : ProcessQueries(search_server, queries)
            ) {
                cout << documents.size() << " documents for query ["s << queries[static_cast<size_t>(id++)] << "]"s << endl;
            }

            cout << endl << endl;
            for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
                cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
            }

    }


    return 0;
}
