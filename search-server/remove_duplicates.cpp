#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server){
    std::vector <int> id_v;
    std::set <int> doc_duble;
    std::vector <std::set<std::string>> documents;
    for (auto [id, words]:search_server.document_to_words_freqs_){
        std::set <std::string> words_in_doc;

        if(!words.empty()){
        for (auto [word, n]:words){

            words_in_doc.insert(word);
            }
        id_v.push_back(id);
        documents.push_back(words_in_doc);
        }

    }


    for(int i = 0; i < documents.size(); i++){
           for (int j =  documents.size(); j >i; j--){
               if (documents[i] == documents[j] && i != j){
                    doc_duble.insert(id_v[j]);

               }
           }
    }

    for (int id_del:doc_duble){
        std::cout<<"Found duplicate document id "<<id_del<<std::endl;
        search_server.RemoveDocument(id_del);
    }
}
