using System;
using CSharpScript;

namespace CSharpScript
{
    /// <summary>
    /// Unity-style parallax background script.
    /// Attach this to the player (or any object). Configure layer names and factors
    /// in the public fields. Each background layer scrolls at a different rate
    /// relative to the camera, creating a depth illusion.
    ///
    /// parallaxFactor: 0.0 = static (infinitely far), 1.0 = moves with camera (same plane)
    /// </summary>
    public class ParallaxBackground : GameScript
    {
        // Background layer names - set these to your background GameObject names
        public string Layer0 = "";
        public string Layer1 = "";
        public string Layer2 = "";
        public string Layer3 = "";

        // Parallax factors for each layer (0 = no scroll, 1 = full camera speed)
        public float Factor0 = 0.1f;
        public float Factor1 = 0.3f;
        public float Factor2 = 0.5f;
        public float Factor3 = 0.8f;

        // Starting positions captured on start
        private vec2 startPos0, startPos1, startPos2, startPos3;
        private bool init0, init1, init2, init3;

        public override void OnStart()
        {
            // Capture each layer's starting position
            if (Layer0 != "")
            {
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
                float nx = startPos0.x + camPos.x * Factor0;
                float ny = startPos0.y + camPos.y * Factor0;
                Transform.SetPosition(Layer0, nx, ny);
            }
            if (init1)
            {
                float nx = startPos1.x + camPos.x * Factor1;
                float ny = startPos1.y + camPos.y * Factor1;
                Transform.SetPosition(Layer1, nx, ny);
            }
            if (init2)
            {
                float nx = startPos2.x + camPos.x * Factor2;
                float ny = startPos2.y + camPos.y * Factor2;
                Transform.SetPosition(Layer2, nx, ny);
            }
            if (init3)
            {
                float nx = startPos3.x + camPos.x * Factor3;
                float ny = startPos3.y + camPos.y * Factor3;
                Transform.SetPosition(Layer3, nx, ny);
            }
        }

        public override void OnDestroy()
        {
        }
    }
}
