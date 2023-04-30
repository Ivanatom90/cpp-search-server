#include "process_queries.h"

std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
        const std::vector<std::string>& queries){
        std::vector<std::vector<Document>> doc_queries(queries.size());
        std::transform(std::execution::par, queries.begin(), queries.end(), doc_queries.begin(), [&search_server]( std::string_view text){
        return search_server.FindTopDocuments(text);
    });
return doc_queries;

}

std::vector<Document> ProcessQueriesJoined(
    const SearchServer& search_server,
        const std::vector<std::string>& queries){
        std::vector<Document> q_docs;
        std::vector<std::vector<Document>> docs = ProcessQueries(search_server, queries);
        for (std::vector<Document> q_doc:ProcessQueries(search_server, queries)){
            q_docs.insert(q_docs.end(),q_doc.begin(), q_doc.end());
        }
return q_docs;

}
