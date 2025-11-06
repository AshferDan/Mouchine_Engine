using GameEngine;

namespace GameScripts
{
    /// <summary>
    /// Simple script that creates two static square objects on start
    /// </summary>
    public class TestGameObjects
    {
        public void OnStart()
        {
            Debug.Log("TwoSquaresSpawner: Creating two squares...");

            // Create first square - Red, on the left
            GameObject.CreateShape(
                "RedSquare",                     // name
                (int)ShapeType.Square,           // type: square
                100, 300,                        // x, y position
                100, 100,                        // width, height
                1.0f, 0.0f, 0.0f, 1.0f,          // r, g, b, a (red color)
                (int)CollisionLayer.Default,     // collision layer
                false,                           // hasPhysics = false (no physics)
                false,                           // isStatic (doesn't matter if no physics)
                false                            // hasGravity = false (no gravity)
            );

            // Create second square - Blue, on the right
            GameObject.CreateShape(
                "BlueSquare",                    // name
                (int)ShapeType.Square,           // type: square
                400, 300,                        // x, y position
                100, 100,                        // width, height
                0.0f, 0.0f, 1.0f, 1.0f,          // r, g, b, a (blue color)
                (int)CollisionLayer.Default,     // collision layer
                false,                           // hasPhysics = false (no physics)
                false,                           // isStatic
                false                            // hasGravity = false (no gravity)
            );

            Debug.Log("TwoSquaresSpawner: Two squares created successfully!");
        }

        public void OnUpdate(float deltaTime)
        {
            // Nothing to update - squares are static
        }

        public void OnDestroy()
        {
            Debug.Log("TwoSquaresSpawner: Cleaning up...");
        }
    }
}