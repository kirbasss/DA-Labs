#include <iostream>
#include <vector>
#include <algorithm>
#include <queue>


int n, m;
std::vector<std::vector<int>> adj;
std::vector<int> matchR;
std::vector<char> used;

bool tryKuhn(int v) {
    if (used[v]) return false;
    used[v] = 1;
    for (int u : adj[v]) {
        if (matchR[u] == -1 || tryKuhn(matchR[u])) {
            matchR[u] = v;
            return true;
        }
    }
    return false;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::cin >> n >> m;
    adj.assign(n + 1, {});

    for(int i = 0; i < m; ++i) {
        int a, b;
        std::cin >> a >> b;
        adj[a].push_back(b);
        adj[b].push_back(a);
    }

    for(int v = 1; v <= n; ++v)
        std::sort(adj[v].begin(), adj[v].end());
    
    // Покрасим, чтобы в дальнейшем перебирать вершины только из левой доли
    std::vector<int> color(n + 1, -1);
    for(int s = 1; s <= n; ++s) {
        if(color[s] != -1) continue;
        std::queue<int> q;
        q.push(s);
        color[s] = 0;
        while(!q.empty()) {
            int v = q.front(); q.pop();
            for(int u : adj[v]) {
                if(color[u] == -1) {
                    color[u] = color[v] ^ 1;
                    q.push(u);
                }
            }
        }
    }

    std::vector<int> lefts;
    for(int v = 1; v <= n; ++v)
        if(color[v] == 0) lefts.push_back(v);

    matchR.assign(n + 1, -1);
    used.assign(n + 1, 0);

    for(int v : lefts) {
        std::fill(used.begin(), used.end(), 0);
        tryKuhn(v);
    }

    std::vector<std::pair<int,int>> ans;
    for (int u = 1; u <= n; ++u) {
        if (matchR[u] != -1) {
            int a = matchR[u], b = u;
            if (a > b) std::swap(a, b);
            ans.emplace_back(a, b);
        }
    }

    std::sort(ans.begin(), ans.end());

    std::cout << ans.size() << "\n";
    for (auto &p : ans)
        std::cout << p.first << " " << p.second << "\n";

    return 0;
}
