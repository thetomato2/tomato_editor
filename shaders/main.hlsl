cbuffer constants : register(b0)
{
    float4x4 proj;
    float4x4 model;
    float2 uv_off;
    int glyph_all;
};

struct Vs_In
{
    float4 pos : POSITION;
    float4 col : COLOR;
    float2 uv : TEXCOORD;
};

struct Vs_Out {
    float4 col : COLOR;
    float4 pos : SV_POSITION;
    float2 coord : TEXCOORD;
};

Texture2D tex;
SamplerState samp;

Vs_Out vs_main(Vs_In IN)
{
    Vs_Out OUT;

    OUT.col = IN.col;
    OUT.pos = mul(proj, mul(model, IN.pos)); 

    if (glyph_all > 0){
        OUT.coord = IN.uv;
    }
    else{
        // TODO: tex.GetDimensions() doesn't work???
        float2 tex_sz;
        tex_sz.x = 1000.0f;
        tex_sz.y = 1150.0f;
        float2 offset;
        offset.x = 0.0f;
        offset.y = 127.0f;
        OUT.coord = (uv_off + (tex_sz / 9.0f) * IN.uv) / tex_sz;
    }

    return OUT;
}

float4 ps_main(Vs_Out IN) : SV_TARGET
{
    float4 tex_col =  tex.Sample(samp, IN.coord);
    tex_col.r += IN.col.r;
    tex_col.g += IN.col.g;
    tex_col.b += IN.col.b;
     
     return tex_col;
}
