#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <algorithm>


std::vector<std::tuple<int,int,char,bool>> lz77_compress(const std::string &s) {
    int n = static_cast<int>(s.size());
    int pos = 0;
    std::vector<std::tuple<int,int,char,bool>> result;

    while (pos < n) {
        int best_len = 0;
        int best_offset = 0;

        // ищем самое длинное совпадение в уже просмотренной части строки
        for (int start = 0; start < pos; ++start) {
            int len = 0;
            while (pos + len < n && start + len < pos && s[start + len] == s[pos + len]) {
                ++len;
            }
            if (len > best_len) {
                best_len = len;
                best_offset = pos - start;
            } else if (len == best_len && len > 0) {
                int off = pos - start;
                if (best_offset == 0 || off < best_offset) best_offset = off;
            }
        }

        if (best_len == 0) {
            // совпадений нет — просто записываем текущий символ
            result.emplace_back(0, 0, s[pos], true);
            pos += 1;
        } else {
            if (pos + best_len < n) {
                result.emplace_back(best_offset, best_len, s[pos + best_len], true);
                pos += best_len + 1;
            } else {
                // совпадение до конца текста — без последнего символа
                result.emplace_back(best_offset, best_len, '\0', false);
                pos += best_len;
            }
        }
    }

    return result;
}

std::string lz77_decompress(const std::vector<std::tuple<int,int,char,bool>> &tokens) {
    std::string result;

    for (const auto &t : tokens) {
        int offset = std::get<0>(t);
        int length = std::get<1>(t);
        char ch = std::get<2>(t);
        bool has_char = std::get<3>(t);

        if (offset == 0 && length == 0) {
            // просто добавляем символ
            if (has_char) result.push_back(ch);
        } else {
            int start = static_cast<int>(result.size()) - offset;
            if (start < 0)
                throw std::runtime_error("Некорректный offset в токене");

            for (int i = 0; i < length; ++i)
                result.push_back(result[start + i]);

            if (has_char)
                result.push_back(ch);
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

        std::vector<std::tuple<int,int,char,bool>> tokens = lz77_compress(text);

        for (const auto &t : tokens) {
            int offset = std::get<0>(t);
            int length = std::get<1>(t);
            char ch = std::get<2>(t);
            bool has_char = std::get<3>(t);

            if (has_char)
                std::cout << offset << ' ' << length << ' ' << ch << '\n';
            else
                std::cout << offset << ' ' << length << '\n';
        }
    }
    else if (command == "decompress") {
        std::vector<std::tuple<int,int,char,bool>> tokens;
        std::string line;

        while (std::getline(std::cin, line)) {
            trim(line);
            if (line.empty()) continue;

            std::stringstream ss(line);
            int offset, length;
            if (!(ss >> offset >> length)) continue;

            std::string rest;
            if (ss >> rest) {
                tokens.emplace_back(offset, length, rest[0], true);
            } else {
                tokens.emplace_back(offset, length, '\0', false);
            }
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
