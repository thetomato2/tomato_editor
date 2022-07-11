namespace tom
{

function void d3d_print_info_queue(GfxState* gfx)
{
    szt msg_cnt = gfx->info_queue->GetNumStoredMessages();

    for (szt i = 0; i < msg_cnt; ++i) {
        szt msg_sz;
        gfx->info_queue->GetMessage(i, nullptr, &msg_sz);
        D3D11_MESSAGE* msg = (D3D11_MESSAGE*)plat_malloc(msg_sz);  // TODO: create an arena for this
        // TODO: basic logging system and/or colors
        if (SUCCEEDED(gfx->info_queue->GetMessage(i, msg, &msg_sz))) {
            switch (msg->Severity) {
                case D3D11_MESSAGE_SEVERITY_MESSAGE:
                    printf("D3D MESSAGE-> %s\n", msg->pDescription);
                    break;
                case D3D11_MESSAGE_SEVERITY_INFO:
                    printf("D3D INFO-> %s\n", msg->pDescription);
                    break;
                case D3D11_MESSAGE_SEVERITY_WARNING:
                    printf("D3D WARNING-> %s\n", msg->pDescription);
                    break;
                case D3D11_MESSAGE_SEVERITY_CORRUPTION:
                    printf("D3D CORRUPTION-> %s\n", msg->pDescription);
                    break;
                case D3D11_MESSAGE_SEVERITY_ERROR:
                    printf("D3D ERROR-> %s\n", msg->pDescription);
                    break;
                default: printf("D3D: %s\n", msg->pDescription); break;
            }
        }
        plat_free(msg);
    }
    gfx->info_queue->ClearStoredMessages();
}

function void d3d_create_resources(GfxState* gfx, WindowDims win_dims)
{
    ID3D11RenderTargetView* null_views[] { nullptr };
    gfx->context->OMSetRenderTargets(sizeof(null_views), null_views, nullptr);
    gfx->render_target_view->Release();
    gfx->depth_buf_view->Release();
    gfx->context->Flush();
}

function void d3d_resize_buffers(GfxState* gfx)
{
    gfx->context->OMSetRenderTargets(0, 0, 0);
    gfx->render_target_view->Release();
    HRESULT hr = gfx->swap_chain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    TOM_ASSERT(SUCCEEDED(hr));
}

function ShaderProg d3d_create_shader_prog(GfxState* gfx, LPCWSTR path)
{
    ShaderProg result;
    u32 flags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef TOM_INTERNAL
    flags |= D3DCOMPILE_DEBUG;
#endif
    ID3DBlob* err_msg_blob;

    auto print_error = [](const char* shader_type, ID3DBlob* err_msg) {
        printf(
            "==========================================================\n"
            "ERROR-> %s shader failed to compile!\n"
            "%s\n"
            "==========================================================\n",
            shader_type, (char*)err_msg->GetBufferPointer());
    };

    if (SUCCEEDED(D3DCompileFromFile(path, nullptr, nullptr, "vs_main", "vs_5_0", flags, 0,
                                     &result.vs_blob, &err_msg_blob))) {
        D3D_CHECK(gfx->device->CreateVertexShader(result.vs_blob->GetBufferPointer(),
                                                  result.vs_blob->GetBufferSize(), nullptr,
                                                  &result.vs));
    } else {
        print_error("vertex", err_msg_blob);
        err_msg_blob->Release();
        TOM_INVALID_CODE_PATH;
    }

    if (SUCCEEDED(D3DCompileFromFile(path, nullptr, nullptr, "ps_main", "ps_5_0", flags, 0,
                                     &result.ps_blob, &err_msg_blob))) {
        D3D_CHECK(gfx->device->CreatePixelShader(result.ps_blob->GetBufferPointer(),
                                                 result.ps_blob->GetBufferSize(), nullptr,
                                                 &result.ps));
    } else {
        print_error("pixel", err_msg_blob);
        err_msg_blob->Release();
        TOM_INVALID_CODE_PATH;
    }

    return result;
}

function void d3d_init(HWND hwnd, GfxState* gfx)
{
    D3D_FEATURE_LEVEL feature_levels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
                                           D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };

    u32 device_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef TOM_INTERNAL
    device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    ID3D11Device* base_device;
    ID3D11DeviceContext* base_device_context;
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, device_flags,
                                 feature_levels, ARRAYSIZE(feature_levels), D3D11_SDK_VERSION,
                                 &base_device, nullptr, &base_device_context))) {
        printf("ERROR!-> Failed to create base D3D11 Base Device!");
        TOM_INVALID_CODE_PATH;
    }

    ID3D11Device1* device;
    if (FAILED(base_device->QueryInterface(__uuidof(ID3D11Device1), (void**)&device))) {
        printf("ERROR!-> Failed to create D3D11 Device!");
        TOM_INVALID_CODE_PATH;
    }
    ID3D11DeviceContext1* device_context;
    if (FAILED(base_device_context->QueryInterface(__uuidof(ID3D11DeviceContext1),
                                                   (void**)&device_context))) {
        printf("ERROR!-> Failed to create D3D11 Device Context!");
        TOM_INVALID_CODE_PATH;
    }

    IDXGIDevice1* dxgi_device;
    if (FAILED(device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgi_device))) {
        printf("ERROR!-> Failed to create D3D11 DXGI Device!");
        TOM_INVALID_CODE_PATH;
    }
    IDXGIAdapter* dxgi_adapter;
    if (FAILED(dxgi_device->GetAdapter(&dxgi_adapter))) {
        printf("ERROR!-> Failed to create D3D11 DXGI adapter!");
        TOM_INVALID_CODE_PATH;
    }
    IDXGIFactory2* dxgi_factory;
    if (FAILED(dxgi_adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgi_factory))) {
        printf("ERROR!-> Failed to create D3D11 DXGI Factory!");
        TOM_INVALID_CODE_PATH;
    }

#ifdef TOM_INTERNAL
    ID3D11Debug* d3d_debug;
    if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3d_debug))) {
        ID3D11InfoQueue* info_queue;
        if (SUCCEEDED(device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&info_queue))) {
            info_queue->SetBreakOnSeverity((D3D11_MESSAGE_SEVERITY_CORRUPTION), true);
            info_queue->SetBreakOnSeverity((D3D11_MESSAGE_SEVERITY_ERROR), true);
            D3D11_MESSAGE_ID hide[] { D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS };
            D3D11_INFO_QUEUE_FILTER filter {};
            filter.DenyList.NumIDs  = (u32)ARRAY_COUNT(hide);
            filter.DenyList.pIDList = hide;
            info_queue->AddStorageFilterEntries(&filter);
            gfx->info_queue = info_queue;
        } else {
            printf("ERROR!-> Failed to create D3D11 Info Queue!");
            TOM_INVALID_CODE_PATH;
        }

    } else {
        printf("ERROR!-> Failed to create D3D11 debug context!");
        TOM_INVALID_CODE_PATH;
    }
#endif

    DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
        // .Width  = 0,  // use window width
        // .Height = 0,  // use window height
        //.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        .Format      = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc  = { .Count = 1, .Quality = 0 },
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .Scaling     = DXGI_SCALING_STRETCH,
        .SwapEffect  = DXGI_SWAP_EFFECT_DISCARD,
        .AlphaMode   = DXGI_ALPHA_MODE_UNSPECIFIED,
    };

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC full_screen_desc = {};
    full_screen_desc.RefreshRate.Numerator           = 144;
    full_screen_desc.RefreshRate.Denominator         = 1;
    full_screen_desc.Windowed                        = true;

    IDXGISwapChain1* swap_chain;
    D3D_CHECK(dxgi_factory->CreateSwapChainForHwnd(device, hwnd, &swap_chain_desc,
                                                   &full_screen_desc, nullptr, &swap_chain));
    dxgi_factory->Release();

    ID3D11Texture2D* frame_buf;
    D3D_CHECK(swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&frame_buf));

    ID3D11RenderTargetView* render_target_view;
    D3D_CHECK(device->CreateRenderTargetView(frame_buf, nullptr, &render_target_view));

    D3D11_TEXTURE2D_DESC depth_buf_desc;
    frame_buf->GetDesc(&depth_buf_desc);  // base on framebuffer properties
    depth_buf_desc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depth_buf_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depth_buf;
    D3D_CHECK(device->CreateTexture2D(&depth_buf_desc, nullptr, &depth_buf));

    ID3D11DepthStencilView* depth_buf_view;
    D3D_CHECK(device->CreateDepthStencilView(depth_buf, nullptr, &depth_buf_view));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd = { .BlendEnable           = true,
                                            .SrcBlend              = D3D11_BLEND_SRC_ALPHA,
                                            .DestBlend             = D3D11_BLEND_INV_SRC_ALPHA,
                                            .BlendOp               = D3D11_BLEND_OP_ADD,
                                            .SrcBlendAlpha         = D3D11_BLEND_SRC_ALPHA,
                                            .DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA,
                                            .BlendOpAlpha          = D3D11_BLEND_OP_ADD,
                                            .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL };

    D3D11_BLEND_DESC blend_desc = {};
    blend_desc.RenderTarget[0]  = rtbd;
    D3D_CHECK(device->CreateBlendState(&blend_desc, &gfx->blend_state));

    float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    UINT sample_mask      = 0xffffffff;
    device_context->OMSetBlendState(gfx->blend_state, blend_factor, sample_mask);

    D3D11_RASTERIZER_DESC1 rasterizer_desc { .FillMode = D3D11_FILL_SOLID,
                                             .CullMode = D3D11_CULL_BACK };

    ID3D11RasterizerState1* rasterizer_state;
    D3D_CHECK(device->CreateRasterizerState1(&rasterizer_desc, &rasterizer_state));

    D3D11_SAMPLER_DESC sampler_desc { .Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT,
                                      .AddressU       = D3D11_TEXTURE_ADDRESS_WRAP,
                                      .AddressV       = D3D11_TEXTURE_ADDRESS_WRAP,
                                      .AddressW       = D3D11_TEXTURE_ADDRESS_WRAP,
                                      .ComparisonFunc = D3D11_COMPARISON_NEVER };

    ID3D11SamplerState* sampler_state;
    D3D_CHECK(device->CreateSamplerState(&sampler_desc, &sampler_state));
    gfx->sampler_state = sampler_state;

    D3D11_DEPTH_STENCIL_DESC depth_stencil_desc { .DepthEnable    = TRUE,
                                                  .DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
                                                  .DepthFunc      = D3D11_COMPARISON_LESS,
                                                  .StencilEnable  = FALSE };

    ID3D11DepthStencilState* depth_stencil_state;
    D3D_CHECK(device->CreateDepthStencilState(&depth_stencil_desc, &depth_stencil_state));

    gfx->device              = device;
    gfx->context             = device_context;
    gfx->swap_chain          = swap_chain;
    gfx->render_target_view  = render_target_view;
    gfx->depth_buf_view      = depth_buf_view;
    gfx->rasterizer_state    = rasterizer_state;
    gfx->depth_stencil_state = depth_stencil_state;
}

}  // namespace tom