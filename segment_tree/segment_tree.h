#include <types/types.h>
#include <alloc/alloc.h>

#include <type_traits>
#include <utility>
#include <random>
#include <cassert>

namespace NIndexTree {

template <class A>
class TIndexTree;

struct TBase {
    enum class ESide {
        LEFT = 0,
        RIGHT
    };
};

template <class T>
struct TIntrusiveBase: public TBase {
    T* Left = nullptr;
    T* Right = nullptr;

    ui64 Size = 0;
    ui64 Priority = 0;
};

template <class T>
class TIntrusiveItem: private TIntrusiveBase<T> {
public:
    using TBase = TIntrusiveBase<T>;
    using typename TBase::ESide;

    T* Left() const {
        return TBase::Left;
    }

    T* Right() const {
        return TBase::Right;
    }

    ui64 Size() const {
        return TBase::Size;
    }

    TBase* Base() {
        return this;
    }
};

template <class T>
class TArena {
public:
    using TAllocator = T;
    using TObj = std::remove_pointer_t<decltype(std::declval<TAllocator>().Alloc())>;
    static_assert(std::is_base_of_v<TIntrusiveItem<TObj>, TObj>);

    explicit TArena(TAllocator&& alloc, ui32 seed = 0)
        : Allocator_(std::move(alloc))
        , Generator_(seed)
    {
    }

    template <class... TArgs>
    TObj* Construct(TArgs&&... args) {
        auto* obj = Allocator_.Alloc(std::forward<TArgs>(args)...);
        obj->Base()->Priority = Dist_(Generator_);
        return obj;
    }

    void Delete(TObj* arg) {
        Allocator_.DeAlloc(arg);
    }

private:
    TAllocator Allocator_;
    std::mt19937 Generator_;
    std::uniform_int_distribution<ui64> Dist_;
};

template <class T>
class TSegmentTree;

template <class T>
class TSegmentTree<TArena<T>> {
public:
    using TArenaImpl = TArena<T>;
    using TObj = typename TArenaImpl::TObj;

public:
    explicit TSegmentTree(TArenaImpl* arena)
        : Arena_(arena)
    {
    }
    TSegmentTree(TSegmentTree&& other) noexcept {
        Swap(other);
    }
    TSegmentTree& operator=(TSegmentTree&& other) noexcept {
        Swap(other);
        return (*this);
    }
    ~TSegmentTree() {
        Clear(Root_);
    }

public:
    void Clear() {
        Clear(Root_);
        Root_ = nullptr;
    }

    ui64 Size() const {
        return Size(Root_);
    }

public:
    template <class TOp>
    void Do(ui64 begin, ui64 end, TOp op) {
        assert(begin < end);

        auto [left, midRight] = Split(Root_, begin);
        auto [mid, right] = Split(midRight, end - begin);
        op(mid);
        Root_ = Merge(left, Merge(mid, right));
    }

    template <class TOp>
    void ForEach(TOp op) {
        ForEach(Root_, std::move(op));
    }

    TSegmentTree Extract(ui64 begin, ui64 end) {
        assert(begin < end);

        auto [left, midRight] = Split(Root_, begin);
        auto [mid, right] = Split(midRight, end - begin);

        Root_ = Merge(left, right);

        TSegmentTree other(Arena_);
        other.Root_ = mid;
        return other;
    }

    void Insert(ui64 index, TSegmentTree&& other) {
        assert(Arena_ == other.Arena_);

        auto [left, right] = Split(Root_, index);
        Root_ = Merge(Merge(left, other.Root_), right);
    }

    void Reserve(ui64 size) {
        if (Size() < size) {
            TObj* obj = Arena_->Construct();
            obj->Base()->Size = size - Size();
            Root_ = Merge(Root_, obj);
        }
    }

private:
    static TObj* Cut(TObj* father, TBase::ESide side) {
        assert(father);

        TObj* child;

        if (side == TBase::ESide::LEFT) {
            child = father->Left();
            father->Base()->Left = nullptr;
        } else {
            child = father->Right();
            father->Base()->Right = nullptr;
        }

        assert(Size(child) < father->Size());
        father->Base()->Size -= Size(child);

        if (child) {
            TObj::TPolicy::Cut(father, child, side);
        }

        return child;
    }

    static TObj* Glue(TObj* father, TObj* obj, TBase::ESide side) {
        assert(father || !obj);

        if (!obj || !father) {
            return father;
        }

        if (side == TBase::ESide::LEFT) {
            father->Base()->Left = obj;
        } else {
            father->Base()->Right = obj;
        }

        father->Base()->Size += obj->Size();

        TObj::TPolicy::Glue(father, obj, side);


        return father;
    }

    static std::pair<TObj*, TObj*> Break(TObj* obj, ui64 index, TArenaImpl* arena) {
        assert(obj);
        assert(!obj->Left() && !obj->Right());
        assert(index > 0 && index < Size(obj));

        TObj* lePa = arena->Construct();
        TObj* raPa = arena->Construct();

        lePa->Base()->Size = index;
        raPa->Base()->Size = Size(obj) - index;

        TObj::TPolicy::Break(obj, lePa, raPa, index);

        arena->Delete(obj);

        return std::make_pair(lePa, raPa);
    }

    static TObj* Merge(TObj* left, TObj* right) {
        if (!left) {
            return right;
        }

        if (!right) {
            return left;
        }

        if (left->Base()->Priority > right->Base()->Priority) {
            TObj* merge = Merge(Cut(left, TBase::ESide::RIGHT), right);
            return Glue(left, merge, TBase::ESide::RIGHT);
        } else {
            TObj* merge = Merge(left, Cut(right, TBase::ESide::LEFT));
            return Glue(right, merge, TBase::ESide::LEFT);
        }
    }

    std::pair<TObj*, TObj*> Split(TObj* obj, ui64 index) {
        if (index == 0) {
            return std::make_pair(nullptr, obj);
        }

        if (index == Size(obj)) {
            return std::make_pair(obj, nullptr);
        }

        assert(index < Size(obj));

        if (Size(obj->Left()) >= index) {
            auto [left, right] = Split(Cut(obj, TBase::ESide::LEFT), index);
            return std::make_pair(left, Glue(obj, right, TBase::ESide::LEFT));
        } else if (index >= Size(obj) - Size(obj->Right())) {
            ui64 delta = Size(obj) - Size(obj->Right());
            auto [left, right] = Split(Cut(obj, TBase::ESide::RIGHT), index - delta);
            return std::make_pair(Glue(obj,  left, TBase::ESide::RIGHT), right);
        } else {
            assert(index > Size(obj->Left()) && index < Size(obj) - Size(obj->Right()));

            TObj* leCh = Cut(obj, TBase::ESide::LEFT);
            TObj* raCh = Cut(obj, TBase::ESide::RIGHT);

            auto [lePa, raPa] = Break(obj, index, Arena_);
            return std::make_pair(Merge(leCh, lePa), Merge(raPa, raCh));
        }
    }

    void Clear(TObj* obj) {
        if (!obj) {
            return;
        }

        Clear(obj->Left());
        Clear(obj->Right());

        Arena_->Delete(obj);
    }

    template <class TOp>
    void ForEach(TObj* obj, TOp op) {
        if (!obj) {
            return;
        }

        ForEach(obj->Left(), op);
        op(obj);
        ForEach(obj->Right(), std::move(op));
    }

private:
    static ui64 Size(TObj* obj) {
        if (obj) {
            return obj->Size();
        }

        return 0;
    }

private:
    TArenaImpl* Arena_ = nullptr;
    TObj* Root_ = nullptr;
};

}