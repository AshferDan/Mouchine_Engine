using CSharpScript;

namespace CSharpScript
{
    public static class InputManager
    {
        // Movement: keyboard WASD/arrows take priority; fall back to left stick + D-pad
        public static vec2 Movement
        {
            get
            {
                vec2 kb = Input.Movement();
                if (kb.x != 0f || kb.y != 0f) return kb;
                if (!Input.IsGamepadConnected()) return new vec2(0f, 0f);

                float x = Input.GetGamepadAxis((int)Input.GamepadAxis.LeftX);
                if (x < 0.2f && x > -0.2f) x = 0f;
                if (Input.GetGamepadButtonHeld(Input.GamepadButton.DpadLeft))  x = -1f;
                if (Input.GetGamepadButtonHeld(Input.GamepadButton.DpadRight)) x =  1f;
                return new vec2(x, 0f);
            }
        }

        // A button = Jump
        public static bool JumpWasPressed =>
            Input.GetKeyPressed(Input.KeyCode.Space) ||
            Input.GetGamepadButtonPressed(Input.GamepadButton.A);

        public static bool JumpWasReleased =>
            Input.GetKeyReleased(Input.KeyCode.Space) ||
            Input.GetGamepadButtonReleased(Input.GamepadButton.A);

        // X button = Fast Drop
        public static bool FastDropWasPressed =>
            Input.GetKeyHeld(Input.KeyCode.S) ||
            Input.GetGamepadButtonHeld(Input.GamepadButton.X);

        // Right Bumper = Run
        public static bool RunWasPressed =>
            Input.GetKeyPressed(Input.KeyCode.J) ||
            Input.GetGamepadButtonPressed(Input.GamepadButton.RightBumper);

        public static bool RunIsHeld =>
            Input.GetKeyHeld(Input.KeyCode.J) ||
            Input.GetGamepadButtonHeld(Input.GamepadButton.RightBumper);

        // Left Bumper = Wall Kick / Zipline
        public static bool WallKickWasPressed =>
            Input.GetKeyPressed(Input.KeyCode.L) ||
            Input.GetGamepadButtonPressed(Input.GamepadButton.LeftBumper);

        // B button = Ground Pound / Roll
        public static bool GroundPoundWasPressed =>
            Input.GetKeyPressed(Input.KeyCode.K) ||
            Input.GetGamepadButtonPressed(Input.GamepadButton.B);

        public static bool RollWasPressed =>
            Input.GetKeyPressed(Input.KeyCode.K) ||
            Input.GetGamepadButtonPressed(Input.GamepadButton.B);

        // A button (hold) = Glide  [was Y button]
        public static bool GlideIsHeld =>
            Input.GetKeyHeld(Input.KeyCode.W) || Input.GetKeyHeld(Input.KeyCode.Space) ||
            Input.GetGamepadButtonHeld(Input.GamepadButton.A);

        // Left Bumper = Zipline (same as Wall Kick)
        public static bool ZiplineWasPressed =>
            Input.GetKeyPressed(Input.KeyCode.L) ||
            Input.GetGamepadButtonPressed(Input.GamepadButton.LeftBumper);

        public static bool ZiplineIsHeld =>
            Input.GetKeyHeld(Input.KeyCode.L) ||
            Input.GetGamepadButtonHeld(Input.GamepadButton.LeftBumper);
    }
}
