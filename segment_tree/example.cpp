#include <alloc/alloc.h>
#include "segment_tree.h"

#include <iostream>
#include <sstream>

static constexpr ui64 NUM_NODE = 3;

struct TSummary: public NIndexTree::TIntrusiveItem<TSummary> {
    TSummary() = default;
    TSummary(i64 value, ui64 size = 1u)
        : Min(value)
        , Max(value)
        , Sum(value * (i64) size)
        , Value(value)
    {
    }

    i64 Min = 0;
    i64 Max = 0;
    i64 Sum = 0;
    i64 Value = 0;

    std::string AsString() const {
        std::stringstream ss;
        ss << "value: " << Value << ", min: " << Min << ", max: " << Max << ", sum: " << Sum;
        return ss.str();
    }

    struct TPolicy {
        static void Cut(TSummary* father, TSummary* child, ESide side) {
            TSummary* other = father->Left();
            if (side == ESide::LEFT) {
                other = father->Right();
            }

            father->Min = father->Value;
            father->Max = father->Value;

            if (other) {
                father->Min = std::min(father->Min, other->Min);
                father->Max = std::max(father->Max, other->Max);
            }

            father->Sum -= child->Sum;
        }

        static void Glue(TSummary* father, TSummary* child, ESide) {
            father->Sum += child->Sum;

            father->Min = std::min(father->Min, child->Min);
            father->Max = std::max(father->Max, child->Max);
        }

        static void Break(TSummary* father, TSummary* left, TSummary* right, ui64 index) {
            left->Min = right->Min = father->Min;
            left->Max = right->Max = father->Max;
            left->Value = right->Value = father->Value;

            left->Sum = (father->Sum / father->Size()) * index;
            right->Sum = (father->Sum / father->Size()) * (father->Size() - index);
        }
    };
};


using TSummaryAllocator = NAlloc::TObjAlloc<TSummary>;
using TSummaryArena = NIndexTree::TArena<TSummaryAllocator>;

using TSegmentTree = NIndexTree::TSegmentTree<TSummaryArena>;

int main() {
    TSummaryAllocator allocator;
    allocator.Initialize(NUM_NODE);

    TSummaryArena arena(std::move(allocator));

    TSegmentTree array(&arena);
    array.Reserve(10u);

    for (size_t i = 0; i < 10; ++i) {
        array.Do(i, i + 1, [i](TSummary* summary) {
           summary->Value = (i * 7) % 10;
           summary->Min = summary->Max = summary->Sum = summary->Value;
        });
    }

    array.Do(0u, 5u, [](const TSummary* summary) {
        std::cout << summary->AsString() << "\n";
    });
    array.Do(3u, 9u, [](const TSummary* summary) {
        std::cout << summary->AsString() << "\n";
    });
    array.ForEach([](const TSummary* summary) {
        std::cout << summary->Value << " ";
    });
    std::cout << "\n";

    return 0;
}
