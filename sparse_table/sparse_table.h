#include <vector>
#include <cassert>
#include <cstdint>

#ifdef __GNUC__
#define BIN_ROUND(x) (8u * sizeof(unsigned long long) -  __builtin_clzll(x) - 1u)
#endif

namespace NSparseTable {

template <class T, class P>
class TSparseTable {
public:
    template <class C, class I>
    void Reset(const C& c, I initer) {
        m_t.resize(c.size());
        size_t n = BIN_ROUND(c.size()) + 1;

        size_t k = 0;
        for (const auto& v: c) {
            m_t[k].resize(n);
            m_t[k++][0] = initer(v);
        }

        for (size_t i = 1; i < n; ++i) {
            size_t m = (1u << i);
            for (size_t j = 0; j + m <= c.size(); ++j) {
                m_t[j][i] = P()(m_t[j][i - 1], m_t[j + m / 2][i - 1]);
            }
        }
    }

    T operator()(uint32_t begin, uint32_t end) const {
        assert(end > begin && end <= m_t.size());
        uint32_t n = BIN_ROUND(end - begin);
        return P()(m_t[begin][n], m_t[end - (1u << n)][n]);
    }

private:
    std::vector<std::vector<T>> m_t;
};
}

#undef BIN_ROUND
