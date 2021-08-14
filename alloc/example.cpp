#include "alloc.h"

#include <iostream>

struct TIntrusiveNode {
    TIntrusiveNode* Left = nullptr;
    TIntrusiveNode* Right = nullptr;
};

template <class T, class TComparator>
TIntrusiveNode* Add(TIntrusiveNode* node, T* value, TComparator comparator) {
    if (!node) {
        return value;
    }

    TIntrusiveNode* father = nullptr;
    TIntrusiveNode* current = node;

    bool left = false;

    while (current) {
        father = current;

        if (comparator(*static_cast<T*>(current), *value)) {
            current = current->Right;
            left = false;
        } else if (comparator(*value, *static_cast<T*>(current))) {
            current = current->Left;
            left = true;
        } else {
            return node;
        }
    }

    if (left) {
        father->Left = value;
    } else {
        father->Right = value;
    }

    return node;
}

template <class T, class TFunc>
void ForEach(TIntrusiveNode* root, TFunc func) {
    if (!root) {
        return;
    }

    ForEach<T, TFunc>(root->Left, func);
    func(*static_cast<T*>(root));
    ForEach<T, TFunc>(root->Right, func);
}

template <class T, class TDeleter>
void Free(TIntrusiveNode* root, TDeleter& deleter) {
    if (!root) {
        return;
    }
    Free<T>(static_cast<T*>(root->Left), deleter);
    Free<T>(static_cast<T*>(root->Right), deleter);
    deleter.Deallocate(root);
}

struct TKey: public TIntrusiveNode {
    explicit TKey(ui64 key)
            : Key(key)
    {
    }

    ui64 Key = 0;
};

void Example() {
    constexpr ui64 N = 10;

    NAlloc::TFixedAllocator<sizeof(TKey)> alloc;
    alloc.Initialize(N);

    TIntrusiveNode* root = nullptr;

    for (ui64 i = 0; i < N; ++i) {
        auto* node = alloc.Construct<TKey>(N - i);
        root = Add(root, node, [](const auto& lhs, const auto& rhs) {
            return lhs.Key < rhs.Key;
        });
    }

    ForEach<TKey>(root, [](const TKey& key) {
       std::cout << key.Key << "\n";
    });

    auto other = std::move(alloc);
    Free<TKey>(root, other);
}

int main() {
    Example();

    return 0;
}
