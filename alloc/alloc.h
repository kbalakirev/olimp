#pragma once

#include <types/types.h>

#include <cstdlib>
#include <cassert>
#include <utility>
#include <new>

namespace NAlloc {

struct TBaseAlloc {
    void* Alloc(ui64 c) {
        return std::malloc(c);
    }
    void DeAlloc(void* p) {
        std::free(p);
    }
    void Destroy() {
    }
};

template <ui32 Size, class TAlloc = TBaseAlloc>
class TFixedAlloc {
public:
    TFixedAlloc() = default;
    TFixedAlloc(const TFixedAlloc&) = delete;
    TFixedAlloc& operator=(const TFixedAlloc&) = delete;

    TFixedAlloc(TFixedAlloc&& other) noexcept {
        Swap(other);
    }
    TFixedAlloc& operator=(TFixedAlloc&& other) noexcept {
        Swap(other);
        return *this;
    }

    void Initialize(ui32 c) {
        assert(!Chuck_);
        Chuck_ = TAlloc().Alloc(c * Block);
        Free_ = c;
        Max_ = c;
    }

    void* Alloc(ui64 = 0) {
        if (Free_ == 0) {
            return TAlloc().Alloc(Size);
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

    void DeAlloc(void* ptr) {
        if (!ptr) {
            return;
        }
        if (ptr < Chuck_ || ptr >= Ptr(Max_)) {
            TAlloc().DeAlloc(ptr);
            return;
        }
        assert(((char*) ptr - (char*) Chuck_) % Block == 0);
        *(ui32*) ptr = Head_;
        Head_ = Idx(ptr);
        ++Free_;
    }

    void Destroy() noexcept {
        TAlloc().DeAlloc(Chuck_);
        Chuck_ = nullptr;
    }

    void Swap(TFixedAlloc& other) noexcept {
        std::swap(Chuck_, other.Chuck_);
        std::swap(Head_, other.Head_);
        std::swap(Free_, other.Free_);
        std::swap(Inited_, other.Inited_);
        std::swap(Max_, other.Max_);
    }

    ~TFixedAlloc() {
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

template <class TObj, class TAlloc = TBaseAlloc>
class TObjAlloc {
public:
    void Initialize(ui32 c) {
        Alloc_.Initialize(c);
    }

    template <class... TArgs>
    TObj* Alloc(TArgs... args) {
        void* obj = Alloc_.Alloc();
        return new (obj) TObj(std::forward<TArgs>(args)...);
    }

    void DeAlloc(void* ptr) {
        Alloc_.DeAlloc(ptr);
    }

    void Destroy() {
        Alloc_.Destroy();
    }

private:
    TFixedAlloc<sizeof(TObj), TAlloc> Alloc_;
};

}
