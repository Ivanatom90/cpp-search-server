#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <cassert>

//#include "search_server.h"

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
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
    int rating;
};

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};


/* Подставьте вашу реализацию класса SearchServer сюда */
class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
                     const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
    return 	FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const {
            auto fun_predicate = [status](int document_id, DocumentStatus status_, int rating){
                return status == status_;
            };
            return FindTopDocuments(raw_query, fun_predicate);
    }






    template <typename Statustype>
    vector<Document> FindTopDocuments(const string& raw_query, Statustype status) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, status);
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (abs(lhs.relevance - rhs.relevance) < 1e-6) {
                     return lhs.rating > rhs.rating;
                 } else {
                     return lhs.relevance > rhs.relevance;
                 }
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
                                                        int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return {matched_words, documents_.at(document_id).status};
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;

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

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;
        for (const int rating : ratings) {
            rating_sum += rating;
        }
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {text, is_minus, IsStopWord(text)};
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }
    template <typename Statustype>
    vector<Document> FindAllDocuments(const Query& query, Statustype status) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                DocumentStatus status_doc = documents_.at(document_id).status;
                int rating_doc = documents_.at(document_id).rating;
                if (status(document_id, status_doc, rating_doc)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }
};

template<typename T, typename U>
void ATestEqual(const T& t, const U& u, const string str_t, const string str_u, const string file, const string function, const int line, const string hint)
{
    if (t != u)
    {
        cerr<<t<<" "<<u<<" "<<str_t<<" "<<str_u<<" "<< file<<" "<<function<<" "<<line<<endl;
    }
}


/*
   Подставьте сюда вашу реализацию макросов
   ASSERT, ASSERT_EQUAL, ASSERT_EQUAL_HINT, ASSERT_HINT и RUN_TEST
*/

#define ASSERT_EQUAL(a, b) ATestEqual(a, b, #a, #b, __FILE__, __FUNCTION__, __LINE__, "")
#define ASSERT_HINT(a, hint) ATestEqual(a, true, #a, "true", __FILE__, __FUNCTION__, __LINE__, hint)
#define ASSERT(a) ATestEqual(a, true, #a, "true", __FILE__, __FUNCTION__, __LINE__, "")
#define RUN_TEST(func) func;

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in");
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the");
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in").empty(),
                    "top words must be excluded from documents");
    }
}

/*
Разместите код остальных тестов здесь
*/

void TestAddDocument(){
    const int doc_id = 42;
    const string content = "cat in the city";
    const vector<int> ratings = {1, 2, 3};

    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
    ASSERT_EQUAL(server.FindTopDocuments("cat city").size(), 1);
    ASSERT_EQUAL(server.FindTopDocuments("cat city")[0].id, doc_id);
}

void TestMinusWords(){
    const int doc_id = 42;
    const string content = "cat in the city good kitty";
    const vector<int> ratings = {1, 2, 3};
    SearchServer server;
    server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);

    ASSERT_EQUAL(server.FindTopDocuments("cat -kitty").size(), 0);
    ASSERT_EQUAL(server.FindTopDocuments("-city").size(), 0);
}

void TestMachedDoc(){
    const string content = "cat in the city good kitty";
    const vector<int> ratings = {1, 2, 3};
    SearchServer server;
    server.AddDocument(0, content, DocumentStatus::ACTUAL, ratings);
    const string content1 = "big dog in the city";
    server.AddDocument(1, content1, DocumentStatus::ACTUAL, ratings);

    double a,b;
    a = server.FindTopDocuments("cat")[0].relevance;
    b = server.FindTopDocuments("cat")[1].relevance;
    auto c = [](const double& rh, const double& hr)
    {
        return rh>hr;
    };
    ASSERT(c(a,b));
}

void TestMidRating(){
    const string content = "cat in the city good kitty";
    const vector<int> ratings = {1, 3, 5};
    SearchServer server;
    server.AddDocument(0, content, DocumentStatus::ACTUAL, ratings);
    ASSERT_EQUAL(server.FindTopDocuments("cat")[0].rating, 3);
}


void TestStatus(){
    const string content = "cat in the city good kitty";
    const vector<int> ratings = {1, 3, 5};
    SearchServer server;
    const string content1 = "cat in the city good dog";
    server.AddDocument(0, content, DocumentStatus::ACTUAL, ratings);
    server.AddDocument(1, content1, DocumentStatus::BANNED, ratings);
    ASSERT_EQUAL(server.FindTopDocuments("cat", DocumentStatus::BANNED)[0].id, 1);

}

void TestRel(){
    const string content = "cat in the city good kitty";
    const vector<int> ratings = {1, 3, 5};
    SearchServer server;
    server.SetStopWords("in the");
    server.AddDocument(0, content, DocumentStatus::ACTUAL, ratings);
    const double& a = server.FindTopDocuments("cat")[0].relevance - (1.00/4.00);
    bool c;
    if (a<1E-6){c = true;};
    ASSERT(c);

}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    TestExcludeStopWordsFromAddedDocumentContent();
    TestAddDocument();
    TestMinusWords();
    TestMachedDoc();
    TestMidRating();
    TestStatus();
    TestRel();
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished" << endl;
}
