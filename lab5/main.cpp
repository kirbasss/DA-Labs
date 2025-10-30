#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <memory>

struct Node {
    std::unordered_map<char,int> next; // char -> node index
    int start;
    int *end;      // inclusive
    int link;      // suffix link
    int example_s1; // example index from s1 in subtree (or -1)
    int example_s2; // example index from s2 in subtree (or -1)
    Node(int s = -1, int *e = nullptr)
        : start(s), end(e), link(-1), example_s1(-1), example_s2(-1) {}
};

class SuffixTree {
private:
    std::string text;
    std::vector<Node> nodes;
    int root;

    int active_node;
    int active_edge;
    int active_length;

    int remaining;
    int *leaf_end;
    int pos;
    int last_new_node;

    std::vector<int*> allocated_ends; // to delete later

    inline int edgeLen(int idx) const {
        return *(nodes[idx].end) - nodes[idx].start + 1;
    }

    int newNode(int start, int *endPtr) {
        nodes.emplace_back(start, endPtr);
        nodes.back().link = -1;
        // record endPtr only if it's not the shared leaf_end (to avoid duplicates)
        if (endPtr != nullptr && endPtr != leaf_end) allocated_ends.push_back(endPtr);
        return (int)nodes.size() - 1;
    }

    bool walkDown(int next) {
        int elen = edgeLen(next);
        if (active_length >= elen) {
            active_edge += elen;
            active_length -= elen;
            active_node = next;
            return true;
        }
        return false;
    }

    void extend(int idx) {
        pos = idx;
        *leaf_end = pos;
        remaining++;
        last_new_node = -1;

        while (remaining > 0) {
            if (active_length == 0) active_edge = pos;
            char a = text[active_edge];
            auto it = nodes[active_node].next.find(a);
            if (it == nodes[active_node].next.end()) {
                int leaf = newNode(pos, leaf_end);
                nodes[active_node].next[a] = leaf;
                if (last_new_node != -1) {
                    nodes[last_new_node].link = active_node;
                    last_new_node = -1;
                }
            } else {
                int next = it->second;
                if (walkDown(next)) continue;
                char cur = text[nodes[next].start + active_length];
                if (cur == text[pos]) {
                    active_length++;
                    if (last_new_node != -1) {
                        nodes[last_new_node].link = active_node;
                        last_new_node = -1;
                    }
                    break;
                }
                int *split_end = new int(nodes[next].start + active_length - 1);
                int split = newNode(nodes[next].start, split_end);
                nodes[active_node].next[a] = split;
                int leaf = newNode(pos, leaf_end);
                nodes[split].next[text[pos]] = leaf;
                nodes[split].next[cur] = next;
                nodes[next].start += active_length;
                if (last_new_node != -1) nodes[last_new_node].link = split;
                last_new_node = split;
            }

            remaining--;
            if (active_node == root && active_length > 0) {
                active_length--;
                active_edge = pos - remaining + 1;
            } else if (nodes[active_node].link != -1) {
                active_node = nodes[active_node].link;
            } else {
                active_node = root;
            }
        }
    }

    void setSuffixIndicesAndExamples(int v, int labelHeight, int pos_dollar, int pos_hash) {
        if (nodes[v].next.empty()) {
            int suffixIndex = (int)text.size() - labelHeight;
            if (suffixIndex >= 0 && suffixIndex < pos_dollar) {
                nodes[v].example_s1 = suffixIndex;
            } else if (suffixIndex > pos_dollar && suffixIndex < pos_hash) {
                nodes[v].example_s2 = suffixIndex;
            }
            return;
        }

        for (auto &kv : nodes[v].next) {
            int to = kv.second;
            setSuffixIndicesAndExamples(to, labelHeight + edgeLen(to), pos_dollar, pos_hash);
            if (nodes[v].example_s1 == -1 && nodes[to].example_s1 != -1) nodes[v].example_s1 = nodes[to].example_s1;
            if (nodes[v].example_s2 == -1 && nodes[to].example_s2 != -1) nodes[v].example_s2 = nodes[to].example_s2;
        }
    }

    void findMaxLen(int v, int curDepth, int &max_len) {
        if (nodes[v].example_s1 != -1 && nodes[v].example_s2 != -1) {
            if (curDepth > max_len) max_len = curDepth;
        }
        for (auto &kv : nodes[v].next) {
            int to = kv.second;
            findMaxLen(to, curDepth + edgeLen(to), max_len);
        }
    }

    void collectStrings(int v, int curDepth, int max_len, std::set<std::string> &res) {
        if (nodes[v].example_s1 != -1 && nodes[v].example_s2 != -1 && curDepth == max_len) {
            if (max_len > 0) {
                int start_pos = nodes[v].example_s1;
                if (start_pos >= 0 && start_pos + max_len <= (int)text.size()) {
                    std::string cand = text.substr(start_pos, max_len);
                    if (cand.find('$') == std::string::npos && cand.find('#') == std::string::npos) {
                        res.insert(cand);
                    }
                }
            }
        }
        for (auto &kv : nodes[v].next) {
            int to = kv.second;
            collectStrings(to, curDepth + edgeLen(to), max_len, res);
        }
    }

public:
    SuffixTree(): root(-1), active_node(0), active_edge(0), active_length(0),
                  remaining(0), leaf_end(nullptr), pos(-1), last_new_node(-1) {}

    void build(const std::string &s) {
        text = s;
        nodes.clear();
        allocated_ends.clear();
        leaf_end = new int(-1);
        nodes.reserve((int)text.size() * 2 + 5);
        root = newNode(-1, new int(-1));
        nodes[root].link = -1;
        active_node = root;
        active_edge = 0;
        active_length = 0;
        remaining = 0;
        last_new_node = -1;

        for (size_t i = 0; i < text.size(); ++i) extend((int)i);
    }

    std::pair<int, std::vector<std::string>> findLCSForTwoStrings(int pos_dollar, int pos_hash) {
        setSuffixIndicesAndExamples(root, 0, pos_dollar, pos_hash);
        int max_len = 0;
        findMaxLen(root, 0, max_len);
        std::set<std::string> res;
        if (max_len > 0) {
            collectStrings(root, 0, max_len, res);
        }
        std::vector<std::string> out(res.begin(), res.end());
        return {max_len, out};
    }

    ~SuffixTree() {
        if (leaf_end) delete leaf_end;
        for (int *p : allocated_ends) delete p;
        allocated_ends.clear();
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string s1, s2;
    if (!(std::cin >> s1 >> s2)) return 0;

    std::string text = s1 + "$" + s2 + "#";
    int pos_dollar = (int)s1.size();
    int pos_hash = pos_dollar + 1 + (int)s2.size();

    SuffixTree st;
    st.build(text);
    auto ans = st.findLCSForTwoStrings(pos_dollar, pos_hash);

    std::cout << ans.first << "\n";
    for (auto &str : ans.second) std::cout << str << "\n";
    return 0;
}
