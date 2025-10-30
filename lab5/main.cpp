#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <limits>

struct Token {
    int offset;
    int length;
    char ch;
    bool has_char;
};

std::vector<int> build_sa(const std::string &s) {
    std::string str = s + "$";
    int n = (int)str.size();
    std::vector<int> p(n), c(n);

    // sort by single character
    {
        std::vector<std::pair<char,int>> a(n);
        for (int i = 0; i < n; ++i) a[i] = {str[i], i};
        std::sort(a.begin(), a.end());
        for (int i = 0; i < n; ++i) p[i] = a[i].second;
        c[p[0]] = 0;
        for (int i = 1; i < n; ++i) {
            c[p[i]] = c[p[i-1]] + (a[i].first != a[i-1].first);
        }
    }

    int k = 0;
    std::vector<int> pn(n), cn(n);
    while ((1 << k) < n) {
        int len = 1 << k;
        for (int i = 0; i < n; ++i)
            pn[i] = (p[i] - len + n) % n;

        // counting sort
        std::vector<int> cnt(n, 0);
        for (int x : c) cnt[x]++;
        std::vector<int> pos(n);
        pos[0] = 0;
        for (int i = 1; i < n; ++i) pos[i] = pos[i-1] + cnt[i-1];
        for (int x : pn) {
            int cl = c[x];
            p[pos[cl]++] = x;
        }

        // recalc classes
        cn[p[0]] = 0;
        for (int i = 1; i < n; ++i) {
            std::pair<int,int> prev = {c[p[i-1]], c[(p[i-1] + len) % n]};
            std::pair<int,int> now  = {c[p[i]],   c[(p[i]   + len) % n]};
            cn[p[i]] = cn[p[i-1]] + (now != prev);
        }
        c.swap(cn);
        ++k;
    }

    return p;
}

// Kasai
std::vector<int> build_lcp(const std::string &s, const std::vector<int> &sa) {
    int n = (int)sa.size();
    std::vector<int> rank(n);
    for (int i = 0; i < n; ++i) rank[sa[i]] = i;
    std::vector<int> lcp(n, 0);
    int h = 0;
    for (int i = 0; i < n; ++i) {
        int r = rank[i];
        if (r == 0) { h = 0; lcp[r] = 0; continue; }
        int j = sa[r - 1];
        while (i + h < n && j + h < n && s[i + h] == s[j + h]) ++h;
        lcp[r] = h;
        if (h > 0) --h;
    }
    return lcp;
}

std::vector<Token> lz77_compress_sa(const std::string &s) {
    std::vector<Token> res;
    int n = (int)s.size();
    if (n == 0) return res;

    std::vector<int> sa = build_sa(s);
    std::vector<int> lcp = build_lcp(s, sa);
    std::vector<int> rank(n);
    for (int i = 0; i < n; ++i) rank[sa[i]] = i;

    int pos = 0;
    while (pos < n) {
        int r = rank[pos];
        int best_len = 0;
        int best_start = 0;

        // go left from r-1 down to 0
        int minL = std::numeric_limits<int>::max();
        for (int i = r - 1; i >= 0; --i) {
            int idx = i + 1; // lcp[i+1] is LCP(sa[i], sa[i+1])
            minL = std::min(minL, lcp[idx]);
            if (minL <= best_len) break; // can't improve going further left
            int j = sa[i];
            if (j < pos) {
                if (minL > best_len || (minL == best_len && (pos - j) < (pos - best_start))) {
                    best_len = minL;
                    best_start = j;
                }
            }
        }

        // go right from r+1 to n-1
        minL = std::numeric_limits<int>::max();
        for (int i = r + 1; i < n; ++i) {
            int idx = i; // lcp[i] is LCP(sa[i], sa[i-1])
            minL = std::min(minL, lcp[idx]);
            if (minL <= best_len) break; // can't improve going further right
            int j = sa[i];
            if (j < pos) {
                if (minL > best_len || (minL == best_len && (pos - j) < (pos - best_start))) {
                    best_len = minL;
                    best_start = j;
                }
            }
        }

        if (best_len == 0) {
            // literal
            res.push_back({0, 0, s[pos], true});
            pos += 1;
        } else {
            int offset = pos - best_start;
            if (pos + best_len < n) {
                res.push_back({offset, best_len, s[pos + best_len], true});
                pos += best_len + 1;
            } else {
                res.push_back({offset, best_len, '\0', false});
                pos += best_len;
            }
        }
    }

    return res;
}

std::string lz77_decompress(const std::vector<Token> &tokens) {
    std::string out;
    for (const Token &t : tokens) {
        if (t.offset == 0 && t.length == 0) {
            if (t.has_char) out.push_back(t.ch);
        } else {
            int start = (int)out.size() - t.offset;
            if (start < 0) throw std::runtime_error("Invalid token offset");
            for (int k = 0; k < t.length; ++k) out.push_back(out[start + k]);
            if (t.has_char) out.push_back(t.ch);
        }
    }
    return out;
}

void trim(std::string &s) {
    size_t a = 0; while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    size_t b = s.size(); while (b > a && std::isspace(static_cast<unsigned char>(s[b-1]))) --b;
    s = s.substr(a, b-a);
}

std::vector<Token> parse_tokens_from_stdin_lines() {
    std::vector<Token> tokens;
    std::string line;
    while (std::getline(std::cin, line)) {
        trim(line);
        if (line.empty()) continue;
        std::stringstream ss(line);
        int off, len;
        if (!(ss >> off >> len)) continue;
        std::string rest;
        if (ss >> rest) tokens.push_back({off, len, rest[0], true});
        else tokens.push_back({off, len, '\0', false});
    }
    return tokens;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string cmd;
    if (!std::getline(std::cin, cmd)) return 0;
    trim(cmd);

    if (cmd == "compress") {
        std::string text;
        if (!std::getline(std::cin, text)) text = "";
        trim(text);
        auto tokens = lz77_compress_sa(text);
        for (const Token &t : tokens) {
            if (t.has_char) std::cout << t.offset << ' ' << t.length << ' ' << t.ch << '\n';
            else std::cout << t.offset << ' ' << t.length << '\n';
        }
    } else if (cmd == "decompress") {
        auto tokens = parse_tokens_from_stdin_lines();
        std::cout << lz77_decompress(tokens) << '\n';
    }

    return 0;
}
