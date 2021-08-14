#include "sparse_table.h"

#include <iostream>

uint32_t GCD(int64_t a, int64_t b) {
    if (a == 0) {
        return b;
    }

    return GCD(b % a, a);
}

int main() {
    struct TPolicy {
        int64_t operator()(int64_t lhs, int64_t rhs) {
            return GCD(lhs, rhs);
        }
    };

    NSparseTable::TSparseTable<int64_t, TPolicy> table;
    auto array = {10, 12, 16, 17, 21, 3, 9, 15};
    table.Reset(array, [](auto a) {
        return a;
    });

    std::cout << table(0, 2) << "\n";
    std::cout << table(2, 4) << "\n";
    std::cout << table(1, 3) << "\n";
    std::cout << table(4, 8) << "\n";

    return 0;
}