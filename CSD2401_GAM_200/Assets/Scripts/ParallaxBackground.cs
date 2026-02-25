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
        public enum Layer { FarBack = 0, Back = 1, Mid = 2, Front = 3 }
        private const int LayerCount = 4;

        public string LayerFarBack = "ParallaxBG";
        public string LayerBack    = "";
        public string LayerMid     = "";
        public string LayerFront   = "";

        public float FactorFarBack = 1.0f;
        public float FactorBack    = 0.3f;
        public float FactorMid     = 0.5f;
        public float FactorFront   = 0.8f;

        private string[] layerNames;
        private float[]  factors;
        private vec2[]   startPos = new vec2[LayerCount];
        private float[]  lengths  = new float[LayerCount];
        private bool[]   inited   = new bool[LayerCount];

        public override void OnStart()
        {
            layerNames = new string[] { LayerFarBack, LayerBack, LayerMid, LayerFront };
            factors    = new float[]  { FactorFarBack, FactorBack, FactorMid, FactorFront };

            foreach (Layer layer in Enum.GetValues(typeof(Layer)))
            {
                int i = (int)layer;
                if (layerNames[i] == "") continue;

                float scaleX = Transform.GetScale(layerNames[i]).x;
                if (scaleX == 0f) continue; // object not found or has no width

                startPos[i] = Transform.GetPosition(layerNames[i]);
                lengths[i]  = scaleX;
                inited[i]   = true;
            }
        }

        public override void OnUpdate(float dt)
        {
            vec2 camPos = Camera.GetPosition();

            foreach (Layer layer in Enum.GetValues(typeof(Layer)))
            {
                int i = (int)layer;
                if (!inited[i]) continue;

                float f    = factors[i];
                float temp = camPos.x * (1 - f);
                float dist = camPos.x * f;

                Transform.SetPosition(layerNames[i],
                    startPos[i].x + dist,
                    startPos[i].y + camPos.y * f);

                // Seamless looping: shift anchor when camera drifts past one tile width
                vec2 sp = startPos[i];
                if      (temp > sp.x + lengths[i]) sp.x += lengths[i];
                else if (temp < sp.x - lengths[i]) sp.x -= lengths[i];
                startPos[i] = sp;
            }
        }

        public override void OnDestroy()
        {
        }
    }
}
