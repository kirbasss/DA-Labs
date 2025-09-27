// Компилировать: g++ -std=c++17 test.cpp -O2 -march=native -o test

#include <bits/stdc++.h>
#include <sys/resource.h>
#include <sys/time.h>
using namespace std;

// --- Упрощённая, но совместимая с отчётом реализация суффиксного дерева ---

struct Node {
    unordered_map<char,int> next;
    int start;
    int *end;
    int link;
    int example_s1;
    int example_s2;
    Node(int s=-1,int *e=nullptr): start(s), end(e), link(-1), example_s1(-1), example_s2(-1) {}
};

struct SuffixTree {
    string text;
    vector<Node> nodes;
    int root;
    int active_node, active_edge, active_length;
    int remaining;
    int *leaf_end;
    int last_new_node;
    vector<int*> allocated_ends;

    SuffixTree(): root(-1), active_node(0), active_edge(0), active_length(0),
                  remaining(0), leaf_end(nullptr), last_new_node(-1) {}

    inline int edgeLen(int idx) const { return *(nodes[idx].end) - nodes[idx].start + 1; }

    int newNode(int start,int *endPtr){
        nodes.emplace_back(start,endPtr);
        nodes.back().link = -1;
        // регистрируем endPtr, если это не общий leaf_end (чтобы не дублировать)
        if (endPtr && endPtr != leaf_end) allocated_ends.push_back(endPtr);
        return (int)nodes.size() - 1;
    }

    bool walkDown(int next){
        int el = edgeLen(next);
        if (active_length >= el){
            active_edge += el;
            active_length -= el;
            active_node = next;
            return true;
        }
        return false;
    }

    void extend(int pos){
        *leaf_end = pos;
        remaining++;
        last_new_node = -1;
        while (remaining > 0){
            if (active_length == 0) active_edge = pos;
            char a = text[active_edge];
            auto it = nodes[active_node].next.find(a);
            if (it == nodes[active_node].next.end()){
                int leaf = newNode(pos, leaf_end);
                nodes[active_node].next[a] = leaf;
                if (last_new_node != -1){
                    nodes[last_new_node].link = active_node;
                    last_new_node = -1;
                }
            } else {
                int next = it->second;
                if (walkDown(next)) continue;
                char cur = text[nodes[next].start + active_length];
                if (cur == text[pos]) {
                    active_length++;
                    if (last_new_node != -1){
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
            if (active_node == root && active_length > 0){
                active_length--;
                active_edge = pos - remaining + 1;
            } else if (nodes[active_node].link != -1) {
                active_node = nodes[active_node].link;
            } else {
                active_node = root;
            }
        }
    }

    void build(const string &s){
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

    void setSuffixIndicesAndExamples(int v,int labelHeight,int pos_dollar,int pos_hash){
        if (nodes[v].next.empty()){
            int suffixIndex = (int)text.size() - labelHeight;
            if (suffixIndex >= 0 && suffixIndex < pos_dollar) nodes[v].example_s1 = suffixIndex;
            else if (suffixIndex > pos_dollar && suffixIndex < pos_hash) nodes[v].example_s2 = suffixIndex;
            return;
        }
        for (auto &kv : nodes[v].next){
            int to = kv.second;
            setSuffixIndicesAndExamples(to, labelHeight + edgeLen(to), pos_dollar, pos_hash);
            if (nodes[v].example_s1 == -1 && nodes[to].example_s1 != -1) nodes[v].example_s1 = nodes[to].example_s1;
            if (nodes[v].example_s2 == -1 && nodes[to].example_s2 != -1) nodes[v].example_s2 = nodes[to].example_s2;
        }
    }

    void findMaxLen(int v,int curDepth,int &max_len){
        if (nodes[v].example_s1 != -1 && nodes[v].example_s2 != -1) if (curDepth > max_len) max_len = curDepth;
        for (auto &kv : nodes[v].next){
            int to = kv.second;
            findMaxLen(to, curDepth + edgeLen(to), max_len);
        }
    }

    void collectStrings(int v,int curDepth,int max_len,set<string> &res){
        if (nodes[v].example_s1 != -1 && nodes[v].example_s2 != -1 && curDepth == max_len){
            if (max_len > 0){
                int start_pos = nodes[v].example_s1;
                if (start_pos >= 0 && start_pos + max_len <= (int)text.size()){
                    string cand = text.substr(start_pos, max_len);
                    if (cand.find('$') == string::npos && cand.find('#') == string::npos) res.insert(cand);
                }
            }
        }
        for (auto &kv : nodes[v].next){
            int to = kv.second;
            collectStrings(to, curDepth + edgeLen(to), max_len, res);
        }
    }

    pair<int,vector<string>> findLCS(int pos_dollar,int pos_hash){
        setSuffixIndicesAndExamples(root, 0, pos_dollar, pos_hash);
        int max_len = 0;
        findMaxLen(root, 0, max_len);
        set<string> res;
        if (max_len > 0) collectStrings(root, 0, max_len, res);
        vector<string> out(res.begin(), res.end());
        return {max_len, out};
    }

    ~SuffixTree(){
        if (leaf_end) delete leaf_end;
        for (int *p : allocated_ends) delete p;
        allocated_ends.clear();
    }
};

// Получение пикового RSS (в килобайтах) — ru_maxrss (замечание: поведение платформозависимо)
static long get_peak_rss_kb(){
    struct rusage r;
    getrusage(RUSAGE_SELF, &r);
    return r.ru_maxrss; // обычно в KB на Linux
}

static string gen_random_string(size_t len){
    string s; s.resize(len);
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()_+-=[]{};:,./<>?";
    static const size_t chlen = sizeof(charset) - 1;
    random_device rd;
    mt19937_64 gen(rd());
    uniform_int_distribution<size_t> dist(0, chlen-1);
    for (size_t i = 0; i < len; ++i) s[i] = charset[dist(gen)];
    return s;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<size_t> sizes = {1<<20, 2<<20, 4<<20, 6<<20}; // 1MB,2MB,4MB,6MB
    cout << "size_bytes,build_sec,find_sec,peak_rss_kb\n";
    for (size_t sz : sizes){
        string s1 = gen_random_string(sz);
        string s2 = gen_random_string(sz);
        string text = s1 + "$" + s2 + "#";

        SuffixTree st;
        auto t0 = chrono::high_resolution_clock::now();
        st.build(text);
        auto t1 = chrono::high_resolution_clock::now();
        auto res = st.findLCS((int)s1.size(), (int)s1.size() + 1 + (int)s2.size());
        auto t2 = chrono::high_resolution_clock::now();

        double build_sec = chrono::duration<double>(t1 - t0).count();
        double find_sec  = chrono::duration<double>(t2 - t1).count();
        long peak_kb = get_peak_rss_kb();

        cout << sz << "," << build_sec << "," << find_sec << "," << peak_kb << "\n";

        // освобождение дерева (переходит в деструктор)
    }
    return 0;
}
