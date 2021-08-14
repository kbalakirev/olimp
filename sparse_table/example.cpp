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
        static int64_t Init(int64_t value) {
            return value;
        }

        static int64_t Merge(int64_t lhs, int64_t rhs) {
            return GCD(lhs, rhs);
        }
    };

    NSparseTable::TSparseTable<int64_t, TPolicy> table;
    auto array = {10, 12, 16, 17, 21, 3, 9, 15};
    table.Reset(array);

    std::cout << table.Query(0, 2) << "\n";
    std::cout << table.Query(2, 4) << "\n";
    std::cout << table.Query(1, 3) << "\n";
    std::cout << table.Query(4, 8) << "\n";

    return 0;
}