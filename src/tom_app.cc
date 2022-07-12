#include "tom_core.hh"
#include "tom_file_io.cc"
#include "tom_graphics.cc"
#include "tom_camera.cc"
#include "tom_input.cc"
#include "tom_sound.cc"
#include "tom_win32.cc"
#include "tom_font.cc"

namespace tom
{

global ID3D11InputLayout* g_input_layout;
global ID3D11Buffer* g_vert_buf;
global ID3D11Buffer* g_ind_buf;
global ID3D11Buffer* g_const_buf;
global ID3D11Texture2D* g_tex;
global ID3D11ShaderResourceView* g_sha_rsc_view;
global szt g_ind_cnt;

global f32 g_text_scale = 0.075f;
global v3 g_start_pos   = { -1.0f, 0.52f, 0.0f };
global f32 g_x_step     = 0.055f;
global f32 g_y_step     = 0.06f;
global u32 g_line_len   = 35;

global char g_text_buf[512] = {};
global u32 g_text_i         = 0;

struct ConstBuf
{
    m4 proj;
    m4 model;
    v4 text_col;
    v2 uv_off;
    i32 glyph_all;
};

function void on_resize(AppState* state)
{
    f32 aspect = (f32)state->win32.win_dims.width / (f32)state->win32.win_dims.height;
    // state->proj = mat_proj_persp(aspect, state->fov, 1.0f, 1000.0f);
    state->proj = mat_proj_ortho(aspect);
}

function void app_init(AppState* state)
{
    auto gfx = &state->gfx;

    state->fov         = 1.0f;
    state->clear_color = { 0.086f, 0.086f, 0.086f, 1.0f };
    state->text_color  = { 0.627f, 0.521f, 0.388f, 1.0f };
    state->vars.unit   = 1.0f;
    state->view        = mat_identity();

    state->input = init_input();

    state->main_shader = d3d_create_shader_prog(gfx, L".\\shaders\\main.hlsl");

    Quad verts = {};

    verts.e[0].pos = { -0.5f, -0.5f, 0.5f, 1.0f };
    verts.e[1].pos = { -0.5f, 0.5f, 0.5f, 1.0f };
    verts.e[2].pos = { 0.5f, 0.5f, 0.5f, 1.0f };
    verts.e[3].pos = { 0.5f, -0.5f, 0.5f, 1.0f };

    verts.e[0].uv = { 0.0f, 0.0f };
    verts.e[1].uv = { 0.0f, 1.0f };
    verts.e[2].uv = { 1.0f, 1.0f };
    verts.e[3].uv = { 1.0f, 0.0f };

    u16 inds[] = { 0, 1, 2, 0, 2, 3 };

    D3D11_INPUT_ELEMENT_DESC input_desc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, pos),
          D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv),
          D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    D3D_CHECK(gfx->device->CreateInputLayout(
        input_desc, _countof(input_desc), state->main_shader.vs_blob->GetBufferPointer(),
        state->main_shader.vs_blob->GetBufferSize(), &g_input_layout));

    D3D11_BUFFER_DESC vert_buf_desc         = { .ByteWidth = sizeof(verts.e),
                                                .Usage     = D3D11_USAGE_DEFAULT,
                                                .BindFlags = D3D11_BIND_VERTEX_BUFFER };
    D3D11_SUBRESOURCE_DATA vert_subrsc_data = { .pSysMem = verts.e };

    D3D_CHECK(gfx->device->CreateBuffer(&vert_buf_desc, &vert_subrsc_data, &g_vert_buf));

    D3D11_BUFFER_DESC ind_buf_desc         = { .ByteWidth = sizeof(inds),
                                               .Usage     = D3D11_USAGE_DEFAULT,
                                               .BindFlags = D3D11_BIND_INDEX_BUFFER };
    D3D11_SUBRESOURCE_DATA ind_subrsc_data = { .pSysMem = inds };

    D3D_CHECK(gfx->device->CreateBuffer(&ind_buf_desc, &ind_subrsc_data, &g_ind_buf));
    g_ind_cnt = _countof(inds);

    i32 width, height, n_channels;
    const char* font_path = "./fonts/liberation-mono.ttf";
    // stbi_set_flip_vertically_on_load(true);
    auto ttf_file = load_ttf_glyph(font_path, 64.0f, 'a');
    if (ttf_file) {
        D3D11_TEXTURE2D_DESC tex_desc = { .Width      = (u32)ttf_file.width,
                                          .Height     = (u32)ttf_file.height,
                                          .MipLevels  = 1,
                                          .ArraySize  = 1,
                                          .Format     = DXGI_FORMAT_R8_UNORM,
                                          .SampleDesc = { .Count = 1, .Quality = 0 },
                                          .Usage      = D3D11_USAGE_IMMUTABLE,
                                          .BindFlags  = D3D11_BIND_SHADER_RESOURCE };

        D3D11_SUBRESOURCE_DATA tex_subrsc_data = { .pSysMem          = (void*)ttf_file.bitmap,
                                                   .SysMemPitch      = ttf_file.width * sizeof(u8),
                                                   .SysMemSlicePitch = 0 };

        D3D_CHECK(gfx->device->CreateTexture2D(&tex_desc, &tex_subrsc_data, &g_tex));

    } else {
        printf("ERROR-> Failed to load %s!", font_path);
        TOM_INVALID_CODE_PATH;
    }
    ttf_file.free();

    const char *font_path2 = "./fonts/Hack-Regular.ttf";
    state->font_sheet = create_font_sheet(font_path2, 128.0f);

    D3D_CHECK(gfx->device->CreateShaderResourceView(g_tex, nullptr, &g_sha_rsc_view));

    // NOTE: cosntant buffers must be 16 byte aligned!!!
    D3D11_BUFFER_DESC const_buf_desc = { .ByteWidth =
                                             sizeof(ConstBuf) + (16 - (sizeof(ConstBuf) % 16)),
                                         .Usage          = D3D11_USAGE_DYNAMIC,
                                         .BindFlags      = D3D11_BIND_CONSTANT_BUFFER,
                                         .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE };
    D3D_CHECK(gfx->device->CreateBuffer(&const_buf_desc, 0, &g_const_buf));
}

function void app_update(AppState* state)
{
    auto gfx             = &state->gfx;
    local u32 stride     = sizeof(Vertex);
    local u32 offset     = 0;
    local bool once_only = false;
    if (!once_only) {
        gfx->context->IASetVertexBuffers(0, 1, &g_vert_buf, &stride, &offset);
        gfx->context->IASetInputLayout(g_input_layout);
        gfx->context->IASetIndexBuffer(g_ind_buf, DXGI_FORMAT_R16_UINT, 0);
        gfx->context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        gfx->context->RSSetState(gfx->rasterizer_state);
        gfx->context->RSSetViewports(1, &gfx->viewport);

        gfx->context->VSSetShader(state->main_shader.vs, nullptr, 0);
        gfx->context->VSSetConstantBuffers(0, 1, &g_const_buf);
        gfx->context->PSSetShader(state->main_shader.ps, nullptr, 0);

        gfx->context->PSSetShaderResources(0, 1, &g_sha_rsc_view);
        gfx->context->PSSetSamplers(0, 1, &gfx->sampler_state);

        gfx->context->OMSetRenderTargets(1, &gfx->render_target_view, gfx->depth_buf_view);
        gfx->context->OMSetDepthStencilState(gfx->depth_stencil_state, 1);

        once_only = true;
    }

    gfx->context->ClearRenderTargetView(gfx->render_target_view, state->clear_color.e);
    gfx->context->ClearDepthStencilView(gfx->depth_buf_view,
                                        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    for (u32 i = 0; i < Keyboard::key_cnt; ++i) {
        auto key = state->input.keyboard.keys[i];
        if (key_pressed(key)) {
            if (key.name == Win32Keys::left_shift || key.name == Win32Keys::back ||
                key.name == Win32Keys::add || key.name == Win32Keys::subtract)
                continue;
            if (key_down(state->input.keyboard.left_shift)) {
                char c = win32key_to_char_mod(key.name);
                if (c) g_text_buf[g_text_i++] = c;
            } else {
                char c = win32key_to_char(key.name);
                if (c) g_text_buf[g_text_i++] = c;
            }

            if (g_text_i == _countof(g_text_buf)) g_text_i = 0;
        }
    }

    if (key_pressed(state->input.keyboard.back)) {
        if (g_text_i != 0) --g_text_i;
    }

    local constexpr f32 scale_inc = 0.005f;
    if (key_pressed(state->input.keyboard.add)) {
        g_text_scale += scale_inc;
    } else if (key_pressed(state->input.keyboard.subtract)) {
        g_text_scale -= scale_inc;
    }

    i32 glyph_ind = 0;
    m4 model      = mat_identity();
    model         = mat_scale(model, g_text_scale);
    model         = mat_set_translation(model, g_start_pos);

    for (u32 i = 0; i < g_text_i; ++i) {
        glyph_ind           = get_glyph_index(g_text_buf[i]);
        ConstBuf mapped_buf = { .proj      = state->proj,
                                .model     = model,
                                .text_col  = state->text_color,
                                .uv_off    = {},
                                .glyph_all = true };
        D3D11_MAPPED_SUBRESOURCE mapped_resrc;
        D3D_CHECK(gfx->context->Map(g_const_buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resrc));
        memcpy(mapped_resrc.pData, &mapped_buf, sizeof(ConstBuf));
        gfx->context->Unmap(g_const_buf, 0);

        gfx->context->DrawIndexed(g_ind_cnt, 0, 0);

        model = mat_translate_x(model, g_x_step);
        if ((i + 1) % g_line_len == 0) {
            model = mat_set_translation_x(model, g_start_pos.x);
            model = mat_translate_y(model, -g_y_step);
        }
    }

    D3D_CHECK(gfx->swap_chain->Present(1, 0));

#ifdef TOM_INTERNAL
    d3d_print_info_queue(gfx);
#endif
}  // namespace tom

function i32 app_start(HINSTANCE hinst)
{
    const TCHAR* icon_path = _T(".\\data\\tomato.ico");
    auto icon              = (HICON)(LoadImage(NULL, icon_path, IMAGE_ICON, 0, 0,
                                               LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED));

    create_console();
    auto cons_hwnd = GetConsoleWindow();
    TOM_ASSERT(cons_hwnd);
    SendMessage(cons_hwnd, WM_SETICON, NULL, (LPARAM)icon);

    printf("Starting...\n");

#if _CPPUWIND
    printf("Exceptions are enabled!\n");
#endif

    AppState state {};
    state.game_update_hertz             = 60;
    state.target_frames_per_second      = 1.0f / (f32)state.game_update_hertz;
    state.target_fps                    = 60;
    state.sound.frames_of_audio_latency = (1.1f / 30.f) * (f32)state.game_update_hertz;
    state.win32.icon                    = icon;

    DWORD exe_path_len = GetModuleFileNameA(NULL, state.exe_path, sizeof(state.exe_path));
    printf("exe path %s\n", state.exe_path);

    TCHAR cwd_buf[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, cwd_buf);
    // _tprintf(TEXT("cwd %s\n"), cwd_buf);

    char cwd[MAX_PATH];
    get_cwd(cwd);
    printf("cwd: %s\n", cwd);

    char* p_ = &state.exe_path[exe_path_len];
    i32 i_   = (i32)exe_path_len;
    while (i_ > -1 && state.exe_path[i_] != '\\') {
        --i_;
    }

    TCHAR set_cwd_buf[MAX_PATH];
    for (int i = 0; i < i_; ++i) {
        set_cwd_buf[i] = state.exe_path[i];
    }
    set_cwd_buf[i_] = '\0';

    bool cwd_is_exe = true;
    int it_buf      = 0;
    while (cwd_buf[it_buf]) {
        if (cwd_buf[it_buf] != set_cwd_buf[it_buf]) cwd_is_exe = false;
        ++it_buf;
    }

    if (!str_equal(cwd_buf, set_cwd_buf)) {
        printf("cwd is not exe dir!\n");
        if (!SetCurrentDirectory(set_cwd_buf)) {
            printf("Failed to set cwd!");
        } else {
            GetCurrentDirectory(MAX_PATH, cwd_buf);
            _tprintf(TEXT("set cwd to %s\n"), cwd_buf);
        }
    }

    LARGE_INTEGER performance_query_result;
    QueryPerformanceFrequency(&performance_query_result);
    state.performance_counter_frequency = performance_query_result.QuadPart;

    state.win32.win_dims.width  = 1600;
    state.win32.win_dims.height = 900;

#ifdef TOM_INTERNAL
    LPVOID base_address = (LPVOID)TERABYTES((u64)2);
#else
    LPVOID base_address = 0;
#endif

    state.memory.permanent_storage_size = MEGABYTES(256);
    state.memory.transient_storage_size = GIGABYTES(1);
    state.total_size = state.memory.permanent_storage_size + state.memory.transient_storage_size;
    // TODO: use large pages
    state.memory_block =
        VirtualAlloc(base_address, state.total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    state.memory.permanent_storage = state.memory_block;
    state.memory.transient_storage =
        (u8*)state.memory.permanent_storage + state.memory.permanent_storage_size;

    create_window(&state.win32);
    state.dpi = (u32)GetDpiForWindow(state.win32.hwnd);
    SetCursorPos(state.win32.win_dims.width / 2, state.win32.win_dims.height / 2);

    d3d_init(state.win32.hwnd, &state.gfx);

    state.gfx.viewport          = {};
    state.gfx.viewport.Width    = state.win32.win_dims.width;
    state.gfx.viewport.Height   = state.win32.win_dims.height;
    state.gfx.viewport.TopLeftX = 0.0f;
    state.gfx.viewport.TopLeftY = 0.0f;
    state.gfx.viewport.MinDepth = 0.0f;
    state.gfx.viewport.MaxDepth = 0.0f;

    i64 last_counter     = get_time();
    u64 last_cycle_count = __rdtsc();

    f32 delta_time = 0.0f;
    // NOTE: dummy thread context, for now
    ThreadContext thread {};

    state.win32.running = true;

    app_init(&state);

    while (true) {
        ++state.frame_cnt;
        // printf("%llu\n",state.frame_cnt);
        if (!state.win32.running) break;
        if (state.win32.resize) {
            on_resize(&state);
            state.win32.resize = false;
        }

        state.target_frames_per_second = 1.0f / (f32)state.target_fps;

        state.win32.ms_scroll = 0;
        process_pending_messages(&state.win32);
        // do_controller_input(*old_input, *new_input, hwnd);
        // NOTE: this isn't calculated and needs to be for a variable framerate
        // state.dt            = state.target_frames_per_second;
        state.dt            = state.ms_frame / 1000.0f;
        local u64 frame_cnt = 0;
        local f32 one_sec   = 0.0f;
        ++frame_cnt;
        one_sec += state.ms_frame;
        if (one_sec > 1000.0f) {
            one_sec -= 1000.0f;
            state.fps = (i32)frame_cnt;
            frame_cnt = 0;
        }

        do_input(&state.input, state.win32.hwnd, state.win32.ms_scroll);

        app_update(&state);

        f32 work_secs_avg = 0.0f;
        for (u32 i = 0; i < ARRAY_COUNT(state.work_secs); ++i) {
            work_secs_avg += (f32)state.work_secs[i];
        }
        work_secs_avg /= (f32)ARRAY_COUNT(state.work_secs);

        // clock stuffs
        auto work_counter = get_time();
        f32 work_seconds_elapsed =
            get_seconds_elapsed(last_counter, work_counter, state.performance_counter_frequency);
        state.work_secs[state.work_ind++] = work_seconds_elapsed;
        if (state.work_ind == ARRAY_COUNT(state.work_secs)) state.work_ind = 0;

        bool is_sleep_granular        = false;
        f32 seconds_elapsed_for_frame = work_seconds_elapsed;
        if (seconds_elapsed_for_frame < state.target_frames_per_second) {
            if (is_sleep_granular) {
                auto sleepMs =
                    (DWORD)(1000.f * (state.target_frames_per_second - seconds_elapsed_for_frame));
                if (sleepMs > 0) {
                    ::Sleep(sleepMs);
                }
            }
            f32 test_seconds_elapsed_for_frame =
                get_seconds_elapsed(last_counter, get_time(), state.performance_counter_frequency);
            while (seconds_elapsed_for_frame < state.target_frames_per_second) {
                seconds_elapsed_for_frame = get_seconds_elapsed(
                    last_counter, get_time(), state.performance_counter_frequency);
            }
        } else {
            printf("WARNING--> missed frame timing!!!\n");
        }

        auto end_counter = get_time();
        state.ms_frame   = 1000.f * get_seconds_elapsed(last_counter, end_counter,
                                                        state.performance_counter_frequency);

        last_counter = end_counter;

        u64 end_cycle_count = __rdtsc();
        u64 cycles_elapsed  = end_cycle_count - last_cycle_count;
        last_cycle_count    = end_cycle_count;
    }

    ReleaseDC(state.win32.hwnd, state.win32.hdc);
    DestroyWindow(state.win32.hwnd);
    UnregisterClass(state.win32.cls_name, NULL);

    return 0;
}

}  // namespace tom