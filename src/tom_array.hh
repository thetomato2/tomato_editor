#ifndef TOMATO_ARRAY_HH_
#define TOMATO_ARRAY_HH_

#include "core.hh"

namespace tom
{

// ============================================================================================
// C-array wrapper
// ============================================================================================

template<typename T, szt Size>
class array
{
    using value_type      = T;
    using reference       = T&;
    using const_reference = T const&;
    using pointer         = T*;
    using const_pointer   = T const*;
    using iterator        = T*;
    using const_iterator  = T const*;
    using riterator       = std::reverse_iterator<iterator>;
    using const_riterator = std::reverse_iterator<const_iterator>;
    using difference_type = std::ptrdiff_t;
    using size_type       = std::size_t;

public:
    array() :
        _size(Size)
    {
    }

    template<typename I>
    array(I begin, I end) :
        _size(Size)
    {
        TOM_ASSERT(std::distance(begin, end) <= _size);
        s32 i = 0;
        for (auto it = begin; it != end; ++it) {
            _buf[i++] = *it;
        }
    }

    array(const std::initializer_list<T>& list) :
        array(std::begin(list), std::end(list))
    {
    }

    ~array() {}

    array(const array& copy) :
        _size(Size)
    {
        for (szt i = 0; i < _size; ++i) {
            if (i < copy._size) {
                _buf[i] = copy._buf[i];
            }
        }
    }

    array& operator=(const array& lhs)
    {
        for (szt i = 0; i < _size; ++i) {
            if (i < lhs._size) {
                _buf[i] = lhs._buf[i];
            }
        }
    }

    array(array&& move) noexcept :
        _size(Size)
    {
        move.swap(*this);
    }

    array& operator=(array&& move)
    {
        move.swap(*this);
        return *this;
    }

    // NOTE: does a placement new
    template<class... Args>
    void emplace_at(szt i, Args const&... args)
    {
        TOM_ASSERT(i < _size);
        _buf[i].~T();
        new (&_buf[i]) T(args...);
    }

    void swap(array& other) noexcept
    {
        for (s32 i = 0; i < _size; ++i) {
            std::swap(_buf[i], other._buf[i]);
        }
    }

    size_type size() const { return _size; }
    pointer* data() { return &_buf[0]; }
    pointer* data() const { return &_buf[0]; }

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

    bool operator!=(const array& rhs) const { return !(*this == rhs); }
    bool operator==(const array& rhs) const
    {
        return (size() == rhs.size()) && std::equal(begin(), end(), rhs.begin());
    }

private:
    size_type _size;
    T _buf[Size];
};

}  // namespace tom

#endif