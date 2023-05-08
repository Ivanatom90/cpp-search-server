#include "search_server.h"

using namespace std;

SearchServer::SearchServer(std::string_view stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
{
}

SearchServer::SearchServer(const std::string& stop_words_text)
  : SearchServer(SplitIntoWords(stop_words_text))// Invoke delegating constructor from string container
{
}

void SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status,
                 const vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw invalid_argument("Invalid document_id");
    }
    if (document.empty()){
        return;
    }
    str_in_doc.push_back(std::string(document));
    const auto words = SplitIntoWordsNoStop(std::string_view(str_in_doc.back()));

    const double inv_word_count = 1.0 / words.size();
    for (const string_view word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        document_to_words_freqs_[document_id][word] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
    document_ids_.insert(document_id);
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
            return document_status == status;
        });
}

vector<Document> SearchServer::FindTopDocuments(string_view raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}




int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument( string_view raw_query,
                                                    int document_id) const {
    const auto query = ParseQuery(raw_query, true);

    std::vector<std::string_view> matched_words;

    for (string_view word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            return {std::vector<std::string_view>{}, documents_.at(document_id).status};
        }
    }

    for (  string_view word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }

    return {matched_words, documents_.at(document_id).status};
}

std::tuple< std::vector< std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, std::string_view raw_query,
                                                                     int document_id) const {

   return MatchDocument(raw_query, document_id);
}

std::tuple< std::vector< std::string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::parallel_policy&, std::string_view raw_query,
                                                                     int document_id) const {
   auto query = ParseQuery(raw_query, false);
  std::vector<std::string_view> matched_words;
  bool doc_have_minus_word = false;

  doc_have_minus_word = std::any_of(std::execution::par, query.minus_words.begin(), query.minus_words.end(),
                        [this, &document_id](std::string_view rhs){
     return (0 != document_to_words_freqs_.at(document_id).count(rhs));
  });

  if (doc_have_minus_word) {
      return {matched_words, documents_.at(document_id).status};
  }


  matched_words.resize(query.plus_words.size());
  auto last_word = std::copy_if(std::execution::par, query.plus_words.begin(), query.plus_words.end(),
               matched_words.begin(), [this, &document_id]( std::string_view rhs){
        return 0 != document_to_words_freqs_.at(document_id).count(rhs);;
  });


  std::sort(std::execution::par, matched_words.begin(), last_word);
  last_word = std::unique(std::execution::par,matched_words.begin(), last_word);
  matched_words.erase(last_word, matched_words.end());

 return {matched_words, documents_.at(document_id).status};

}


bool SearchServer::IsStopWord( std::string_view word) const {
    return stop_words_.count(std::string(word)) > 0;
}

 bool SearchServer::IsValidWord( string_view word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}



 std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop( string_view text) const {
     vector<std::string_view> words;
     for ( std::string_view word : SplitIntoWords(text)) {
         if (!IsValidWord(word)) {
             throw invalid_argument("Word  is invalid");
         }
         if (!IsStopWord(word)) {
             words.push_back(word);
         }
     }
     return words;
 }

  int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
     if (ratings.empty()) {
         return 0;
     }
     int rating_sum = 0;
     rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
     return rating_sum / static_cast<int>(ratings.size());
 }

  SearchServer::QueryWord SearchServer::ParseQueryWord( string_view text) const {
      if (text.empty()) {
          throw invalid_argument("Query word is empty");
      }
      std::string_view word(text);
      bool is_minus = false;
      if (word[0] == '-') {
          is_minus = true;
          word = word.substr(1);
      }
      if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
          throw invalid_argument("Query word is invalid");
      }

      return {word, is_minus, IsStopWord(word)};
  }


  SearchServer::Query SearchServer::ParseQuery( string_view text, bool seq) const {
      Query result;
      for (string_view word : SplitIntoWords(text)) {
          const auto query_word = ParseQueryWord(word);
          if (!query_word.is_stop) {
              if (query_word.is_minus) {
                  result.minus_words.push_back(query_word.data);
              } else {
                  result.plus_words.push_back(query_word.data);
              }
          }

      }
      if(seq){
          std::sort( result.minus_words.begin(), result.minus_words.end());
          std::sort(result.plus_words.begin(), result.plus_words.end());
          auto it_minus = std::unique(result.minus_words.begin(), result.minus_words.end());
          auto it_plus = std::unique(result.plus_words.begin(), result.plus_words.end());
          result.minus_words.erase(it_minus, result.minus_words.end());
          result.plus_words.erase(it_plus, result.plus_words.end());
      }

      return result;
  }

  double SearchServer::ComputeWordInverseDocumentFreq(string_view word) const {
      return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
  }

  std::set<int>::const_iterator SearchServer::begin(){
      return document_ids_.begin();
  }

  std::set<int>::const_iterator SearchServer::end(){
      return document_ids_.end();
  }

  const  std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
     if (document_to_words_freqs_.at(document_id).empty()){
      const static map<std::string_view, double> empty_map{};
      return empty_map;
      }
return document_to_words_freqs_.at(document_id);
 }

  void SearchServer::RemoveDocument(int document_id){
      if(document_to_words_freqs_.at(document_id).empty()){
          return;
      }

      for (pair<string_view, double> a : document_to_words_freqs_[document_id]){
                if(word_to_document_freqs_[a.first].count(document_id)){
                     word_to_document_freqs_[a.first].erase(document_id);
                     if (word_to_document_freqs_[a.first].empty()){
                         word_to_document_freqs_.erase(a.first);
                     }
                }
      }

      document_to_words_freqs_.erase(document_id);
      documents_.erase(document_id);
      auto it = find(document_ids_.begin(), document_ids_.end(), document_id);
      if (it != document_ids_.end()){
          document_ids_.erase(it);
      }

  }


  void SearchServer::RemoveDocument(const std::execution::sequenced_policy&,int document_id){
      if(document_to_words_freqs_.at(document_id).empty()){
          return;
      }
      std::map<std::string_view, double>& dtwf = document_to_words_freqs_[document_id];
      std::vector<const std::string_view*> words_in_doc_id_vector(dtwf.size());
      std::transform(std::execution::seq, dtwf.begin(), dtwf.end(), words_in_doc_id_vector.begin(), [](std::pair<const std::string_view, double>& rhs){
          return &rhs.first;
      });
      std::for_each(std::execution::seq, words_in_doc_id_vector.begin(), words_in_doc_id_vector.end(), [document_id, this](const std::string_view *rhs){
          word_to_document_freqs_[*rhs].erase(document_id);
      });

      document_to_words_freqs_.erase(document_id);
      documents_.erase(document_id);
      auto it = find(document_ids_.begin(), document_ids_.end(), document_id);
      if (it != document_ids_.end()){
          document_ids_.erase(it);
      }

  }

  void SearchServer::RemoveDocument(const std::execution::parallel_policy&, int document_id){
      if(document_to_words_freqs_.at(document_id).empty()){
          return;
      }


      std::map<std::string_view, double>& dtwf = document_to_words_freqs_[document_id];
      std::vector<const std::string_view*> words_in_doc_id_vector(dtwf.size());
      std::transform(std::execution::par, dtwf.begin(), dtwf.end(), words_in_doc_id_vector.begin(), [](const std::pair<const std::string_view, double>& rhs){
          return &rhs.first;
      });
      std::for_each(std::execution::par, words_in_doc_id_vector.begin(), words_in_doc_id_vector.end(), [document_id, this](const std::string_view *rhs){
          word_to_document_freqs_[*rhs].erase(document_id);
      });

      document_to_words_freqs_.erase(document_id);
      documents_.erase(document_id);
      auto it = find(document_ids_.begin(), document_ids_.end(), document_id);
      if (it != document_ids_.end()){
          document_ids_.erase(it);
      }

  }
