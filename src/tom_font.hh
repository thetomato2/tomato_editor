// ============================================================================================
// Hard-coded to work with one simple bitmap image
// TODO: all the font stuff
// ============================================================================================

namespace tom
{

#define MAX_GLYPH_CNT 256

struct Glyph
{
    v2 uv;
};

struct FontSheet
{
    u32 glyph_cnt;
    Glyph glyphs[MAX_GLYPH_CNT];
};

// NOTE: HARDCODED!!!
// maps a char to a glyph index
function i32 get_glyph_index(char c)
{
    switch (c) {
        case 'A': return 72;
        case 'B': return 73;
        case 'C': return 74;
        case 'D': return 75;
        case 'E': return 76;
        case 'F': return 77;
        case 'G': return 78;
        case 'H': return 79;
        case 'I': return 80;
        case 'J': return 63;
        case 'K': return 64;
        case 'L': return 65;
        case 'M': return 66;
        case 'N': return 67;
        case 'O': return 68;
        case 'P': return 69;
        case 'Q': return 70;
        case 'R': return 71;
        case 'S': return 54;
        case 'T': return 55;
        case 'U': return 56;
        case 'V': return 57;
        case 'W': return 58;
        case 'X': return 59;
        case 'Y': return 60;
        case 'Z': return 61;
        case 'a': return 45;
        case 'b': return 46;
        case 'c': return 47;
        case 'd': return 48;
        case 'e': return 49;
        case 'f': return 50;
        case 'g': return 51;
        case 'h': return 52;
        case 'i': return 53;
        case 'j': return 36;
        case 'k': return 37;
        case 'l': return 38;
        case 'm': return 39;
        case 'n': return 40;
        case 'o': return 41;
        case 'p': return 42;
        case 'q': return 43;
        case 'r': return 44;
        case 's': return 27;
        case 't': return 28;
        case 'u': return 29;
        case 'v': return 30;
        case 'w': return 31;
        case 'x': return 32;
        case 'y': return 33;
        case 'z': return 34;
        case '0': return 18;
        case '1': return 19;
        case '2': return 20;
        case '3': return 21;
        case '4': return 22;
        case '5': return 23;
        case '6': return 24;
        case '7': return 25;
        case '8': return 26;
        case '9': return 9;
        case '!': return 17;
        case '\'': return 16;
        case '#': return 15;
        case '$': return 14;
        case ':': return 13;
        case ';': return 12;
        case ',': return 11;
        case '_': return 10;
        case ' ': return 8;
        case '@': return 7;
        case ')': return 6;
        case '(': return 5;
        case '&': return 4;
        case '%': return 3;
        case '?': return 2;
        case '/': return 1;
        case '\"': return 0;
    }

    return -1;
}

// NOTE: hard-coded
// TODO: dynanic font-sheet creation
function FontSheet create_font_sheet()
{
    FontSheet result;

    f32 w = (1.0f / 9.0f) * 1000.0f;
    f32 h = (1.0f / 9.0f) * 1150.0f;

    for (i32 y = 0; y < 9; ++y) {
        for (i32 x = 0; x < 9; ++x) {
            i32 i                 = (y * 9) + x;
            result.glyphs[i].uv.x = w * (f32)x;
            result.glyphs[i].uv.y = h * (f32)y;
        }
    }

    return result;
}

}  // namespace tom
