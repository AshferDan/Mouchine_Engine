
using CSharpScript;
using System;

namespace GameScripts
{
    public class TestGameObject
    {
        private float redSquareX = -200.0f;
        private const float redSquareY = 300.0f;
        private const float moveSpeed = 100.0f; // Speed in units per second

        public void OnStart()
        {


            // Create first square - Red, on the left
            GameObject.CreateShape(
                "RedSquare",                     // name
                (int)ShapeType.Square,           // type: square (fixed from Circle)
                -200, 300,                        // x, y position
                200, 200,                        // width, height
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
                500, 300,                        // x, y position
                300, 300,                        // width, height
                0.0f, 0.0f, 1.0f, 1.0f,          // r, g, b, a (blue color)
                (int)CollisionLayer.Default,     // collision layer
                false,                           // hasPhysics = false (no physics)
                false,                           // isStatic
                false                            // hasGravity = false (no gravity)
            );
           

            GameObject.CreateAnimatedImage("C#_AniImage",0,200,
                300, 300,
                "../Assets/spritesheet_R.png",
                11,8,
                24,29,
                16,(int)CollisionLayer.Default,
                false,false,
                false);
        }

        public void OnUpdate(float deltaTime)
        {
            redSquareX -= moveSpeed * deltaTime;

            // 3. Apply the updated position to the game object
            Transform.SetPosition("RedSquare", redSquareX, redSquareY);

            if (redSquareX < -800)
            {
                redSquareX = 500;
            }
        }

        public void OnDestroy()
        {
            Debug.Log("TestGameObject: Cleaning up...");
        }
    }
}