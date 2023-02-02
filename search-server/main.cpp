#include <iostream>
#include <stdexcept>
#include "search_server.h"
#include "request_queue.h"
#include "paginator.h"
#include "remove_duplicates.h"
#include "document.h"

using namespace std;


void Test2(){
    SearchServer search_server(std::string("and with"));
    search_server.AddDocument(1, std::string("funny pet and nasty rat"), DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, std::string("funny pet with curly hair"), DocumentStatus::ACTUAL, { 1, 2, 3 });
    search_server.AddDocument(3, std::string("big cat nasty hair"), DocumentStatus::ACTUAL, { 1, 2, 8 });
    search_server.AddDocument(4, std::string("big dog cat Vladislav"), DocumentStatus::ACTUAL, { 1, 3, 2 });
    search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, { 1, 1, 1 });
    const auto search_results = search_server.FindTopDocuments("curly dog"s);
    //const vector<Document> search_results = search_server.FindTopDocuments(std::string("curly dog"));
    size_t page_size = 2;
    //const auto pages  = Paginate(search_results, page_size);
    const auto pages = Paginate(search_results, page_size);
    //Paginator<vector<Document>::const_iterator> pages_  = Paginate(search_results, page_size);
    //Paginator<std::vector<Document>::iterator> pages(search_results.begin(),search_results.end(), page_size);
    // Выводим найденные документы по страницам
    for (auto page = pages.page.begin(); page != pages.page.end(); ++page) {
        cout << *page << endl;
        cout << "Page break";
        cout << endl;
   }
}

int main() {
    SearchServer search_server(string("and with"));

    search_server.AddDocument( 1, string("funny pet and nasty rat"), DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, string("funny pet with curly hair"), DocumentStatus::ACTUAL, {1, 2});

    // дубликат документа 2, будет удалён
    search_server.AddDocument(3, string("funny pet with curly hair"), DocumentStatus::ACTUAL, {1, 2});

    // отличие только в стоп-словах, считаем дубликатом
    search_server.AddDocument(4, string("funny pet and curly hair"), DocumentStatus::ACTUAL, {1, 2});

    // множество слов такое же, считаем дубликатом документа 1
    search_server.AddDocument( 5, string("funny funny pet and nasty nasty rat"), DocumentStatus::ACTUAL, {1, 2});

    // добавились новые слова, дубликатом не является
    search_server.AddDocument( 6, string("funny pet and not very nasty rat"), DocumentStatus::ACTUAL, {1, 2});

    // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
    search_server.AddDocument( 7, string("very nasty rat and not very funny pet"), DocumentStatus::ACTUAL, {1, 2});

    // есть не все слова, не является дубликатом
    search_server.AddDocument( 8, string("pet with rat and rat and rat"), DocumentStatus::ACTUAL, {1, 2});

    // слова из разных документов, не является дубликатом
    search_server.AddDocument( 9, string("nasty rat with curly hair"), DocumentStatus::ACTUAL, {1, 2});

    cout << "Before duplicates removed: " << search_server.GetDocumentCount() << endl;
    RemoveDuplicates(search_server);
    cout << "After duplicates removed: " << search_server.GetDocumentCount() << endl;
    Test2();
}
