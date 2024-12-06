#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <string>

using namespace std;

class GrepOption {
public:
    bool ignoreCase = false;
    bool lineNumber = false;
    bool invertMatch = false;
    bool countMatches = false;
    bool filesWithMatches = false;
    bool noFilename = false;
    bool suppressErrors = false;
    bool matchOnly = false;
    string patternFile = "";
    vector<string> extraPatterns;

    GrepOption() {}

    void setFlag(char flag) {
        switch (flag) {
        case 'i': ignoreCase = true; break;
        case 'n': lineNumber = true; break;
        case 'v': invertMatch = true; break;
        case 'c': countMatches = true; break;
        case 'l': filesWithMatches = true; break;
        case 'h': noFilename = true; break;
        case 's': suppressErrors = true; break;
        case 'o': matchOnly = true; break;
        default:
            cerr << "Неизвестная опция: " << flag << endl;
            throw invalid_argument("Неизвестная опция");
        }
    }
};

class GrepSearch {
private:
    GrepOption options;
    vector<regex> patterns;
    int matchCount;

    void printMatch(const string& line, const string& filename, int lineNum) const {
        if (options.countMatches || options.filesWithMatches) {
            return;
        }
        if (options.matchOnly) {
            for (const auto& pat : patterns) {
                smatch match;
                if (regex_search(line, match, pat)) {
                    cout << match.str() << endl;
                }
            }
        }
        else {
            if (options.lineNumber) {
                cout << (options.noFilename ? "" : filename + ":") << lineNum << ":" << line << endl;
            }
            else {
                cout << (options.noFilename ? "" : filename + ":") << line << endl;
            }
        }
    }

    bool searchLine(const string& line) {
        bool foundMatch = false;
        for (const auto& pat : patterns) {
            if (regex_search(line, pat)) {
                foundMatch = true;
                if (!options.invertMatch) {
                    return true;
                }
            }
        }
        return options.invertMatch ? !foundMatch : foundMatch;
    }

public:
    GrepSearch(const GrepOption& opt) : options(opt), matchCount(0) {
        if (!options.patternFile.empty()) {
            ifstream file(options.patternFile);
            if (!file.is_open()) {
                throw runtime_error("Не удалось открыть файл с шаблонами: " + options.patternFile);
            }
            string line;
            while (getline(file, line)) {
                if (!line.empty()) {
                    patterns.push_back(regex(line, options.ignoreCase ? regex_constants::icase : regex_constants::ECMAScript));
                }
            }
        }
        for (const auto& pat : options.extraPatterns) {
            addPattern(pat);
        }
    }

    void addPattern(const string& pat) {
        if (!pat.empty()) {
            patterns.push_back(regex(pat, options.ignoreCase ? regex_constants::icase : regex_constants::ECMAScript));
        }
    }

    void searchFile(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            if (!options.suppressErrors) {
                cerr << "Не удалось открыть файл: " << filename << endl;
            }
            return;
        }
        string line;
        int lineNum = 1;
        bool fileMatched = false;
        matchCount = 0;

        while (getline(file, line)) {
            if (searchLine(line)) {
                fileMatched = true;
                printMatch(line, filename, lineNum);
                if (!options.countMatches && !options.filesWithMatches) {
                    matchCount++;
                }
            }
            lineNum++;
        }
        if (options.countMatches) {
            cout << filename << ": " << matchCount << endl;
        }
        if (options.filesWithMatches && fileMatched) {
            cout << filename << endl;
        }
    }
};

int main() {
    setlocale(LC_ALL, "Russian");
    GrepOption options;
    vector<string> files;

    cout << "Введите опции: ";
    string flags;
    getline(cin, flags);

    for (const char& flag : flags) {
        options.setFlag(flag);
    }

    string inputPath;
    cout << "Введите путь к файлу или его название: ";
    getline(cin, inputPath);
    if (!inputPath.empty()) {
        files.push_back(inputPath);
    }
    else {
        cerr << "Ошибка: путь к файлу не может быть пустым." << endl;
        return 1;
    }

    string pattern;
    cout << "Введите ключевые слова: ";
    getline(cin, pattern);
    if (!pattern.empty()) {
        options.extraPatterns.push_back(pattern);
    }
    else {
        cerr << "Ошибка: ключевые слова не могут быть пустыми." << endl;
        return 1;
    }

    try {
        GrepSearch grep(options);
        for (const auto& file : files) {
            grep.searchFile(file);
        }
    }
    catch (const exception& e) {
        cerr << "Ошибка: " << e.what() << endl;
        return 1;
    }

    return 0;
}