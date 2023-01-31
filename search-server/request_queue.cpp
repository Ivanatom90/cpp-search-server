#include "request_queue.h"

using namespace std;

vector<Document> RequestQueue::AddFindRequest (const string& raw_query, DocumentStatus status) {
    // напишите реализацию
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

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    // напишите реализацию
           if (requests_.size()==min_in_day_){
            if(requests_.front().documents_find.size() == 0 ){
                empty_request_counter--;
                  //  cout<<empty_request_counter<<"    ";
            }
          requests_.pop_front();
        }

    vector <Document> vector_result = search_server_.FindTopDocuments(raw_query);
    requests_.push_back({raw_query, vector_result});
    if(vector_result.size()==0){
        empty_request_counter++;
        //cout<<empty_request_counter<<"     "<<vector_result.size()<<endl;
    }
return vector_result;
}


int RequestQueue::GetNoResultRequests() const {
    return empty_request_counter;
}

template <typename DocumentPredicate>
 std::vector<Document> RequestQueue::AddFindRequest(const  std::string& raw_query, DocumentPredicate document_predicate) {
        if (requests_.size()==min_in_day_){
           if(requests_.front().documents_find.size() == 0 ){
                --empty_request_counter;
            }
          requests_.pop_front();
        }

     std::vector <Document> vector_result = search_server_.FindTopDocuments(raw_query, document_predicate);
    requests_.push_back({raw_query, vector_result});
    if (vector_result.size() == 0){
        ++empty_request_counter;
    }

    return vector_result;


    // напишите реализацию
}
