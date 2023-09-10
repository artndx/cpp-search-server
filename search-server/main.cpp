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
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
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

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }
    
    void AddDocument(int document_id, const string& document_string) {
        if (!document_string.empty()) {
            const vector<string> document = SplitIntoWordsNoStop(document_string);
            double TF = 0.;
            double word_count = 0.;
            for (string word : document) {

                word_count = count_if(document.begin(), document.end(),
                    [&word](const string& s) {
                    return s == word; });
                TF = word_count / document.size();
                documents_[word].insert({ document_id, TF });
            }
            TF = 0.;
            ++document_count_;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
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

private:
    map<string, map<int, double>> documents_;
    double document_count_ = 0.;
    set<string> stop_words_;

    struct Query
    {
        set<string> plus_words;
        set<string> minus_words;
    };

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

    Query ParseQuery(const string& text) const {
        Query query_words;
        for (string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-'){
                word = word.substr(1);
                if (stop_words_.count(word) == 0){
                    query_words.minus_words.insert(word);
                }
            } else{
                query_words.plus_words.insert(word);
            }
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        map<int, double > id_relevance;
        for (const auto& plus_word : query_words.plus_words) {
            double IDF = 0.;
            if (documents_.count(plus_word)) {
                IDF = log(static_cast<double>(document_count_) / 
                    static_cast<double>(documents_.at(plus_word).size()));
            }
            else {
                continue;
            }

            for (const auto& [id, TF] : documents_.at(plus_word)) {
                id_relevance[id] += IDF * TF;
            }
            for (const auto& minus_word : query_words.minus_words) {
                if (documents_.count(minus_word)) {
                    for (const auto& [id, TF] : documents_.at(minus_word)) {
                        id_relevance.erase(id);
                    }
                }
            }
        }
        for (const auto& [id, relevance] : id_relevance) {
            matched_documents.push_back({ id,relevance });
        }


        return matched_documents;

    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }
}