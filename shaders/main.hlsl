cbuffer constants : register(b0)
{
    float4x4 proj;
    float4x4 model;
    float4 text_col;
    float2 tex_sz;
    float2 uv_off;
    float2 i_cnt;
    float uv_r;
    int glyph_all;
};

struct Vs_In
{
    float4 pos : POSITION;
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

    OUT.pos = mul(proj, mul(model, IN.pos)); 
    OUT.col = text_col;

    if (glyph_all > 0){
        OUT.coord = IN.uv;
    }
    else{
        OUT.coord = (uv_off + (tex_sz / i_cnt) * IN.uv) / tex_sz;
    }

    return OUT;
}

float4 ps_main(Vs_Out IN) : SV_TARGET
{
    float tex_alpha =  tex.Sample(samp, IN.coord);
    float4 out_col;
    out_col.r = IN.col.r;
    out_col.g = IN.col.g;
    out_col.b = IN.col.b;
    out_col.a = tex_alpha;
     
     return out_col;
}
