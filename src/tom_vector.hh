

namespace tom
{

// ============================================================================================
// Simple custom vector class that stores data on the heap
// Useful for temporary allocations that you don't know the end size of
// TODO: custom generic memory allocator
// ============================================================================================

template<typename T>
class Vector
{
public:
    using value_type      = T;
    using reference       = T&;
    using const_reference = const T&;
    using pointer         = T*;
    using const_pointer   = const T*;
    using iterator        = T*;
    using const_iterator  = T*;
    using riterator       = std::reverse_iterator<iterator>;
    using const_riterator = std::reverse_iterator<const_iterator>;
    using difference_type = std::ptrdiff_t;
    using size_type       = std::size_t;

    Vector() :
        _size(0),
        _cap(10),
        _buf(static_cast<T*>(plat_malloc(sizeof(T) * _cap)))
    {
    }

    Vector(size_type capacity) :
        _size(0),
        _cap(capacity),
        _buf(static_cast<T*>(plat_malloc(sizeof(T) * _cap)))
    {
    }

    template<typename I>
    Vector(I begin, I end) :
        _cap(std::distance(begin, end)),
        _size(0),
        _buf(static_cast<T*>(plat_malloc(sizeof(T) * _cap)))
    {
        for (auto it = begin; it != end; ++it) {
            internal_push_back(*it);
        }
    }

    Vector(const std::initializer_list<T>& list) :
        Vector(std::begin(list), std::end(list))
    {
    }

    ~Vector()
    {
        // NOTE: will deallocate the buffer if an exception is thrown
        // and blows up the stack
        deleter del(_buf);
        clear_elements<T>();
    }

    Vector(const Vector& copy) :
        _size(0),
        _cap(copy._cap),
        _buf(static_cast<T*>(plat_malloc(sizeof(T) * _cap)))
    {
        // try {
        //     for (u32 i = 0; i < copy._size; ++i) {
        //         push_back(copy._buf[i]);
        //     }
        // } catch (...) {
        //     deleter del(_buf);
        //     clear_elements<T>();

        //     // Make sure the exceptions continue propagating after
        //     // the cleanup has completed.
        //     throw;
        // }

        for (u32 i = 0; i < copy._size; ++i) {
            push_back(copy._buf[i]);
        }
    }

    Vector& operator=(const Vector& copy)
    {
        copy_assign<T>(copy);
        return *this;
    }

    Vector(Vector&& move) noexcept :
        _cap(0),
        _size(0),
        _buf(nullptr)
    {
        move.swap(*this);
    }

    Vector& operator=(Vector&& move) noexcept
    {
        move.swap(*this);
        return *this;
    }

    void swap(Vector& other) noexcept
    {
        std::swap(_cap, other._cap);
        std::swap(_size, other._size);
        std::swap(_buf, other._buf);
    }

    void push_back(const value_type& val)
    {
        resize_if_required();
        internal_push_back(val);
    }

    void push_back(value_type&& val)
    {
        resize_if_required();
        internal_move_back(val);
    }

    template<typename... Args>
    void emplace_back(Args&&... args)
    {
        resize_if_required();
        internal_emplace_back(std::move(args)...);
    }

    void pop_back(size_type n = 1)
    {
        for (szt i = 0; i < n; ++i) {
            if (_size == 0) break;
            _buf[_size--].~T();
        }
    }

    void reserve(size_type new_capacity)
    {
        if (new_capacity > _cap) {
            reserve_capacity(new_capacity);
        }
    }

    void clear()
    {
        clear_elements<T>();
        _size = 0;
    }

    size_type size() const { return _size; }

    bool empty() const { return _size == 0; }

    reference at(size_type index)
    {
        validate_index(index);
        return _buf[index];
    }

    const_reference at(size_type index) const
    {
        validate_index(index);
        return _buf[index];
    }

    reference front() { return _buf[0]; }
    const_reference front() const { return _buf[0]; }
    reference back() { return _buf[_size - 1]; }
    const_reference back() const { _buf[_size - 1]; }

    iterator begin() { return _buf; }
    const_iterator begin() const { return _buf; }
    iterator end() { return _buf + _size; }
    const_iterator end() const { return _buf + _size; }

    riterator rbegin() { return riterator(end()); }
    const_riterator rbegin() const { return const_riterator(end()); }
    riterator rend() { return riterator(begin()); }
    const_riterator rend() const { return const_riterator(begin()); }

    const_iterator cbegin() const { return begin(); }
    const_riterator crbegin() const { return rbegin(); }
    const_iterator cend() const { return end(); }
    const_riterator crend() const { return rend(); }

    pointer data() { return &_buf[0]; }
    pointer data() const { return &_buf[0]; }

    reference operator[](size_type index)
    {
        TOM_ASSERT(index < _size);
        return _buf[index];
    }

    const_reference operator[](size_type index) const
    {
        TOM_ASSERT(index < _size);
        return _buf[index];
    }

    bool operator!=(const Vector& rhs) const { return !(*this == rhs); }

    bool operator==(const Vector& rhs) const
    {
        return (size() == rhs.size()) && std::equal(begin(), end(), rhs.begin());
    }

private:
    size_type _size;
    size_type _cap;
    T* _buf;

    struct deleter
    {
        void* buf = nullptr;

        deleter(T* buf_) :
            buf((void*)buf_)
        {
        }

        ~deleter() { plat_free(buf); }
    };

    void validate_index(size_type index)
    {
        if (index >= _size) {
            TOM_INVALID_CODE_PATH;
            // throw std::out_of_range("Out of Range");
        }
    }

    void resize_if_required()
    {
        TOM_ASSERT(_size <= _cap);

        if (_size == _cap) {
            szt new_cap = (szt)std::max(2.0, _cap * 1.5);
            reserve_capacity(new_cap);
        }
    }

    void reserve_capacity(size_type new_cap)
    {
        Vector<T> tmp_buf(new_cap);
        simple_copy<T>(tmp_buf);
        tmp_buf.swap(*this);
    }

    void internal_push_back(const value_type& val)
    {
        new (_buf + _size) T(val);
        ++_size;
    }

    void internal_move_back(value_type&& val)
    {
        new (_buf + _size) T(std::move(val));
        ++_size;
    }

    template<typename... Args>
    void internal_emplace_back(Args&&... args)
    {
        new (_buf + _size) T(std::move(args)...);
        ++_size;
    }

    // optimizations that use SFINAE to only instantiate one
    // of two versions of a function.
    //      simple_copy()       moves when no exceptions are guaranteed, otherwise copies.
    //      clear_elements()    when no destructor remove loop.
    //      copy_assign()       avoid resource allocation when no exceptions guaranteed.
    //                          ie. when copying integers reuse the buffer if we can
    //                          to avoid expensive resource allocation.

    template<typename X>
    typename std::enable_if<std::is_nothrow_move_constructible<X>::value == false>::type
    simple_copy(Vector<T>& dst)
    {
        std::for_each(_buf, _buf + _size, [&dst](const T& v) { dst.internal_push_back(v); });
    }

    template<typename X>
    typename std::enable_if<std::is_nothrow_move_constructible<X>::value == true>::type simple_copy(
        Vector<T>& dst)
    {
        std::for_each(_buf, _buf + _size, [&dst](T& v) { dst.internal_move_back(std::move(v)); });
    }

    template<typename X>
    typename std::enable_if<std::is_trivially_destructible<X>::value == false>::type
    clear_elements()
    {
        // Call the destructor on all the members in reverse order
        for (int i = 0; i < _size; ++i) {
            // Note we destroy the elements in reverse order.
            _buf[_size - 1 - i].~T();
        }
    }

    template<typename X>
    typename std::enable_if<std::is_trivially_destructible<X>::value == true>::type clear_elements()
    {
        // Trivially destructible objects can be reused without using the destructor.
    }

    template<typename X>
    typename std::enable_if<(std::is_nothrow_copy_constructible<X>::value &&
                             std::is_nothrow_destructible<X>::value) == true>::type
    copy_assign(const Vector<X>& copy)
    {
        // this function is only used if there is no chance of an exception being
        // thrown during destruction or copy construction of the type t.

        // quick return for self assignment.
        if (this == &copy) {
            return;
        }

        if (_cap >= copy._size) {
            // if we have enough space to copy then reuse the space we currently
            // have to avoid the need to perform an expensive resource allocation.

            clear_elements<T>();  // potentially does nothing (see above)
                                  // but if required will call the destructor of
                                  // all elements.

            // buffer now ready to get a copy of the data.
            _size = 0;
            for (szt i = 0; i < copy._size; ++i) {
                internal_push_back(copy[i]);
            }
        } else {
            // fallback to copy and swap if we need to more space anyway
            Vector<T> tmp(copy);
            tmp.swap(*this);
        }
    }

    template<typename X>
    typename std::enable_if<(std::is_nothrow_copy_constructible<X>::value &&
                             std::is_nothrow_destructible<X>::value) == false>::type
    copy_assign(Vector<X>& copy)
    {
        // Copy and Swap idiom
        Vector<T> tmp(copy);
        tmp.swap(*this);
    }
};

}  // namespace tom
