namespace tom
{

struct Color
{
    union
    {
        struct
        {
            u8 r;
            u8 g;
            u8 b;
            u8 a;
        };
        u32 rgba;
    };
};

inline Color v3_to_color(v3 col, f32 a = 1.0f)
{
    Color res;

    res.r = (u8)(col.r * 255.0f);
    res.g = (u8)(col.g * 255.0f);
    res.b = (u8)(col.b * 255.0f);
    res.a = (u8)(a * 255.0f);

    return res;
}

inline Color f32_to_color(f32 col, f32 a = 1.0f)
{
    Color res;

    res.r = (u8)(col * 255.0f);
    res.g = (u8)(col * 255.0f);
    res.b = (u8)(col * 255.0f);
    res.a = (u8)(a * 255.0f);

    return res;
}

global constexpr v4 red_v4   = { 1.0f, 0.0f, 0.0f, 1.0f };
global constexpr v4 green_v4 = { 0.0f, 1.0f, 0.0f, 1.0f };
global constexpr v4 blue_v4  = { 0.0f, 0.0f, 1.0f, 1.0f };
global constexpr v4 pink_v4  = { 1.0f, 0.0f, 1.0f, 1.0f };

global constexpr v3 red_v3   = { 1.0f, 0.0f, 0.0f };
global constexpr v3 green_v3 = { 0.0f, 1.0f, 0.0f };
global constexpr v3 blue_v3  = { 0.0f, 0.0f, 1.0f };

// note: argb
// global constexpr color red   =  0xff'00'00'ff ;
// global constexpr color green =  0x00'ff'00'ff ;
// global constexpr color blue  =  0x00'00'ff'ff ;
// global constexpr color pink  =  0xff'00'ff'ff ;
// global constexpr color white =  0xff'ff'ff'ff ;
// global constexpr color black =  0x00'00'00'ff ;

}  // namespace tom
