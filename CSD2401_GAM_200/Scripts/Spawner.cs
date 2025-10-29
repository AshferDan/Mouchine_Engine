using GameEngine; // Import the API you just defined

// This namespace MUST be "GameScripts" for your C++ code to find it
namespace GameScripts
{
    // The class name "Spawner" is what you will use in C++
    public class Spawner
    {
        // This method will be called by ScriptSystem::CallOnStart
        public void OnStart()
        {
            Debug.Log("Hello from C#! The Spawner script is starting.");

            // Now, call the C++ API to create a new GameObject
            GameObject.CreateShape(
                "C-Sharp-Object",                // name
                (int)ShapeType.Circle,           // type
                0, 500,                          // x, y
                75, 75,                          // width, height
                0.0f, 1.0f, 0.0f, 1.0f,          // r, g, b, a (green)
                (int)CollisionLayer.Enemy,       // layer
                true,                            // hasPhysics
                false,                           // isStatic
                true                             // hasGravity
            );

            Physics.SetVelocity("C-Sharp-Object", 100.0f, 0.0f);
        }

        // This method will be called by ScriptSystem::CallOnUpdate
        public void OnUpdate(float deltaTime)
        {
            // This will run every frame
            // Debug.Log("C# Update!"); 
        }
    }
}