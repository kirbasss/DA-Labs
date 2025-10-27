#include <iostream>
#include <vector>
#include <algorithm>

struct Seg {
    int L, R, idx;
};

int main() {
    int N;
    std::cin >> N;
    std::vector<Seg> segs(N);
    for (int i = 0; i < N; ++i) {
        std::cin >> segs[i].L >> segs[i].R;
        segs[i].idx = i;
    }

    int M;
    std::cin >> M;

    std::sort(segs.begin(), segs.end(), [](auto& a, auto& b) {
        return a.L < b.L;
    });

    std::vector<Seg> res;
    int cur = 0;
    int i = 0;
    int bestR = -1; // Будем жадно брать тот отрезок, который позже всех кончается
    int bestIdx = -1;

    while (cur < M) {
        bool found = false;
        while (i < N && segs[i].L <= cur) {
            if (segs[i].R > bestR) {
                bestR = segs[i].R;
                bestIdx = i;
                found = true;
            }
            ++i;
        }

        if (!found) {
            std::cout << 0 << "\n";
            return 0;
        }

        res.push_back(segs[bestIdx]);
        cur = bestR;
    }

    std::sort(res.begin(), res.end(), [](auto& a, auto& b) {
        return a.idx < b.idx;
    });

    std::cout << res.size() << "\n";
    for (auto& s : res)
        std::cout << s.L << " " << s.R << "\n";
}
