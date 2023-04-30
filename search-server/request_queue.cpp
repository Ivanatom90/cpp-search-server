#include "request_queue.h"

using namespace std;

vector<Document> RequestQueue::AddFindRequest (const string_view& raw_query, DocumentStatus status) {
         if (requests_.size()==min_in_day_){
            if(requests_.front().documents_find.size() == 0 ){
                --empty_request_counter;
            }
          requests_.pop_front();
        }

        vector <Document> vector_result = search_server_.FindTopDocuments(raw_query, status);
        requests_.push_back({raw_query, vector_result});
        if(vector_result.size() == 0){
            ++empty_request_counter;
        }

 return vector_result;
}

vector<Document> RequestQueue::AddFindRequest(const string_view& raw_query) {
           if (requests_.size()==min_in_day_){
            if(requests_.front().documents_find.size() == 0 ){
                empty_request_counter--;
            }
          requests_.pop_front();
        }

    vector <Document> vector_result = search_server_.FindTopDocuments(raw_query);
    requests_.push_back({raw_query, vector_result});
    if(vector_result.size()==0){
        empty_request_counter++;
    }
return vector_result;
}


int RequestQueue::GetNoResultRequests() const {
    return empty_request_counter;
}
