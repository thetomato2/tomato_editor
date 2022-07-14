namespace tom
{
struct AppInput
{
    Input* current;
    Input* last;
};

struct AppVars
{
    bool line_mode;
    f32 unit;
};

struct AppMemory
{
    u64 permanent_storage_size;
    void* permanent_storage;  // NOTE: required to be cleared to 0!
    u64 transient_storage_size;
    void* transient_storage;  // NOTE: required to be cleared to 0!
};

struct AppState
{
    AppMemory memory;
    void* memory_block;
    Win32State win32;
    GfxState gfx;
    SoundState sound;
    Input input;
    szt total_size;
    u32 game_update_hertz;
    u32 dpi;
    f32 target_frames_per_second;
    AppVars vars;
    i64 performance_counter_frequency;
    u64 frame_cnt;
    u32 work_ind;
    f32 work_secs[256];
    f32 ms_frame;
    i32 fps;
    i32 target_fps;
    char exe_path[MAX_PATH];
    f32 dt;
    f32 fov;
    f32 key_repeat_delay;
    f32 key_repeat_speed;

    m4 world;
    m4 view;
    m4 proj;
    m4 wvp;

    v4 clear_color;
    v4 text_color;
    
    ShaderProg main_shader;
    FontSheet font_sheet;
    char key_buf[256];

};
}  // namespace tom