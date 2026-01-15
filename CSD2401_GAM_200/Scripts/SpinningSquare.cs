using System;
using CSharpScript;

namespace CSharpScript
{
    public class SpinningSquare : GameScript
    {
        private float rotationSpeed = 2.0f;

        public override void OnStart()
        {
            Debug.Log($"SpinningSquare started on {OwnerName}");
        }

        public override void OnUpdate(float dt)
        {
            Transform.AddRotation(OwnerName, rotationSpeed * dt);
            Graphics.SetColor(OwnerName,0.0f, 0.0f, 0.0f, 1.0f);
        }

        public override void OnDestroy()
        {
            Debug.Log($"SpinningSquare destroyed on {OwnerName}");
        }
    }
}