#pragma once
#include "search_server.h"
#include <deque>


class RequestQueue {
public:
    explicit RequestQueue( SearchServer& search_server) : search_server_(search_server),  empty_request_counter(0){
    }
    // сделаем "обёртки" для всех методов поиска, чтобы сохранять результаты для нашей статистики
    template <typename DocumentPredicate>
     std::vector<Document> AddFindRequest(const  std::string& raw_query, DocumentPredicate document_predicate);


     std::vector<Document> AddFindRequest (const  std::string& raw_query, DocumentStatus status);


     std::vector<Document> AddFindRequest(const  std::string& raw_query);


    int GetNoResultRequests() const;

private:
    struct QueryResult {
         std::string query;
         std::vector <Document> documents_find;
        // определите, что должно быть в структуре
    };
     std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
     std::vector<Document> doc;

    SearchServer& search_server_;
    int empty_request_counter;
    // возможно, здесь вам понадобится что-то ещё
};


