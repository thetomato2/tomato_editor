// ============================================================================================
// Custom string stuff here
// TODO: float to string converstion -> https://github.com/ulfjack/ryu/tree/master/ryu
// not a trivial thing to implement, but my Handmade spirit wills me so
// TODO: c++ stream overload. I don't know if I want to use streams
// TODO: fix the operator+ overloads and write some tests
// TODO: small string optimizations
// ============================================================================================

namespace tom
{

function char itos(i32 d)
{
    TOM_ASSERT(d < 10);

    switch (d) {
        case 0: return '0';
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        default: break;
    }
    return '\0';
}

// TODO: this is pretty barebones
function i32 stoi(const char* str)
{
    i32 i = 0;
    while (*str >= '0' && *str <= '9') {
        i = i * 10 + (*str - '0');
        str++;
    }
    return i;
}

function i32 stoi(const char c)
{
    switch (c) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;

        default: break;
    }

    return -1;
}

template<typename CharT>
function bool str_equal(const CharT* a, const CharT* b)
{
    while (*a && *a == *b) {
        ++a;
        ++b;
    }
    if (*a != *b) return false;
    return true;
}

// TODO: vectorize this
template<typename CharT>
function szt str_len(const CharT* str)
{
    const CharT* s;
    for (s = str; *s; ++s)
        ;
    szt res = s - str;

    return res;
}

template<typename CharT>
function CharT* str_copy(const CharT* str)
{
    szt len       = str_len(str);
    CharT* result = (CharT*)plat_malloc(sizeof(CharT) * (len + 1));
    for (szt i = 0; i < len; ++i) {
        result[i] = *(str + i);
    }
    result[len] = '\0';

    return result;
}

template<typename CharT>
function void str_copy(CharT* buf, const CharT* str)
{
    szt len = str_len(str);
    // CharT* result = (CharT*)plat_malloc(sizeof(CharT) * (len + 1));
    for (szt i = 0; i < len; ++i) {
        buf[i] = *(str + i);
    }
    buf[len] = '\0';
}

template<typename CharT>
function CharT* str_copy(const CharT* str, szt len)
{
    CharT* result = (CharT*)plat_malloc(sizeof(CharT) * (len + 1));
    for (szt i = 0; i < len; ++i) {
        result[i] = *(str + i);
    }
    result[len] = '\0';

    return result;
}

template<typename CharT>
function CharT* str_copy(const CharT* buf1, const CharT* buf2)
{
    szt len1       = str_len(buf1);
    szt len2       = str_len(buf2);
    CharT* result  = (CharT*)plat_malloc(sizeof(CharT) * (len1 + len2 + 1));
    CharT* res_ptr = result;
    for (szt i = 0; i < len1; ++i) {
        result[i] = buf1[i];
    }

    for (szt i = 0; i < len2; ++i) {
        result[len1 + i] = buf2[i];
    }
    result[len1 + len2] = '\0';

    return result;
}

template<typename CharT>
function CharT* str_copy(const CharT* buf1, const CharT* buf2, const CharT* buf3)
{
    szt len1 = str_len(buf1);
    szt len2 = str_len(buf2);
    szt len3 = str_len(buf3);

    CharT* result = (CharT*)plat_malloc(sizeof(CharT) * (len1 + len2 + len3 + 1));

    for (szt i = 0; i < len1; ++i) {
        result[i] = buf1[i];
    }

    for (szt i = 0; i < len2; ++i) {
        result[len1 + i] = buf2[i];
    }

    for (szt i = 0; i < len3; ++i) {
        result[len1 + len2 + i] = buf3[i];
    }
    result[len1 + len2 + len3] = '\0';

    return result;
}

template<typename CharT>
function CharT* str_copy(const CharT* buf1, const CharT* buf2, szt len1, szt len2)
{
    CharT* result   = (CharT*)plat_malloc(sizeof(CharT) * (len1 + len2 + 1));
    CharT* res_ptr  = result;
    CharT* buf1_ptr = buf1;
    CharT* buf2_ptr = buf2;
    while (*buf1_ptr) {
        *res_ptr++ = *buf1_ptr++;
    }
    while (*buf2_ptr) {
        *res_ptr++ = *buf2_ptr++;
    }
    *res_ptr = '\0';

    return result;
}

template<typename CharT>
function CharT* str_copy(CharT* buf1, CharT c, szt len1)
{
    CharT* result   = (CharT*)plat_malloc(sizeof(CharT) * (len1 + 2));
    CharT* res_ptr  = result;
    CharT* buf1_ptr = buf1;
    while (*buf1_ptr) {
        *res_ptr++ = *buf1_ptr++;
    }
    *res_ptr++ = c;
    *res_ptr   = '\0';

    return result;
}

template<typename CharT>
function CharT* str_copy(CharT* buf1, const CharT* str)
{
    szt len1        = str_len(buf1);
    szt len2        = str_len(str);
    CharT* result   = (CharT*)plat_malloc(sizeof(CharT) * (len1 + len2 + 1));
    CharT* res_ptr  = result;
    CharT* buf1_ptr = buf1;
    while (*buf1_ptr) {
        *res_ptr++ = *buf1_ptr++;
    }
    for (szt i = 0; i < len2; ++i) {
        *res_ptr++ = *(str + i);
    }
    *res_ptr = '\0';

    return result;
}

template<typename CharT>
function CharT* rev_str(const CharT* str)
{
    szt len       = str_len(str);
    CharT* result = (CharT*)plat_malloc(sizeof(CharT) * (len + 1));
    for (szt i = 0; i < len; ++i) {
        result[i] = *(str + (len - i - 1));
    }
    result[len] = '\0';

    return result;
}
#if 0
template<typename CharT>
function StringBase<CharT> make_string(CharT* buf)
{
    StringBase<CharT> result;
    
    
    while (buf) {

    }
}
#endif

template<typename CharT>
struct StringBase
{
    const CharT* str;
    szt len;
};

typedef StringBase<char> String;
typedef StringBase<wchar> WString;

template<typename CharT>
class StringBuilderBase
{
    typedef typename std::make_unsigned<CharT>::type UCharT;

public:
    using value_type      = CharT;
    using reference       = CharT&;
    using const_reference = const CharT&;
    using pointer         = CharT*;
    using const_pointer   = const CharT*;
    using iterator        = CharT*;
    using const_iterator  = const CharT*;
    using riterator       = std::reverse_iterator<iterator>;
    using const_riterator = std::reverse_iterator<const_iterator>;
    using difference_type = std::ptrdiff_t;
    using size_type       = std::size_t;

    StringBuilderBase() {}

    StringBuilderBase(szt capacity) :
        _buf(capacity)
    {
    }

    ~StringBuilderBase() {}

    StringBuilderBase(const StringBuilderBase& copy) :
        _buf(copy._buf)
    {
    }

    StringBuilderBase& operator=(const StringBuilderBase& copy)
    {
        _buf = copy._buf;
        return *this;
    }

    StringBuilderBase(StringBuilderBase&& move) noexcept { move.swap(*this); }

    StringBuilderBase& operator=(StringBuilderBase&& move) noexcept
    {
        move.swap(*this);
        return *this;
    }

    void swap(StringBuilderBase& other) noexcept { std::swap(_buf, other._buf); }

    void push_back(CharT c) { _buf.push_back(c); }

    void push_back(const char* str)
    {
        szt len = str_len(str);
        for (szt i = 0; i < len; ++i) {
            _buf.push_back(str[i]);
        }
    }

    // TODO: memcpy of some sort faster?
    void push_back(const StringBase<CharT>& str)
    {
        for (auto c : str) {
            _buf.push_back(c);
        }
    }

    StringBase<CharT> to_string() const
    {
        if (_buf.back() != '\0') _buf.push_back('\0');
        StringBase<CharT> result { _buf.data(), _buf.size() };
        return result;
    }

    void push_back(i64 n)
    {
        bool neg = n < 0;
        if (neg) {
            n *= -1;
        }

        Vector<CharT> buf;

        if (n == 0) {
            buf.push_back('0');
        } else {
            while (n > 0) {
                i64 t = n % 10;
                buf.push_back(itos(t));
                n /= 10;
            }

            if (neg) buf.push_back('-');
            for (auto rit = buf.rbegin(); rit != buf.rend(); ++rit) {
                _buf.push_back(*rit);
            }
        }
    }

    void push_back(u64 n)
    {
        Vector<CharT> buf;

        if (n == 0) {
            buf.push_back('0');
        } else {
            while (n > 0) {
                u64 t = n % 10;
                buf.push_back(itos(t));
                n /= 10;
            }
            for (auto rit = buf.rbegin(); rit != buf.rend(); ++rit) {
                _buf.push_back(*rit);
            }
        }
    }

    void push_back(i32 n) { push_back((i64)n); }
    void push_back(i16 n) { push_back((i64)n); }
    void push_back(i8 n) { push_back((i64)n); }

    void push_back(u32 n) { push_back((u64)n); }
    void push_back(u16 n) { push_back((u64)n); }
    void push_back(u8 n) { push_back((u64)n); }

    void pop_back(szt n = 1) { _buf.pop_back(n); }
    void reserve(szt new_capacity) { _buf.reserve(new_capacity); }

    void clear() { _buf.clear(); }

    reference front() { return _buf.front(); }
    const_reference front() const { return _buf.front(); }
    reference back() { return _buf.back(); }
    const_reference back() const { _buf.back(); }

    iterator begin() { return _buf.begin(); }
    const_iterator begin() const { return _buf.begin(); }
    iterator end() { return _buf.end(); }
    const_iterator end() const { return _buf.end(); }

    riterator rbegin() { return _buf.rend(); }
    const_riterator rbegin() const { return _buf.rbegin(); }
    riterator rend() { return _buf.rend(); }
    const_riterator rend() const { return _buf.rend(); }

    const_iterator cbegin() const { return _buf.cbegin(); }
    const_riterator crbegin() const { return _buf.crbegin(); }
    const_iterator cend() const { return _buf.cend(); }
    const_riterator crend() const { return _buf.crend(); }

    pointer data() { return _buf.data(); }
    pointer data() const { return _buf.data(); }

    szt size() const { return _buf.size(); }
    bool empty() const { return _buf.empty(); }

    reference operator[](szt i) { return _buf[i]; }
    reference const& operator[](szt i) const { return _buf[i]; }

    StringBuilderBase& operator+=(const char rhs)
    {
        push_back(rhs);
        return *this;
    }
    StringBuilderBase& operator+=(const char* rhs)
    {
        push_back(rhs);
        return *this;
    }
    StringBuilderBase& operator+=(const StringBase<CharT>& rhs)
    {
        push_back(rhs);
        return *this;
    }
    StringBuilderBase& operator<<(const char rhs)
    {
        push_back(rhs);
        return *this;
    }
    StringBuilderBase& operator<<(const char* rhs)
    {
        push_back(rhs);
        return *this;
    }
    StringBuilderBase& operator<<(const StringBase<CharT>& rhs)
    {
        push_back(rhs);
        return *this;
    }

private:
    Vector<CharT> _buf;
};

typedef StringBuilderBase<char> StrBuilder;
typedef StringBuilderBase<wchar> WStrBuilder;

template<typename CharT>
StringBase<CharT> operator+(const StringBase<CharT>& lhs, char rhs)
{
    StringBuilderBase<CharT> result { lhs };
    result.push_back(rhs);
    return result.to_string();
}

template<typename CharT>
StringBase<CharT> operator+(const StringBase<CharT>& lhs, const char* rhs)
{
    StringBuilderBase<CharT> result;
    result.push_back(lhs);
    result.push_back(rhs);
    return result.to_string();
}

template<typename CharT>
StringBase<CharT> operator+(const StringBase<CharT>& lhs, StringBase<CharT>& rhs)
{
    StringBuilderBase<CharT> result;
    result.push_back(lhs);
    result.push_back(rhs);
    return result.to_string();
}
#if 0
function String convert_wstring_to_string(const WString& wstr)
{
    i32 cnt   = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    char* buf = new char[cnt];
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, buf, cnt, NULL, NULL);
    String result { buf };
    delete buf;

    return result;
}

function WString convert_string_to_wstring(const String& str)
{
    i32 cnt    = MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
    wchar* buf = new wchar[cnt];
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buf, cnt);
    WString result { buf };
    delete buf;

    return result;
}

function String convert_wstring_to_string_utf8(const WString& wstr)
{
    i32 cnt   = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    char* buf = new char[cnt];
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buf, cnt, NULL, NULL);
    String result { buf };
    delete buf;

    return result;
}

function WString convert_string_to_wstring_utf8(const String& str)
{
    i32 cnt    = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar* buf = new wchar[cnt];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buf, cnt);
    WString result { buf };
    delete buf;

    return result;
}
#endif

}  // namespace tom