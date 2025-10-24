#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

int rank_matrix(std::vector<std::vector<long double>> a) {
    // a: matrix with rows = equations, cols = N
    int m = static_cast<int>(a.size());
    int n = (m ? static_cast<int>(a[0].size()) : 0);
    int r = 0;
    const long double EPS = 1e-12L;

    for (int col = 0; col < n && r < m; ++col) {
        // find pivot in [r..m-1]
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
        // normalize row r
        for (int j = col; j < n; ++j)
            a[r][j] /= piv;
        // eliminate
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

    int M, N;
    if (!(std::cin >> M >> N)) return 0;

    struct Item {
        std::vector<int> row;
        int price;
        int idx;
    };

    std::vector<Item> items(M);
    for (int i = 0; i < M; ++i) {
        items[i].row.resize(N);
        for (int j = 0; j < N; ++j)
            std::cin >> items[i].row[j];
        std::cin >> items[i].price;
        items[i].idx = i + 1; // 1-based index
    }

    if (M < N) {
        std::cout << -1 << "\n";
        return 0;
    }

    // sort by price ascending
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
        // try to add items[id]
        chosen_rows.push_back(std::vector<long double>(N));
        for (int j = 0; j < N; ++j)
            chosen_rows.back()[j] = static_cast<long double>(items[id].row[j]);

        int r = rank_matrix(chosen_rows);
        if (r > static_cast<int>(chosen_idx.size())) { // rank increased -> keep it
            chosen_idx.push_back(items[id].idx);
            if (r == N) break; // got full rank
        } else {
            // didn't increase rank -> remove last appended
            chosen_rows.pop_back();
        }
    }

    if (static_cast<int>(chosen_idx.size()) < N) {
        std::cout << -1 << "\n";
    } else {
        std::sort(chosen_idx.begin(), chosen_idx.end());
        for (size_t i = 0; i < chosen_idx.size(); ++i) {
            if (i > 0) std::cout << ' ';
            std::cout << chosen_idx[i];
        }
        std::cout << "\n";
    }

    return 0;
}
