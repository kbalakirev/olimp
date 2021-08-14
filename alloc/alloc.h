#pragma once

#include <types/types.h>

#include <cstdlib>
#include <cassert>
#include <utility>
#include <new>

namespace NAlloc {

template <ui32 Size>
class TFixedAllocator {
public:
    TFixedAllocator() = default;

    TFixedAllocator(TFixedAllocator&& other) noexcept {
        Swap(other);
    }
    TFixedAllocator& operator=(TFixedAllocator&& other) noexcept {
        Swap(other);
        return *this;
    }

    void Initialize(ui32 c) {
        assert(!Chuck_);
        Chuck_ = std::malloc(c * Block);
        Free_ = c;
        Max_ = c;
    }

    void* Allocate(ui64 size) {
        if (Free_ == 0 || size != Size) {
            return std::malloc(size);
        }
        if (Inited_ < Max_) {
            *(ui32*) Ptr(Inited_) = Inited_ + 1;
            ++Inited_;
        }
        void* block = Ptr(Head_);
        Head_ = *(ui32*) block;
        --Free_;
        return block;
    }

    template <class T, class... TArgs>
    T* Construct(TArgs&&... args) {
        void* p = Allocate(sizeof(T));
        try {
            return new(p) T(std::forward<TArgs>(args)...);
        } catch (...) {
            Deallocate(p);
            throw;
        }
    }

    void Deallocate(void* p) {
        if (!p) {
            return;
        }
        if (p < Chuck_ || p >= Ptr(Max_)) {
            std::free(p);
            return;
        }
        assert(((char*) p - (char*) Chuck_) % Block == 0);
        *(ui32*) p = Head_;
        Head_ = Idx(p);
        ++Free_;
    }

    void Destroy() noexcept {
        std::free(Chuck_);
        Chuck_ = nullptr;
    }

    void Swap(TFixedAllocator& other) noexcept {
        std::swap(Chuck_, other.Chuck_);
        std::swap(Head_, other.Head_);
        std::swap(Free_, other.Free_);
        std::swap(Inited_, other.Inited_);
        std::swap(Max_, other.Max_);
    }

    ~TFixedAllocator() {
        Destroy();
    }

private:
    static constexpr ui32 ALIGN = sizeof(void*);
    static constexpr ui32 Align(ui32 c) {
        if (c % ALIGN) {
            return c - (c % ALIGN) + ALIGN;
        }
        return c;
    }
    static constexpr ui64 Block = Align(Size);
    ui32 Idx(void* p) const {
        return ((char*) p - (char*) Chuck_) / Block;
    }
    void* Ptr(ui32 i) {
        return (char*) Chuck_ + (Block * i);
    }

private:
    void* Chuck_ = nullptr;
    ui32 Head_ = 0;
    ui32 Free_ = 0;
    ui32 Inited_ = 0;
    ui32 Max_ = 0;
};

}
