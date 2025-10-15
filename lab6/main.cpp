#include <iostream>
#include <vector>
#include <string>


int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    if (!(std::cin >> n)) return 0;

    std::vector<long long> dp(n + 1);
    std::vector<unsigned char> op(n + 1, 0); // 0 = -1, 1 = /2, 2 = /3

    dp[1] = 0;
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

    std::cout << dp[n] << '\n';

    std::string result;
    result.reserve(n * 3);
    int cur = n;

    while (cur > 1) {
        if (op[cur] == 0) {
            result += "-1 ";
            cur -= 1;
        } else if (op[cur] == 1) {
            result += "/2 ";
            cur /= 2;
        } else {
            result += "/3 ";
            cur /= 3;
        }
    }
    std::cout << result << '\n';
    return 0;
}
