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

function char parse_digit(i32 d)
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

}  // namespace tom