using System;
using CSharpScript;

namespace CSharpScript
{
    /// <summary>
    /// Unity-style parallax background script.
    /// Attach to the player. Set Layer names to your background GameObjects.
    /// Each layer scrolls at its own rate relative to the camera.
    ///
    /// Factor: 0.0 = static (infinitely far), 1.0 = moves with camera
    /// </summary>
    public class ParallaxBackground : GameScript
    {
        public string Layer0 = "ParallaxBG";
        public string Layer1 = "";
        public string Layer2 = "";
        public string Layer3 = "";

        public float Factor0 = 1.0f;
        public float Factor1 = 0.3f;
        public float Factor2 = 0.5f;
        public float Factor3 = 0.8f;

        private vec2 startPos0, startPos1, startPos2, startPos3;
        private bool init0, init1, init2, init3;

        public override void OnStart()
        {
            if (Layer0 != "")
            {
                Transform.SetPosition(Layer0, 0, 430);
                Transform.SetScale(Layer0, 3100, 4320);
                startPos0 = Transform.GetPosition(Layer0);
                init0 = true;
            }
            if (Layer1 != "")
            {
                startPos1 = Transform.GetPosition(Layer1);
                init1 = true;
            }
            if (Layer2 != "")
            {
                startPos2 = Transform.GetPosition(Layer2);
                init2 = true;
            }
            if (Layer3 != "")
            {
                startPos3 = Transform.GetPosition(Layer3);
                init3 = true;
            }
        }

        public override void OnUpdate(float dt)
        {
            vec2 camPos = Camera.GetPosition();

            if (init0)
            {
                Transform.SetPosition(Layer0,
                    startPos0.x + camPos.x * Factor0,
                    startPos0.y + camPos.y * Factor0);
            }
            if (init1)
            {
                Transform.SetPosition(Layer1,
                    startPos1.x + camPos.x * Factor1,
                    startPos1.y + camPos.y * Factor1);
            }
            if (init2)
            {
                Transform.SetPosition(Layer2,
                    startPos2.x + camPos.x * Factor2,
                    startPos2.y + camPos.y * Factor2);
            }
            if (init3)
            {
                Transform.SetPosition(Layer3,
                    startPos3.x + camPos.x * Factor3,
                    startPos3.y + camPos.y * Factor3);
            }
        }

        public override void OnDestroy()
        {
        }
    }
}
