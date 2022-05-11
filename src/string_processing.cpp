#include "string_processing.h"

#include <sstream>

using namespace std;

vector<string> SplitIntoWords(const string &text) {
    vector<string> words;
    string word;
    std::istringstream iss(text, std::istringstream::in);

    while(iss >> word) {
            words.push_back(word);
    }

    return words;
}

vector<string_view> SplitIntoWordsView(string_view str) {
    vector<string_view> result;
    const int64_t pos_end = static_cast<int64_t>(str.npos);
    while (true) {
        int64_t space = static_cast<int64_t>(str.find(' '));
        string_view s = str.substr(0, static_cast<size_t>(space));
        if (!s.empty()) {
            result.push_back(s);
        }
        if (space == pos_end) {
            break;
        } else {
            str.remove_prefix(static_cast<size_t>(space+1));
        }
    }
    return result;
}
