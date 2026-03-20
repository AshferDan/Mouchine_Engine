using System;
using CSharpScript;

namespace CSharpScript
{
    /// <summary>
    /// Parallax background script. Attach to any GameObject in the scene (typically the Hero/Player).
    ///
    /// HOW TO ADD THIS SCRIPT TO A NEW SCENE — follow these steps:
    ///
    ///  1. BACKGROUND OBJECTS
    ///     In the scene editor, create one Sprite/Image GameObject per background layer.
    ///     Name them exactly as you will put in the LayerXxx fields below (e.g. "BG_Far").
    ///     Position each one where it should appear on-screen at the START of the level.
    ///     Set their render layers so they draw behind the level geometry.
    ///
    ///  2. TILING (optional — skip if your background is a single large image)
    ///     If a layer should tile horizontally, duplicate the main object twice:
    ///       "<LayerName>_L"  — left  copy
    ///       "<LayerName>_R"  — right copy
    ///     Place _L and _R at exactly ±TileWidth from the centre object along X.
    ///     Set TileWidthXxx = the pixel width of ONE copy (e.g. 4200 if the sprite is 4200px wide).
    ///     All three copies must have the same Y position and scale.
    ///
    ///  3. ATTACH THE SCRIPT
    ///     Add a CSharpScript component to the Hero (or any persistent object).
    ///     Set the Script Type to "ParallaxBackground".
    ///     Fill in the LayerXxx names and adjust factors/offsets as needed.
    ///
    ///  4. FACTOR GUIDE
    ///     FactorX controls how much a layer scrolls horizontally relative to the camera.
    ///       0.0 = frozen in world-space (moves fastest across screen — use for close foreground)
    ///       0.5 = scrolls at half camera speed (medium depth)
    ///       1.0 = locked to camera (frozen on screen — use for sky / UI-like layers)
    ///     FactorY controls vertical parallax.
    ///       1.0 = locked to camera on Y (recommended — backgrounds don't drift when jumping)
    ///       0.0 = fixed in world-space on Y
    ///
    ///  5. RECOMMENDED STARTING VALUES
    ///     Far background (sky):  FactorX=0.1, FactorY=1.0, no tiling
    ///     Mid background (hills): FactorX=0.4, FactorY=1.0, tiling if wide level
    ///     Near background (trees/pillars): FactorX=0.7, FactorY=1.0, tiling
    ///     Foreground detail:     FactorX=0.9, FactorY=1.0, tiling
    /// </summary>
    public class ParallaxBackground : GameScript
    {
        private const int LayerCount = 4;

        // --- Layer GameObject names ---
        public string LayerFarBack = "Parallax_BG";
        public string LayerBack    = "";
        public string LayerMid     = "Pillar_BackGround";
        public string LayerFront   = "";

        // --- Horizontal scroll factors (0 = world-fixed, 1 = camera-locked) ---
        public float FactorFarBack = 0.15f;
        public float FactorBack    = 0.30f;
        public float FactorMid     = 0.55f;
        public float FactorFront   = 0.80f;

        // --- Vertical scroll factors (keep at 1.0 to prevent Y drift when jumping) ---
        public float FactorYFarBack = 1.0f;
        public float FactorYBack    = 1.0f;
        public float FactorYMid     = 1.0f;
        public float FactorYFront   = 1.0f;

        // --- Tile widths. Set > 0 to enable 3-copy seamless horizontal tiling.
        //     Side copies must be named "<LayerName>_L" and "<LayerName>_R". ---
        public float TileWidthFarBack = 0f;
        public float TileWidthBack    = 0f;
        public float TileWidthMid     = 0f;
        public float TileWidthFront   = 0f;

        // --- Per-layer world-space offsets applied on top of the parallax position ---
        public float OffsetXFarBack = 0f; public float OffsetYFarBack = 0f;
        public float OffsetXBack    = 0f; public float OffsetYBack    = 0f;
        public float OffsetXMid     = 0f; public float OffsetYMid     = 0f;
        public float OffsetXFront   = 0f; public float OffsetYFront   = 0f;

        // Internal state
        private string[] _names;
        private string[] _namesL;
        private string[] _namesR;
        private float[]  _factorsX;
        private float[]  _factorsY;
        private vec2[]   _startWorldPos = new vec2[LayerCount];   // world pos of bg at init
        private vec2     _startCamPos;                            // camera pos at init
        private bool[]   _inited  = new bool[LayerCount];
        private bool[]   _initedL = new bool[LayerCount];
        private bool[]   _initedR = new bool[LayerCount];

        public override void OnStart()
        {
            _names = new string[] { LayerFarBack, LayerBack, LayerMid, LayerFront };
            _factorsX = new float[] { FactorFarBack, FactorBack, FactorMid, FactorFront };
            _factorsY = new float[] { FactorYFarBack, FactorYBack, FactorYMid, FactorYFront };

            _namesL = new string[LayerCount];
            _namesR = new string[LayerCount];
            for (int i = 0; i < LayerCount; i++)
            {
                _namesL[i] = _names[i] + "_L";
                _namesR[i] = _names[i] + "_R";
            }

            // Snapshot camera position so parallax is relative to level start,
            // not world origin. This fixes backgrounds spawning offset when the
            // level camera starts away from (0,0).
            _startCamPos = Camera.GetPosition();

            for (int i = 0; i < LayerCount; i++)
                TryInitLayer(i);
        }

        public override void OnUpdate(float dt)
        {
            if (_names == null) OnStart();

            vec2 cam = Camera.GetPosition();

            // How far the camera has moved since level start
            float camDeltaX = cam.x - _startCamPos.x;
            float camDeltaY = cam.y - _startCamPos.y;

            for (int i = 0; i < LayerCount; i++)
            {
                if (!_inited[i])
                {
                    TryInitLayer(i);
                    if (!_inited[i]) continue;
                }

                float fx  = _factorsX[i];
                float fy  = _factorsY[i];
                vec2  off = GetOffset(i);
                float tw  = GetTileWidth(i);

                // Y: background moves by (camDelta * fy).
                //    fy=1.0 → locked to camera (no vertical drift when jumping).
                float posY = _startWorldPos[i].y + camDeltaY * fy + off.y;

                float posX;
                if (tw > 0f)
                {
                    // Tiling X:
                    //   drift   = how far the background has lagged behind the camera.
                    //   wrapped = drift wrapped within [0, tw) for seamless looping.
                    //   posX    = start world pos + full camera delta - drift wrap.
                    //   This is equivalent to the non-tiling formula when drift < tw,
                    //   and seamlessly repeats after each full tile-width of camera travel.
                    float drift   = camDeltaX * (1f - fx);
                    float wrapped = drift % tw;
                    if (wrapped < 0f) wrapped += tw;

                    posX = _startWorldPos[i].x + camDeltaX - wrapped + off.x;

                    // Retry side-copy init in case they loaded after OnStart
                    if (!_initedL[i] || !_initedR[i])
                        TryInitLayer(i);

                    if (_initedL[i])
                        Transform.SetPosition(_namesL[i], posX - tw, posY);
                    if (_initedR[i])
                        Transform.SetPosition(_namesR[i], posX + tw, posY);
                }
                else
                {
                    // No tiling — simple drift: background moves by (camDelta * fx).
                    // fx=0 → stays at start world pos. fx=1 → locked to camera.
                    posX = _startWorldPos[i].x + camDeltaX * fx + off.x;
                }

                Transform.SetPosition(_names[i], posX, posY);
            }
        }

        public override void OnDestroy() { }

        // -----------------------------------------------------------------------

        private void TryInitLayer(int i)
        {
            if (string.IsNullOrEmpty(_names[i])) return;

            if (!_inited[i])
            {
                vec2 s = Transform.GetScale(_names[i]);
                if (s.x != 0f || s.y != 0f)    // non-zero scale = object exists in scene
                {
                    _startWorldPos[i] = Transform.GetPosition(_names[i]);
                    _inited[i] = true;
                }
            }

            if (GetTileWidth(i) > 0f)
            {
                if (!_initedL[i] && !string.IsNullOrEmpty(_namesL[i]))
                {
                    vec2 s = Transform.GetScale(_namesL[i]);
                    if (s.x != 0f || s.y != 0f) _initedL[i] = true;
                }
                if (!_initedR[i] && !string.IsNullOrEmpty(_namesR[i]))
                {
                    vec2 s = Transform.GetScale(_namesR[i]);
                    if (s.x != 0f || s.y != 0f) _initedR[i] = true;
                }
            }
        }

        private vec2 GetOffset(int i)
        {
            switch (i)
            {
                case 0: return new vec2(OffsetXFarBack, OffsetYFarBack);
                case 1: return new vec2(OffsetXBack,    OffsetYBack);
                case 2: return new vec2(OffsetXMid,     OffsetYMid);
                case 3: return new vec2(OffsetXFront,   OffsetYFront);
                default: return new vec2(0f, 0f);
            }
        }

        private float GetTileWidth(int i)
        {
            switch (i)
            {
                case 0: return TileWidthFarBack;
                case 1: return TileWidthBack;
                case 2: return TileWidthMid;
                case 3: return TileWidthFront;
                default: return 0f;
            }
        }
    }
}
