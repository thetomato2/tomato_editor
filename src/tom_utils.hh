
#include "core.hh"

namespace tom
{

template<typename First, typename Second>
struct Pair
{
    First first;
    Second second;
};

template<class FwdIt, class T>
void fill(FwdIt first, FwdIt last, const T& val)
{
    for (; first != last; ++first) {
        *first = val;
    }
}

template<typename T>
unsigned char* uchar_cast(T* p)
{
    return reinterpret_cast<unsigned char*>(p);
}

template<typename T>
unsigned char const* uchar_cast(T const* p)
{
    return reinterpret_cast<unsigned char const*>(p);
}

template<typename T>
unsigned char& most_sig_byte(T& obj)
{
    return *(reinterpret_cast<unsigned char*>(&obj) + sizeof(obj) - 1);
}

template<int N>
bool lsb(unsigned char byte)
{
    return byte & (1u << N);
}

template<int N>
bool msb(unsigned char byte)
{
    return byte & (1u << (CHAR_BIT - N - 1));
}

template<int N>
void set_lsb(unsigned char& byte, bool bit)
{
    if (bit) {
        byte |= 1u << N;
    } else {
        byte &= ~(1u << N);
    }
}

template<int N>
void set_msb(unsigned char& byte, bool bit)
{
    if (bit) {
        byte |= 1u << (CHAR_BIT - N - 1);
    } else {
        byte &= ~(1u << (CHAR_BIT - N - 1));
    }
}

}  // namespace tom
