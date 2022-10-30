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
        double part_of_word = 1.0/word_count_doc;

        for (const auto& word : words)
        {


            documents_[word][document_id] += part_of_word;
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
        QWords qwords_;
        set<string> pwords;
        set<string> mwords;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-')
            {
                mwords.insert(word.substr(1));

            } else
            {
               pwords.insert(word);
            }

        }

        qwords_.plus_words = move(pwords);
        qwords_.minus_words = move(mwords);
        return qwords_;
     }

    vector<Document> FindAllDocuments(const QWords& query_words)  const{
        vector<Document> matched_documents;
        map <int, double> doc_relevance;
        map<string, map<int, double>> words_TF;
        Document Doc;
        map <string, map<int, double>> words_q;

        for (const string& pword: query_words.plus_words)
        {
            if (documents_.count(pword) !=0)
            {
                double coun = documents_.at(pword).size();
                double IDF = log(document_count_/coun);
                const map <int, double>& word_id_TF = documents_.at(pword);
                for (auto& [id, tf] : word_id_TF)
                {
                    doc_relevance[id] += tf*IDF;
                }
            }
        }

        for (const pair <int, double>& a : doc_relevance)
        {
            Doc.id = a.first;
            Doc.relevance = a.second;
            matched_documents.push_back(Doc);

        }
        return matched_documents;
    }

    static int MatchDocument(const map<string,map<int, double>> content, const QWords& query_words) {
        if (query_words.plus_words.empty()) {
            return 0;
        }
        for (const auto& minus_word : content)
        {

            if (query_words.minus_words.count(minus_word.first) != 0) {return 0;}

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

    //const string query = ReadLine();
    const string query = "white cat long tail";
    for ( auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = " << document_id << ", "<< "relevance = "<< relevance << " }"<< endl;
    }
}
