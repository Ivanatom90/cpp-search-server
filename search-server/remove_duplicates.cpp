#include "remove_duplicates.h"

void RemoveDuplicates(SearchServer& search_server){
    std::set <int> doc_duble;
    std::set<std::string_view> words;
    std::set <std::set<std::string_view>> documents;

    for (auto a = search_server.begin(); a != search_server.end(); a++){
        if (search_server.GetWordFrequencies(*a).empty()){
            doc_duble.insert(*a);
            continue;
        }
        for (auto [word, fregs]:search_server.GetWordFrequencies(*a)){
            words.insert(word);
        }
        if (count(documents.begin(), documents.end(), words)!=0){
            doc_duble.insert(*a);
        } else {
       documents.insert(words);
    }
    words.clear();
 }
        for (int id_del:doc_duble){
            std::cout<<"Found duplicate document id "<<id_del<<std::endl;
            search_server.RemoveDocument(id_del);
        }


}

