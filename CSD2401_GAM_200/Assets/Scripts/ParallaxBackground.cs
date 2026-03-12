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
    ///   0 (default) = no tiling (single image, no side copies)
    ///   > 0          = enables 3-copy seamless tiling. Duplicate the layer
    ///                  GameObject twice and name the copies:
    ///                    <LayerName>_L   (left  copy)
    ///                    <LayerName>_R   (right copy)
    ///                  Set TileWidth to the visual width of one copy.
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

        // --- Tile widths. Set > 0 to enable 3-copy seamless tiling.
        //     Side copies must be named "<LayerName>_L" and "<LayerName>_R". ---
        public float TileWidthFarBack = 0f;
        public float TileWidthBack    = 0f;
        public float TileWidthMid     = 0f;
        public float TileWidthFront   = 0f;

        private string[] layerNames;
        private string[] layerNames_L;  // "<name>_L" side copies
        private string[] layerNames_R;  // "<name>_R" side copies
        private float[]  factors;
        private vec2[]   startPos = new vec2[LayerCount];
        private bool[]   inited   = new bool[LayerCount];
        private bool[]   inited_L = new bool[LayerCount];
        private bool[]   inited_R = new bool[LayerCount];

        public override void OnStart()
        {
            layerNames = new string[] { LayerFarBack, LayerBack, LayerMid, LayerFront };
            factors    = new float[]  { FactorFarBack, FactorBack, FactorMid, FactorFront };

            layerNames_L = new string[LayerCount];
            layerNames_R = new string[LayerCount];
            for (int i = 0; i < LayerCount; i++)
            {
                layerNames_L[i] = layerNames[i] + "_L";
                layerNames_R[i] = layerNames[i] + "_R";
            }

            // Try to init layers that are already loaded at this point
            foreach (Layer layer in Enum.GetValues(typeof(Layer)))
                TryInitLayer((int)layer);
        }

        // Reads the tile width for layer i directly from the public fields
        // (avoids caching issues when OnStart runs before scene field overrides are applied).
        private float GetTileWidth(int i)
        {
            switch (i)
            {
                case (int)Layer.FarBack: return TileWidthFarBack;
                case (int)Layer.Back:    return TileWidthBack;
                case (int)Layer.Mid:     return TileWidthMid;
                case (int)Layer.Front:   return TileWidthFront;
                default:                 return 0f;
            }
        }

        // Attempts to initialize a layer (and its side copies) if the objects
        // are present in the scene. Called from OnStart and lazily from OnUpdate.
        private void TryInitLayer(int i)
        {
            if (layerNames == null || layerNames[i] == "") return;

            // Main copy
            if (!inited[i])
            {
                float scaleX = Transform.GetScale(layerNames[i]).x;
                if (scaleX != 0f)
                {
                    startPos[i] = Transform.GetPosition(layerNames[i]);
                    inited[i]   = true;
                }
            }

            // Side copies — only checked when tiling is enabled
            if (GetTileWidth(i) > 0f)
            {
                if (!inited_L[i] && Transform.GetScale(layerNames_L[i]).x != 0f)
                    inited_L[i] = true;
                if (!inited_R[i] && Transform.GetScale(layerNames_R[i]).x != 0f)
                    inited_R[i] = true;
            }
        }

        public override void OnUpdate(float dt)
        {
            // Defensive: if OnStart was skipped by the engine, run it now.
            if (layerNames == null) OnStart();

            vec2 camPos = Camera.GetPosition();

            foreach (Layer layer in Enum.GetValues(typeof(Layer)))
            {
                int i = (int)layer;

                // Lazy init: retry until the object appears in the scene
                if (!inited[i])
                {
                    TryInitLayer(i);
                    if (!inited[i]) continue; // still not ready
                }

                float f  = factors[i];
                float tw = GetTileWidth(i);
                float posY = startPos[i].y + camPos.y * f;
                float posX;

                if (tw > 0f)
                {
                    // How far the background has drifted on-screen total
                    float drift = camPos.x * (1f - f);

                    // Wrap drift into [0, tw) so the position cycles seamlessly
                    float wrapped = drift % tw;
                    if (wrapped < 0f) wrapped += tw;

                    // World position that produces the wrapped screen position
                    posX = startPos[i].x - wrapped + camPos.x;

                    // Lazy init side copies in case they loaded after OnStart
                    if (!inited_L[i] || !inited_R[i])
                        TryInitLayer(i);

                    // L and R always flank the center by exactly one tile width
                    if (inited_L[i])
                        Transform.SetPosition(layerNames_L[i], posX - tw, posY);
                    if (inited_R[i])
                        Transform.SetPosition(layerNames_R[i], posX + tw, posY);
                }
                else
                {
                    // No tiling — simple parallax drift
                    posX = startPos[i].x + camPos.x * f;
                }

                Transform.SetPosition(layerNames[i], posX, posY);
            }
        }

        public override void OnDestroy()
        {
        }
    }
}
