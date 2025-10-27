#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <string>
#include <iomanip>
#include <fstream>

struct Seg {
    int L, R, idx;
};

std::vector<Seg> greedy_cover(std::vector<Seg> segs, int M) {
    std::sort(segs.begin(), segs.end(), [](const Seg& a, const Seg& b){
        if (a.L != b.L) return a.L < b.L;
        return a.R > b.R;
    });
    std::vector<Seg> res;
    int cur = 0;
    std::size_t i = 0;
    int n = static_cast<int>(segs.size());

    while (cur < M) {
        int bestR = cur;
        int bestIdx = -1;
        while (i < segs.size() && segs[i].L <= cur) {
            if (segs[i].R > bestR) {
                bestR = segs[i].R;
                bestIdx = static_cast<int>(i);
            }
            ++i;
        }
        if (bestIdx == -1) {
            return std::vector<Seg>();
        }
        res.push_back(segs[bestIdx]);
        cur = bestR;
    }
    return res;
}

enum class TestKind { RANDOM, OVERLAPPING, ADVERSARIAL };
std::vector<Seg> generate_test(int N, int M, TestKind kind, std::mt19937 &rng) {
    std::vector<Seg> segs;
    segs.reserve(N);

    std::uniform_int_distribution<int> left_d(-M/2, M);
    std::uniform_int_distribution<int> len_d(0, M/2);
    std::uniform_int_distribution<int> small_len(1, std::max(1, M/10));

    if (kind == TestKind::RANDOM) {
        for (int i = 0; i < N; ++i) {
            int L = left_d(rng);
            int len = len_d(rng);
            int R = L + len;
            if (R < L) R = L;
            segs.push_back({L, R, i});
        }
    } else if (kind == TestKind::OVERLAPPING) {
        std::uniform_int_distribution<int> center_d(-M/4, M/2);
        for (int i = 0; i < N; ++i) {
            int center = center_d(rng);
            int len = std::uniform_int_distribution<int>(1, std::max(1, M))(rng);
            int L = center;
            int R = center + len;
            segs.push_back({L, R, i});
        }
    } else {
        for (int i = 0; i < N; ++i) {
            int L = std::max(0, i / 2 - 3);
            int R = L + std::min(M, 1 + (i % std::max(1, M)));
            // scatter a bit
            if (R > M + M/2) R = M + M/2;
            segs.push_back({L, R, i});
        }
        // shuffle a bit
        std::shuffle(segs.begin(), segs.end(), rng);
    }

    return segs;
}

int main() {
    std::vector<int> Ns = {1000, 5000, 10000, 20000, 50000}; // размеры N
    int trials = 5;
    int M_factor = 1; // M ~ M_factor * N
    unsigned seed = 123456789u;
    std::mt19937 rng(seed);

    std::cout << "test_kind,N,M,trial,gen_time_ms,alg_time_ms,selected_count\n";

    for (auto kind : {TestKind::RANDOM, TestKind::OVERLAPPING, TestKind::ADVERSARIAL}) {
        std::string kind_name = (kind == TestKind::RANDOM ? "RANDOM" : (kind == TestKind::OVERLAPPING ? "OVERLAPPING" : "ADVERSARIAL"));
        for (int N : Ns) {
            int M = M_factor * N;
            for (int t = 0; t < trials; ++t) {
                // measure generation time
                auto gstart = std::chrono::high_resolution_clock::now();
                auto segs = generate_test(N, M, kind, rng);
                auto gend = std::chrono::high_resolution_clock::now();
                double gen_ms = std::chrono::duration<double, std::milli>(gend - gstart).count();

                // measure algorithm time
                auto astart = std::chrono::high_resolution_clock::now();
                auto result = greedy_cover(segs, M);
                auto aend = std::chrono::high_resolution_clock::now();
                double alg_ms = std::chrono::duration<double, std::milli>(aend - astart).count();

                int sel = static_cast<int>(result.size());

                std::cout << kind_name << "," << N << "," << M << "," << (t+1) << ",";
                std::cout << std::fixed << std::setprecision(3) << gen_ms << "," << alg_ms << "," << sel << "\n";
            }
        }
    }

    return 0;
}
