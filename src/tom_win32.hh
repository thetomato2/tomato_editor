namespace tom
{

struct Win32State
{
    bool running;
    bool pause;
    bool resize;
    WindowDims win_dims;
    i32 ms_scroll;
    WINDOWPLACEMENT win_pos = { sizeof(win_pos) };
    HWND hwnd;
    HINSTANCE hinst;
    HDC hdc;
    const TCHAR* cls_name;
    HICON icon;
};

}  // namespace tom