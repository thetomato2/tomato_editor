namespace tom
{

enum class Cam_Move_Dir
{
    forward,
    backward,
    up,
    down,
    right,
    left,
};

struct Camera
{
    f32 speed;
    // NOTE: in open_gl coords
    v3 pos;
    f32 angle_h;
    f32 angle_v;
    v2 mouse_pos;
    v3 up;
    v3 forward;
    v3 target_pos;
};

}  // namespace tom
