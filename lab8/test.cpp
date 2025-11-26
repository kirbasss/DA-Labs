#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>

struct Kuhn {
    int N;
    std::vector<std::vector<int>> adj;
    std::vector<int> matchR;
    std::vector<char> used;

    void init(int n) {
        N = n;
        adj.assign(N + 1, {});
    }

    bool dfs(int v) {
        if (used[v]) return false;
        used[v] = 1;
        for (int u : adj[v]) {
            if (matchR[u] == -1 || dfs(matchR[u])) {
                matchR[u] = v;
                return true;
            }
        }
        return false;
    }

    int run(const std::vector<int>& lefts) {
        matchR.assign(N + 1, -1);
        used.assign(N + 1, 0);
        int matched = 0;
        for (int v : lefts) {
            std::fill(used.begin(), used.end(), 0);
            if (dfs(v)) ++matched;
        }
        return matched;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    // Размеры левой доли
    std::vector<int> sizes = {50, 100, 200, 400, 500, 1000};

    // Заголовок CSV
    std::cout << "nL,V,E,matching,time_ms,time_per_nL_E_ms\n";

    for (int nL : sizes) {
        int nR = nL;
        int V = nL + nR;
        long long E = 1LL * nL * nR;

        Kuhn solver;
        solver.init(V);

        for (int i = 1; i <= nL; ++i)
            for (int j = 1; j <= nR; ++j)
                solver.adj[i].push_back(nL + j);

        std::vector<int> lefts;
        for (int i = 1; i <= nL; ++i) lefts.push_back(i);

        auto t1 = std::chrono::high_resolution_clock::now();
        int matched = solver.run(lefts);
        auto t2 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

        // нормализация по nL * E
        double time_per = ms / ((double)nL * (double)E);

        std::cout << nL << "," << V << "," << E << "," << matched << ","
                  << std::fixed << std::setprecision(3) << ms << ","
                  << std::scientific << std::setprecision(6) << time_per << "\n";
    }

    return 0;
}
