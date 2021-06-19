#ifndef APPFW_SPAN_H
#define APPFW_SPAN_H
#include <appfw/dbg.h>

namespace appfw {

/**
 * An object that references a sequence of N objects in memory.
 * Really basic non-standart implementation of std::span from C++20.
 */
template <typename T>
class span {
public:
    span() = default;

    inline span(T *ptr, size_t size) noexcept {
        m_Ptr = ptr;
        m_Size = size;
    }

    inline span(std::vector<T> &cont) noexcept : span(cont.data(), cont.size()) {}

    template <size_t N>
    inline span(std::array<T, N> &cont) noexcept : span(cont.data(), N) {}

    inline T *begin() const noexcept { return m_Ptr; }
    inline T *end() const noexcept { return m_Ptr + m_Size; }

    inline T &front() const noexcept { return *m_Ptr; }
    inline T &back() const noexcept { return *(m_Ptr + m_Size - 1); }

    inline T &operator[](size_t idx) const noexcept {
        AFW_ASSERT_MSG(idx >= 0 && idx < m_Size, "span index out of range");
        return *(m_Ptr + idx);
    }

    inline T *data() const noexcept { return m_Ptr; }

    inline size_t size() const noexcept { return m_Size; }
    inline size_t size_bytes() const noexcept { return m_Size * sizeof(T); }

    inline bool empty() const noexcept { return m_Size == 0; }

    inline span<T> first(size_t n) const noexcept {
        AFW_ASSERT_MSG(n <= m_Size, "subspan size out of range");
        return span(m_Ptr, n);
    }

    inline span<T> last(size_t n) const noexcept {
        AFW_ASSERT_MSG(n <= m_Size, "subspan size out of range");
        return span(m_Ptr + m_Size - n, n);
    }

    inline span<T> subspan(size_t offset, size_t count) const noexcept {
        AFW_ASSERT_MSG(count + offset <= m_Size, "subspan out of range");
        return span(m_Ptr + offset, count);
    }

private:
    T *m_Ptr = nullptr;
    size_t m_Size = 0;
};

} // namespace appfw

#endif
