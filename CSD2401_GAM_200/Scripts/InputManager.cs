using CSharpScript;

namespace GameScripts
{
    public static class InputManager
    {
        public static vec2 Movement => Input.Movement();
        public static bool JumpWasPressed => Input.GetKeyPressed(Input.KeyCode.Space);
        public static bool JumpWasReleased => Input.GetKeyReleased(Input.KeyCode.Space);
        public static bool DashWasPressed => Input.GetKeyPressed(Input.KeyCode.LeftShift);
        public static bool WallKickWasPressed => Input.GetKeyPressed(Input.KeyCode.E);
        public static bool GroundPoundWasPressed => Input.GetKeyPressed(Input.KeyCode.S);
        public static bool RollWasPressed => Input.GetKeyPressed(Input.KeyCode.C);
        public static bool GlideIsHeld => Input.GetKeyHeld(Input.KeyCode.LeftControl);
        public static bool ZiplineWasPressed => Input.GetKeyPressed(Input.KeyCode.F);
        public static bool ZiplineIsHeld => Input.GetKeyHeld(Input.KeyCode.F);
    }
}
