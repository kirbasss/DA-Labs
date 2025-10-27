#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <random>
#include <sstream>
#include <chrono>


int rank_matrix(std::vector<std::vector<long double>> a) {
    int m = static_cast<int>(a.size());
    int n = (m ? static_cast<int>(a[0].size()) : 0);
    int r = 0;
    const long double EPS = 1e-12L;

    for (int col = 0; col < n && r < m; ++col) {
        int sel = -1;
        long double best = 0;
        for (int i = r; i < m; ++i) {
            long double val = std::fabs(a[i][col]);
            if (val > best + EPS) {
                best = val;
                sel = i;
            }
        }
        if (sel == -1) continue;
        std::swap(a[sel], a[r]);
        long double piv = a[r][col];
        for (int j = col; j < n; ++j)
            a[r][j] /= piv;
        for (int i = 0; i < m; ++i) {
            if (i == r) continue;
            long double factor = a[i][col];
            if (std::fabs(factor) <= EPS) continue;
            for (int j = col; j < n; ++j)
                a[i][j] -= factor * a[r][j];
        }
        ++r;
    }
    return r;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::vector<std::pair<int, int>> tests = {
        {50, 10},
        {100, 20},
        {200, 30},
        {400, 40},
        {800, 50},
        {800, 80},
        {800, 120}
    };

    for (auto test_case : tests) {
        int M = test_case.first;
        int N = test_case.second;

        std::vector<std::vector<int>> rows(M, std::vector<int>(N));
        std::vector<int> prices(M);
        std::mt19937_64 rng(42);
        std::uniform_int_distribution<int> dist(-10, 10);
        std::uniform_int_distribution<int> price_dist(1, 1000);

        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < N; ++j)
                rows[i][j] = dist(rng);
            prices[i] = price_dist(rng);
        }

        auto start = std::chrono::high_resolution_clock::now();

        struct Item {
            std::vector<int> row;
            int price;
            int idx;
        };

        std::vector<Item> items(M);
        for (int i = 0; i < M; ++i) {
            items[i].row = rows[i];
            items[i].price = prices[i];
            items[i].idx = i + 1;
        }

        if (M < N) {
            std::cout << "M=" << M << " N=" << N << " -> -1 (M<N)\n";
            continue;
        }

        std::vector<int> ord(M);
        std::iota(ord.begin(), ord.end(), 0);
        std::sort(ord.begin(), ord.end(), [&](int a, int b) {
            if (items[a].price != items[b].price)
                return items[a].price < items[b].price;
            return items[a].idx < items[b].idx;
        });

        std::vector<std::vector<long double>> chosen_rows;
        std::vector<int> chosen_idx;

        for (int id : ord) {
            chosen_rows.push_back(std::vector<long double>(N));
            for (int j = 0; j < N; ++j)
                chosen_rows.back()[j] = static_cast<long double>(items[id].row[j]);

            int r = rank_matrix(chosen_rows);
            if (r > static_cast<int>(chosen_idx.size())) {
                chosen_idx.push_back(items[id].idx);
                if (r == N) break;
            } else {
                chosen_rows.pop_back();
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if (static_cast<int>(chosen_idx.size()) < N) {
            std::cout << "M=" << M << " N=" << N << " -> -1 | " << ms << " ms\n";
        } else {
            std::cout << "M=" << M << " N=" << N << " -> ";
            std::sort(chosen_idx.begin(), chosen_idx.end());
            for (std::size_t i = 0; i < chosen_idx.size(); ++i) {
                if (i > 0) std::cout << ' ';
                std::cout << chosen_idx[i];
            }
            std::cout << " | " << ms << " ms\n";
        }
    }

    return 0;
}
