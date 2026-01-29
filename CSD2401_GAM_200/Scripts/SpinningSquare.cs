using System;
using CSharpScript;

namespace CSharpScript
{
    public class SpinningSquare : GameScript
    {
        public float RotationSpeed = 2.0f; // Made public
        public bool ShouldRotate = true;
        public string Message = "Hello, Editor!";
        public int LoopCount = 5;

        public override void OnStart()
        {
            Debug.Log($"SpinningSquare started on {OwnerName}");
        }

        public override void OnUpdate(float dt)
        {
            vec2 velocity = Physics.GetVelocity(OwnerName);
            Debug.Log($"Velocity of {OwnerName}: {velocity.x}, {velocity.y}");
            if (ShouldRotate)
            {
                Transform.AddRotation(OwnerName, RotationSpeed * dt);
                Graphics.SetColor(OwnerName,0.0f, 0.0f, 0.0f, 1.0f);
            }
        }

        public override void OnDestroy()
        {
            Debug.Log($"SpinningSquare destroyed on {OwnerName}");
        }
    }
}