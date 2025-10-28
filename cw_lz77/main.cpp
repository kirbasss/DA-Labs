#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cctype>

struct Token {
    int offset;
    int length;
    char ch;
    bool has_char;
};

std::vector<Token> lz77_compress(const std::string &s) {
    std::vector<Token> result;
    int n = static_cast<int>(s.size());
    int pos = 0;

    while (pos < n) {
        int best_len = 0;
        int best_offset = 0;

        for (int start = 0; start < pos; ++start) {
            int len = 0;
            while (pos + len < n && start + len < pos && s[start + len] == s[pos + len]) {
                ++len;
            }
            if (len > best_len || (len == best_len && len > 0 && pos - start < best_offset)) {
                best_len = len;
                best_offset = pos - start;
            }
        }

        if (best_len == 0) {
            result.push_back({0, 0, s[pos], true});
            pos += 1;
        } else {
            if (pos + best_len < n) {
                result.push_back({best_offset, best_len, s[pos + best_len], true});
                pos += best_len + 1;
            } else {
                result.push_back({best_offset, best_len, '\0', false});
                pos += best_len;
            }
        }
    }

    return result;
}

std::string lz77_decompress(const std::vector<Token> &tokens) {
    std::string result;

    for (const Token &t : tokens) {
        if (t.offset == 0 && t.length == 0) {
            if (t.has_char) result.push_back(t.ch);
        } else {
            int start = static_cast<int>(result.size()) - t.offset;
            if (start < 0)
                throw std::runtime_error("Некорректный offset");

            for (int i = 0; i < t.length; ++i)
                result.push_back(result[start + i]);

            if (t.has_char)
                result.push_back(t.ch);
        }
    }

    return result;
}

void trim(std::string &s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    s = s.substr(start, end - start);
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string command;
    if (!std::getline(std::cin, command))
        return 0;

    trim(command);

    if (command == "compress") {
        std::string text;
        if (!std::getline(std::cin, text)) text = "";
        trim(text);

        std::vector<Token> tokens = lz77_compress(text);

        for (const Token &t : tokens) {
            if (t.has_char)
                std::cout << t.offset << ' ' << t.length << ' ' << t.ch << '\n';
            else
                std::cout << t.offset << ' ' << t.length << '\n';
        }
    } else if (command == "decompress") {
        std::vector<Token> tokens;
        std::string line;

        while (std::getline(std::cin, line)) {
            trim(line);
            if (line.empty()) continue;

            std::stringstream ss(line);
            Token t;
            if (!(ss >> t.offset >> t.length))
                continue;

            std::string rest;
            if (ss >> rest) {
                t.ch = rest[0];
                t.has_char = true;
            } else {
                t.ch = '\0';
                t.has_char = false;
            }

            tokens.push_back(t);
        }

        try {
            std::string result = lz77_decompress(tokens);
            std::cout << result << '\n';
        } catch (const std::exception &e) {
            std::cerr << "Ошибка при разжатии: " << e.what() << '\n';
            return 1;
        }
    }

    return 0;
}
