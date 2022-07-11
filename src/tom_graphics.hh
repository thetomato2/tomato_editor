
#define MAX_TEXTURE_COUNT        16384
#define MAX_GLYPH_COUNT          16384
#define MAX_RENDER_COMMAND_COUNT 1028

#define SIZE_OF_GLYPH_INSTANCE_IN_BYTES (sizeof(float) * 14)

#define GLYPH_INSTANCE_DATA_TOTAL_SIZE_IN_BYTES MAX_GLYPH_COUNT* SIZE_OF_GLYPH_INSTANCE_IN_BYTES

#define SIZE_OF_TEXTURE_INSTANCE_IN_BYTES (sizeof(float) * 14)
#define TEXTURE_INSTANCE_DATA_TOTAL_SIZE_IN_BYTES \
    MAX_TEXTURE_COUNT* SIZE_OF_TEXTURE_INSTANCE_IN_BYTES

#ifdef TOM_INTERNAL
    #define D3D_CHECK(x)                                                      \
        {                                                                     \
            if (FAILED(x)) {                                                  \
                ScopedPtr<char> hr_err = d3d_error_code(x);                   \
                printf("ERROR-> DirectX Check Failed! - %s\n", hr_err.get()); \
                d3d_print_info_queue(gfx);                                    \
                TOM_INVALID_CODE_PATH;                                        \
            }                                                                 \
        }
#else
    #define D3D_CHECK(x)
#endif

namespace tom
{

struct d3d_Constants
{
    m4 transform;
    m4 projection;
    v3 light_v3;
};

struct ShaderProg
{
    ID3DBlob* vs_blob;
    ID3DBlob* ps_blob;
    ID3D11VertexShader* vs;
    ID3D11PixelShader* ps;
};

struct GfxState
{
    ID3D11Device1* device;
    ID3D11DeviceContext1* context;
    IDXGISwapChain1* swap_chain;
    ID3D11InfoQueue* info_queue;
    ID3D11RenderTargetView* render_target_view;
    ID3D11DepthStencilView* depth_buf_view;
    ID3D11DepthStencilState* depth_stencil_state;
    ID3D11RasterizerState1* rasterizer_state;
    ID3D11SamplerState* sampler_state;
    ID3D11BlendState* blend_state;
    D3D11_VIEWPORT viewport;
};

// pos, col
union Vertex
{
    struct
    {
        v4 pos;
        v4 col;
        v2 uv;
    };
    f32 e[10];
};

struct Quad
{
    Vertex e[4];
};

}  // namespace tom