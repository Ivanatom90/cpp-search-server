#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char& c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};
struct QWords
{
    set<string> plus_words;
    set<string> minus_words;


};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        double word_count_doc = words.size();
        double TF = 0;
        for (const auto& word : words)
        {
            TF = count(words.begin(),words.end(), word)/word_count_doc;
            documents_[word].insert({document_id, TF});
        }

    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const QWords query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }


int document_count_ = 0;


private:



    map<string, map<int, double>>  documents_;


    set<string> stop_words_;



    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }




     QWords ParseQuery(const string& text) const {
        QWords QWords_;
        set<string> pwords;
        set<string> mwords;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            pwords.insert(word);
        }
        string mwir;
        for (const string& mw : pwords)
        {
            if (mw[0] == '-')
            {
                string str = "";

                for (const char& a: mw)
                {

                    if (a != '-')
                    {
                        str += a;
                        mwir = mw;
                    }

                }

                mwords.insert(str);

            }
        }
        pwords.erase(mwir);
        QWords_.plus_words = pwords;
        QWords_.minus_words = mwords;


        return QWords_;
     }
    vector<Document> FindAllDocuments(const QWords& query_words)  const{
        vector<Document> matched_documents;
        map <int, double> doc_relevance;

        map<string, map<int, double>> words_TF;
       // map<int, map<string, double>> doc_relevance;
        //for (const auto& document : documents_) {
        //map<string,set<int>> documents_;
        Document Doc;

        map <string, map<int, double>> words_q;



        for (const string& pword: query_words.plus_words)

        {

            if (documents_.count(pword) !=0)
            {


                double coun = documents_.at(pword).size();
                double IDF = log(document_count_/coun);
                map <int, double> word_id_TF = documents_.at(pword);
                for (auto& [id, tf] : documents_.at(pword))
                {


                    doc_relevance[id] += tf*IDF;

                }
            }





        }

/*
        for (const auto& a : doc_relevance)
        {
            for (const pair<string, double>& b: a.second)
            {
                doc_plus[a.first] += b.second;


            }
        }

        for (const pair <string, set<int>> a : documents_) //слово и список id документов где оно содержится
        {



            if (query_words.plus_words.count(a.first) != 0)
             {
                 for (const int id : a.second){
                 doc_plus[id] += 1;}
             }

             if (query_words.minus_words.count(a.first) == 0)
             {
                 for (const int id : a.second){
                 doc_plus.erase(id);
                }
             }

        }

*/


        for (const pair <int, double>& a : doc_relevance)
        {
            Doc.id = a.first;
            Doc.relevance = a.second;

            matched_documents.push_back(Doc);

        }

/*

            const int relevance = MatchDocument(documents_, query_words);
            if (relevance > 0)
        {
                doc_plus[documents_.] = relevance;
                matched_documents.push_back({document.id, relevance});
            }
        }

      */

        return matched_documents;
    }

    static int MatchDocument(const map<string,map<int, double>> content, const QWords& query_words) {
        if (query_words.plus_words.empty()) {
            return 0;
        }
        for (const auto& wm : content)
        {

            if (query_words.minus_words.count(wm.first) != 0) {return 0;}

         }


        set<string> matched_words;
        for (const pair<string,map<int, double>> word : content) {

            if (matched_words.count(word.first) != 0) {
                continue;
            }
            if (query_words.plus_words.count(word.first) != 0) {
                matched_words.insert(word.first);
            }
        }
        return static_cast<int>(matched_words.size());
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    //search_server.SetStopWords("is are was a an in the with near at");
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    //const int document_count = 3;
    search_server.document_count_ = document_count;

    vector<string> doc_str;
    doc_str.push_back("a colorful parrot with green wings and red tail is lost");
    doc_str.push_back("a grey hound with black ears is found at the railway station");
    doc_str.push_back("a white cat with long furry tail is found near the red square");

    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
        //search_server.AddDocument(document_id, doc_str[document_id]);


    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    //const string query = "white cat long tail";
    for ( auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = " << document_id << ", "<< "relevance = "<< relevance << " }"<< endl;
    }
}
