using System;
using CSharpScript;

namespace GameScripts
{
    public class TestGameObjects2
    {
        private float rotationSpeed = 90.0f; // Degrees per second

        public void OnStart()
        {
            // --- Moved Blue Square ---
            GameObject.CreateShape(
                "BlueSquare",
                (int)ShapeType.Square,
                300, 300,
                300, 300,
                0.0f, 0.0f, 1.0f, 1.0f, // Blue
                (int)CollisionLayer.Default,
                false, false, false
            );
            Components.AddPhysicsComponent("BlueSquare");

            GameObject.CreateShape(
                "GreenSquare",
                (int)ShapeType.Square,
                -100, 300,                  // Positioned between Red and Blue starting positions
                150, 150,                // Slightly smaller
                0.0f, 1.0f, 0.0f, 1.0f,  // Green
                (int)CollisionLayer.Default,
                false, false, false
            );
        }

        public void OnUpdate(float deltaTime)
        {
            // Rotate Green Square every frame
            Transform.AddRotation("GreenSquare", rotationSpeed * deltaTime);
        }

        public void OnDestroy()
        {

        }
    }
}