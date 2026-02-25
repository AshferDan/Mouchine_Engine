using System;
using CSharpScript;

namespace CSharpScript
{
    /// <summary>
    /// Parallax background script. Attach to the player/hero.
    /// Set LayerXxx to the name of each background GameObject.
    ///
    /// Factor:
    ///   0.0 = object stays in world space (scrolls fastest on screen)
    ///   0.8 = subtle parallax drift
    ///   1.0 = moves exactly with camera (appears FROZEN on screen)
    ///
    /// TileWidthXxx:
    ///   0 (default) = no looping (use for single-image backgrounds)
    ///   &gt; 0          = tile width for seamless infinite looping
    /// </summary>
    public class ParallaxBackground : GameScript
    {
        public enum Layer { FarBack = 0, Back = 1, Mid = 2, Front = 3 }
        private const int LayerCount = 4;

        // --- Layer names (GameObjects to move) ---
        public string LayerFarBack = "Parallax_BG";
        public string LayerBack    = "";
        public string LayerMid     = "Pillar_BackGround";
        public string LayerFront   = "";

        // --- Scroll factors (how much of the camera movement the layer follows) ---
        public float FactorFarBack = 0.8f;   // subtle drift — far wall
        public float FactorBack    = 0.3f;
        public float FactorMid     = 0.7f;   // pillars move mostly with camera
        public float FactorFront   = 0.9f;

        // --- Tile widths for seamless looping. Leave at 0 to disable looping ---
        public float TileWidthFarBack = 0f;
        public float TileWidthBack    = 0f;
        public float TileWidthMid     = 0f;
        public float TileWidthFront   = 0f;

        private string[] layerNames;
        private float[]  factors;
        private float[]  tileWidths;
        private vec2[]   startPos = new vec2[LayerCount];
        private bool[]   inited   = new bool[LayerCount];

        public override void OnStart()
        {
            layerNames = new string[] { LayerFarBack, LayerBack, LayerMid, LayerFront };
            factors    = new float[]  { FactorFarBack, FactorBack, FactorMid, FactorFront };
            tileWidths = new float[]  { TileWidthFarBack, TileWidthBack, TileWidthMid, TileWidthFront };

            foreach (Layer layer in Enum.GetValues(typeof(Layer)))
            {
                int i = (int)layer;
                if (layerNames[i] == "") continue;

                float scaleX = Transform.GetScale(layerNames[i]).x;
                if (scaleX == 0f) continue; // object not found in scene

                startPos[i] = Transform.GetPosition(layerNames[i]);
                inited[i]   = true;
            }
        }

        private void TryInitLayer(int i)
        {
            if (layerNames == null || layerNames[i] == "") return;
            float scaleX = Transform.GetScale(layerNames[i]).x;
            if (scaleX == 0f) return; // object not yet in scene
            startPos[i] = Transform.GetPosition(layerNames[i]);
            inited[i]   = true;
        }

        public override void OnUpdate(float dt)
        {
            vec2 camPos = Camera.GetPosition();

            foreach (Layer layer in Enum.GetValues(typeof(Layer)))
            {
                int i = (int)layer;

                // Lazy init: try each frame until the object appears in the scene
                if (!inited[i])
                {
                    TryInitLayer(i);
                    if (!inited[i]) continue; // still not ready
                }

                float f = factors[i];

                // Position the layer so it lags behind the camera by (1 - f)
                Transform.SetPosition(layerNames[i],
                    startPos[i].x + camPos.x * f,
                    startPos[i].y + camPos.y * f);

                // Seamless looping — only when TileWidth is set
                if (tileWidths[i] > 0f)
                {
                    float temp = camPos.x * (1f - f);
                    vec2  sp   = startPos[i];
                    if      (temp > sp.x + tileWidths[i]) sp.x += tileWidths[i];
                    else if (temp < sp.x - tileWidths[i]) sp.x -= tileWidths[i];
                    startPos[i] = sp;
                }
            }
        }

        public override void OnDestroy()
        {
        }
    }
}
