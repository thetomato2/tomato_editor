#include <cassert>
#include <cmath>

#include <cstdio>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#ifndef NOMINMAX
    #define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN 0
#include <windows.h>
#include <tchar.h>

#include <d3d11_1.h>
#include <d3dcompiler.h>

#include <xinput.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../extern/stb/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "../extern/stb/stb_truetype.h"

#ifdef _MSVC
    #define MSVC 1
#endif

#ifdef _LLVM
    #define LLVM 1
#endif

#if MSVC
    /* #include <intrin.h> */
    #pragma intrinsic(_BitScanForward)
#endif

#include "tom_types.hh"

#define internal static
#define function static
#define global   static
#define local    static

#define Z_UP 1

#ifdef TOM_WIN32
    #define TOM_DLL_EXPORT __declspec(dllexport)
#else
    #define TOM_DLL_EXPORT
#endif

#ifdef TOM_INTERNAL
    #define TOM_ASSERT(x)                                               \
        if (!(x)) {                                                     \
            printf("FAILED ASSERT -> %s at :%d\n", __FILE__, __LINE__); \
            __debugbreak();                                             \
        }                                                               \
        assert(x)

    #define TOM_ASSERT_MSG(x, msg)                                                \
        if (!(x)) {                                                               \
            printf("FAILED ASSERT -> %s at :%d - %s\n", __FILE__, __LINE__, msg); \
            __debugbreak();                                                       \
        }                                                                         \
        assert(x)
    #define DEBUG_BREAK(x)  \
        if (x) {            \
            __debugbreak(); \
        }
    #define INTERNAL_ONLY_EXECUTE(args) args
#else
    #define TOM_ASSERT(x)
    #define TOM_ASSERT_MSG(x, msg)
    #define DEBUG_BREAK(x)
    #define INTERNAL_ONLY_EXECUTE(args)
#endif

// FIXME: there is no EXPP_TEXT
#define EXPP_HRESULT(hr, what) EXPP_EXCEPTION(HRESULT, (HRESULT)hr, EXPP_TEXT(what))
#define EVALUATE_HRESULT(call, what)      \
    {                                     \
        HRESULT hr;                       \
        if (FAILED(hr = call)) {          \
            throw EXPP_HRESULT(hr, what); \
        }                                 \
    }
#define EVALUATE_HRESULT_HR(hr, call, what) \
    if (FAILED(hr = call)) {                \
        throw EXPP_HRESULT(hr, what);       \
    }

#define TOM_INVALID_CODE_PATH TOM_ASSERT(!"Invalid code path!")

#define ARRAY_COUNT(array) (sizeof((array)) / sizeof((array)[0]))
#define STRINGIFY(x) #x

namespace tom
{
struct ThreadContext
{
    i32 place_holder;
};

struct WindowDims
{
    i32 width;
    i32 height;
};

// Generic flag stuff

inline bool is_flag_set(i32 flags, i32 flag)
{
    return flags & flag;
}

inline void set_flags(i32& flags, i32 flag)
{
    flags |= flag;
}

inline void clear_flags(i32& flags, i32 flag)
{
    flags &= ~flag;
}

inline u32 safe_truncate_u32_to_u64(u64 value)
{
    TOM_ASSERT(value <= U32_MAX);
    u32 result = (u32)value;
    return result;
}
}  // namespace tom

#include "tom_intrinsic.hh"
#include "tom_math.hh"
#include "tom_color.hh"
#include "tom_memory.hh"
#include "tom_string.hh"
#include "tom_input.hh"
#include "tom_time.hh"
#include "tom_file_io.hh"
#include "tom_sound.hh"
#include "tom_win32.hh"
#include "tom_graphics.hh"
#include "tom_dx_error.hh"
#include "tom_font.hh"
#include "tom_camera.hh"
#include "tom_app.hh"
