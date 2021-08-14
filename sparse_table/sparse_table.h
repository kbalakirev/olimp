#include <vector>
#include <cassert>
#include <cstdint>

#ifdef __GNUC__
#define BIN_ROUND __builtin_ctzll
#endif

namespace NSparseTable {
template <class T, class P>
class TSparseTable {
public:
    template <class TContainer>
    void Reset(const TContainer& c) {
        m_t.resize(c.size());

        size_t n = BIN_ROUND(c.size()) + 1;

        size_t k = 0;
        for (const auto& value: c) {
            m_t[k].resize(n);
            m_t[k++][0] = P::Init(value);
        }

        for (size_t i = 1; i < n; ++i) {
            size_t m = (1u << i);
            for (size_t j = 0; j + m <= c.size(); ++j) {
                m_t[j][i] = P::Merge(m_t[j][i - 1], m_t[j + m / 2][i - 1]);
            }
        }
    }

    T Query(uint32_t begin, uint32_t end) {
        assert(end > begin);
        uint32_t n = BIN_ROUND(end - begin);
        return P::Merge(m_t[begin][n], m_t[end - (1u << n)][n]);
    }

private:
    std::vector<std::vector<T>> m_t;
};
}

#undef BIN_ROUND
