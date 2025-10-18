#include <iostream>
#include <vector>
#include <string>
#include <chrono>

int main() {
    std::vector<int> tests = {100000, 200000, 500000, 1000000, 2000000, 5000000, 10000000};

    std::cout << "n,time_ms,ops_len,dp_n" << '\n';

    for (int n : tests) {
        try {
            std::vector<long long> dp(n + 1);
            std::vector<unsigned char> op(n + 1, 0);
            dp[1] = 0;

            auto t0 = std::chrono::high_resolution_clock::now();

            for (int x = 2; x <= n; ++x) {
                long long bestVal = dp[x - 1];
                unsigned char bestOp = 0;
                if (x % 2 == 0 && dp[x / 2] < bestVal) {
                    bestVal = dp[x / 2];
                    bestOp = 1;
                }
                if (x % 3 == 0 && dp[x / 3] < bestVal) {
                    bestVal = dp[x / 3];
                    bestOp = 2;
                }
                dp[x] = static_cast<long long>(x) + bestVal;
                op[x] = bestOp;
            }

            auto t1 = std::chrono::high_resolution_clock::now();
            long long time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

            // Для больших n не реконструируем всю последовательность (только длину)
            long long ops_len = 0;
            if (n <= 500000) {
                int cur = n;
                while (cur > 1) {
                    ++ops_len;
                    if (op[cur] == 0) cur = cur - 1;
                    else if (op[cur] == 1) cur = cur / 2;
                    else cur = cur / 3;
                }
            } else {
                // грубая оценка длины последовательности: reconstruct не выполняем
                ops_len = -1; // маркер: не вычислено
            }

            std::cout << n << ',' << time_ms << ',' << ops_len << ',' << dp[n] << '\n';

        } catch (const std::bad_alloc &e) {
            std::cerr << "Memory allocation failed for n=" << n << '\n';
        }
    }

    return 0;
}