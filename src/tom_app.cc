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

global v3f g_start_pos = { -1.0f, 0.52f, 0.0f };

struct TextAtrribs
{
    f32 scale;
    f32 x_step;
    f32 y_step;
    u32 line_len;
};

TextAtrribs g_text_attribs;
global constexpr f32 g_scale_inc = 0.002f;
global f32 g_key_repeat_timer    = 0.0f;

global constexpr szt g_text_buf_sz = 10000;
global char* g_text_buf            = (char*)plat_malloc(g_text_buf_sz);
global u32 g_text_i                = 0;

global Win32Key g_skip_keys[] = { Win32Key::left_shift, Win32Key::left_alt, Win32Key::back,
                                  Win32Key::add,        Win32Key::subtract, Win32Key::left,
                                  Win32Key::up,         Win32Key::right,    Win32Key::down,
                                  Win32Key::f1,         Win32Key::f2,       Win32Key::f3,
                                  Win32Key::f4,         Win32Key::f5,       Win32Key::f6,
                                  Win32Key::f7,         Win32Key::f8,       Win32Key::f9,
                                  Win32Key::f10,        Win32Key::f11,      Win32Key::f12 };

struct ConstBuf
{
    m4 proj;
    m4 model;
    v4f text_col;
    v2f tex_sz;
    v2f uv_off;
    v2f i_cnt;
    f32 uv_r;
    i32 glyph_all;
};

function TextAtrribs set_text_attribs_from_scale(f32 scale)
{
    TextAtrribs result;
    result.scale    = scale;
    result.x_step   = result.scale / 2.0f;
    result.y_step   = result.scale / 1.25f;
    result.line_len = u32(2.0f / result.x_step);

    return result;
}

function void on_resize(AppState* state)
{
    f32 aspect = (f32)state->win32.win_dims.x1 / (f32)state->win32.win_dims.y1;
    // state->proj = mat_proj_persp(aspect, state->fov, 1.0f, 1000.0f);
    d3d_on_resize(&state->gfx, state->win32.win_dims);
    state->proj = mat_proj_ortho(aspect);
}

function void app_init(AppState* state)
{
    auto gfx = &state->gfx;

    state->fov = 1.0f;
    // state->clear_color      = { 0.086f, 0.086f, 0.086f, 1.0f };
    state->clear_color      = { 0.047f, 0.047f, 0.047f, 1.0f };
    state->text_color      = { 0.686f, 0.686f, 0.686f, 1.0f };
    state->vars.unit        = 1.0f;
    state->view             = mat_identity();
    state->key_repeat_delay = 0.2f;
    state->key_repeat_speed = 0.02f;

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

    d3d_Check(gfx->device->CreateInputLayout(
        input_desc, CountOf(input_desc), state->main_shader.vs_blob->GetBufferPointer(),
        state->main_shader.vs_blob->GetBufferSize(), &g_input_layout));

    D3D11_BUFFER_DESC vert_buf_desc         = { .ByteWidth = sizeof(verts.e),
                                                .Usage     = D3D11_USAGE_DEFAULT,
                                                .BindFlags = D3D11_BIND_VERTEX_BUFFER };
    D3D11_SUBRESOURCE_DATA vert_subrsc_data = { .pSysMem = verts.e };

    d3d_Check(gfx->device->CreateBuffer(&vert_buf_desc, &vert_subrsc_data, &g_vert_buf));

    D3D11_BUFFER_DESC ind_buf_desc         = { .ByteWidth = sizeof(inds),
                                               .Usage     = D3D11_USAGE_DEFAULT,
                                               .BindFlags = D3D11_BIND_INDEX_BUFFER };
    D3D11_SUBRESOURCE_DATA ind_subrsc_data = { .pSysMem = inds };

    d3d_Check(gfx->device->CreateBuffer(&ind_buf_desc, &ind_subrsc_data, &g_ind_buf));
    g_ind_cnt = CountOf(inds);

#define BUILD_FONT_TABLE 1

#if BUILD_FONT_TABLE
    const char* font_path2 = "./fonts/Hack-Regular.ttf";
    state->font_sheet      = create_font_sheet("Hack", font_path2, 64.0f);
    char* font_glyph_table = write_fontsheet_png(&state->font_sheet);
#else
    const char* font_glyph_table = "./out/Hack_glyph_table.png";
#endif

    // auto font_sheet        = create_font_sheet_all("Hack_all", font_path2, 64.0f);
    // char* font_glyph_table2 = write_fontsheet_png(&font_sheet);
    // plat_free(font_sheet.bitmap);

    stbi_set_flip_vertically_on_load(true);
    i32 width, height, n_channels;
    byt* data = stbi_load(font_glyph_table, &width, &height, &n_channels, 0);
    if (data) {
        D3D11_TEXTURE2D_DESC tex_desc = { .Width      = (u32)width,
                                          .Height     = (u32)height,
                                          .MipLevels  = 1,
                                          .ArraySize  = 1,
                                          .Format     = DXGI_FORMAT_R8_UNORM,
                                          .SampleDesc = { .Count = 1, .Quality = 0 },
                                          .Usage      = D3D11_USAGE_IMMUTABLE,
                                          .BindFlags  = D3D11_BIND_SHADER_RESOURCE };

        D3D11_SUBRESOURCE_DATA tex_subrsc_data = { .pSysMem          = (void*)data,
                                                   .SysMemPitch      = width * sizeof(byt),
                                                   .SysMemSlicePitch = 0 };
        d3d_Check(gfx->device->CreateTexture2D(&tex_desc, &tex_subrsc_data, &g_tex));
    } else {
        printf("ERROR-> failed to load %s!\n", font_glyph_table);
        InvalidCodePath;
    }

    d3d_Check(gfx->device->CreateShaderResourceView(g_tex, nullptr, &g_sha_rsc_view));

    // NOTE: cosntant buffers must be 16 byte aligned!!!
    D3D11_BUFFER_DESC const_buf_desc = { .ByteWidth =
                                             sizeof(ConstBuf) + (16 - (sizeof(ConstBuf) % 16)),
                                         .Usage          = D3D11_USAGE_DYNAMIC,
                                         .BindFlags      = D3D11_BIND_CONSTANT_BUFFER,
                                         .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE };
    d3d_Check(gfx->device->CreateBuffer(&const_buf_desc, 0, &g_const_buf));

    g_text_attribs = set_text_attribs_from_scale(0.035f);
}

function void app_update(AppState* state)
{
    auto gfx             = &state->gfx;
    auto kb              = &state->input.keyboard;
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

    gfx->context->OMSetRenderTargets(1, &gfx->render_target_view, gfx->depth_buf_view);
    gfx->context->ClearRenderTargetView(gfx->render_target_view, state->clear_color.e);
    gfx->context->ClearDepthStencilView(gfx->depth_buf_view,
                                        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    g_key_repeat_timer += state->dt;

    for (u32 i = 0; i < Keyboard::key_cnt; ++i) {
        auto key = kb->keys[i];

        for (auto k : g_skip_keys) {
            if (key.name == k) goto cnt;
        }
        if (key_pressed(key)) {
            char c = '\0';
            if (key_down(kb->left_shift)) {
                c = win32key_to_char_mod(key.name);
            } else {
                c = win32key_to_char(key.name);
            }
            if (c) g_text_buf[g_text_i] = c;
            if (g_text_i < g_text_buf_sz) ++g_text_i;
        }

        {  // NOTE: lol so gotos can't skip variable declaratiosn even if YOU DON'T FUCKING USE IT
           // AFTER THE GOTO
            f32 tc = state->dt * (f32)key.half_transition_cnt;
            if (tc > state->key_repeat_delay && g_key_repeat_timer > state->key_repeat_speed) {
                char c = '\0';
                if (key_down(kb->left_shift)) {
                    c = win32key_to_char_mod(key.name);
                } else {
                    c = win32key_to_char(key.name);
                }
                if (c) g_text_buf[g_text_i] = c;
                if (g_text_i < g_text_buf_sz) ++g_text_i;
                g_key_repeat_timer = 0.0f;
            }
        }
    cnt:;
    }

    if (key_pressed(kb->f1)) {
        auto file_result = read_file("./data/alice29.txt");
        char* ptr        = (char*)file_result.contents;
        for (szt i = 0; i < 3000; ++i) {
            g_text_buf[g_text_i] = ptr[i];
            if (g_text_i < g_text_buf_sz) ++g_text_i;
        }
    }

    if (key_pressed(kb->back) && g_text_i > 0) --g_text_i;
    f32 tc = state->dt * (f32)state->input.keyboard.back.half_transition_cnt;
    if (tc > state->key_repeat_delay && g_key_repeat_timer > state->key_repeat_speed &&
        g_text_i > 0) {
        --g_text_i;
        g_key_repeat_timer = 0.0f;
    }

    if (key_down(kb->add)) {
        g_text_attribs = set_text_attribs_from_scale(g_text_attribs.scale + g_scale_inc);
    } else if (key_down(kb->subtract)) {
        g_text_attribs = set_text_attribs_from_scale(g_text_attribs.scale - g_scale_inc);
    }

    i32 glyph_ind = 0;
    m4 model      = mat_identity();
    model         = mat_scale(model, g_text_attribs.scale);
    // model         = mat_rot_z(model, to_radian(180));
    model = mat_set_translation(model, g_start_pos);

    if (key_down(kb->f2)) state->view = mat_identity();

    if (key_down(kb->up)) {
        if (key_down(kb->left_shift))
            state->view = mat_rot_x(state->view, -0.01f);
        else
            state->view = mat_translate_y(state->view, -0.01f);
    } else if (key_down(kb->down)) {
        if (key_down(kb->left_shift))
            state->view = mat_rot_x(state->view, 0.01f);
        else
            state->view = mat_translate_y(state->view, 0.01f);
    }

    if (key_down(kb->left))
        state->view = mat_translate_x(state->view, 0.01f);
    else if (key_down(kb->right))
        state->view = mat_translate_x(state->view, -0.01f);

    state->wvp = state->view * state->proj;

    for (szt i = 0; i < g_text_i; ++i) {
        glyph_ind = get_glyph_index(g_text_buf[i]);
        if (g_text_buf[i] == '\n') {
            model = mat_set_translation_x(model, g_start_pos.x);
            model = mat_translate_y(model, -g_text_attribs.y_step);
            continue;
        }
        ConstBuf mapped_buf = {
            .proj     = state->wvp,
            .model    = model,
            .text_col = state->text_color,
            .tex_sz   = { (f32)state->font_sheet.width, (f32)state->font_sheet.height },
            .uv_off   = get_uv_offset(&state->font_sheet, glyph_ind),
            // .uv_off    = {0.0f, (f32)state->font_sheet.height - (f32)state->font_sheet.r},
            .i_cnt     = { (f32)state->font_sheet.x_cnt, (f32)state->font_sheet.y_cnt },
            .uv_r      = (f32)state->font_sheet.r,
            .glyph_all = false
        };
        D3D11_MAPPED_SUBRESOURCE mapped_resrc;
        d3d_Check(gfx->context->Map(g_const_buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resrc));
        memcpy(mapped_resrc.pData, &mapped_buf, sizeof(ConstBuf));
        gfx->context->Unmap(g_const_buf, 0);

        gfx->context->DrawIndexed(g_ind_cnt, 0, 0);

        model = mat_translate_x(model, g_text_attribs.x_step);
        if ((i + 1) % g_text_attribs.line_len == 0) {
            model = mat_set_translation_x(model, g_start_pos.x);
            model = mat_translate_y(model, -g_text_attribs.y_step);
        }
    }

    d3d_Check(gfx->swap_chain->Present(1, 0));

#ifdef TOM_INTERNAL
    if (key_pressed(kb->f3)) d3d_print_info_queue(gfx);
#endif
}  // namespace tom

function i32 app_start(HINSTANCE hinst)
{
    const TCHAR* icon_path = _T(".\\data\\tomato.ico");
    auto icon              = (HICON)(LoadImage(NULL, icon_path, IMAGE_ICON, 0, 0,
                                               LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED));

    create_console();
    auto cons_hwnd = GetConsoleWindow();
    Assert(cons_hwnd);
    SendMessage(cons_hwnd, WM_SETICON, NULL, (LPARAM)icon);
    SetConsoleColor(FG_WHITE);

    printf("Starting...\n");

#if _CPPUWIND
    printf("Exceptions are enabled!\n");
#endif

    AppState state                      = {};
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

    state.win32.win_dims.x1 = 1600;
    state.win32.win_dims.y1 = 900;

#ifdef TOM_INTERNAL
    LPVOID base_address = (LPVOID)Terabytes((u64)2);
#else
    LPVOID base_address          = 0;
#endif

    state.memory.permanent_storage_size = Megabytes(256);
    state.memory.transient_storage_size = Gigabytes(256);
    state.total_size = state.memory.permanent_storage_size + state.memory.transient_storage_size;
    // TODO: use large pages
    state.memory_block =
        VirtualAlloc(base_address, state.total_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    state.memory.permanent_storage = state.memory_block;
    state.memory.transient_storage =
        (u8*)state.memory.permanent_storage + state.memory.permanent_storage_size;

    prevent_windows_DPI_scaling();
    create_window(&state.win32);
    state.dpi = (u32)GetDpiForWindow(state.win32.hwnd);
    SetCursorPos(state.win32.win_dims.x1 / 2, state.win32.win_dims.y1 / 2);

    d3d_init(state.win32.hwnd, &state.gfx);

    state.gfx.viewport          = {};
    state.gfx.viewport.Width    = (f32)state.win32.win_dims.x1;
    state.gfx.viewport.Height   = (f32)state.win32.win_dims.y1;
    state.gfx.viewport.TopLeftX = 0.0f;
    state.gfx.viewport.TopLeftY = 0.0f;
    state.gfx.viewport.MinDepth = 0.0f;
    state.gfx.viewport.MaxDepth = 1.0f;

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
        for (u32 i = 0; i < CountOf(state.work_secs); ++i) {
            work_secs_avg += (f32)state.work_secs[i];
        }
        work_secs_avg /= (f32)CountOf(state.work_secs);

        // clock stuffs
        auto work_counter = get_time();
        f32 work_seconds_elapsed =
            get_seconds_elapsed(last_counter, work_counter, state.performance_counter_frequency);
        state.work_secs[state.work_ind++] = work_seconds_elapsed;
        if (state.work_ind == CountOf(state.work_secs)) state.work_ind = 0;

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
            PrintWarning("Missed frame timing!");
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