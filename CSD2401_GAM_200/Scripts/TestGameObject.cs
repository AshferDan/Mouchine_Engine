
/* Start Header ************************************************************************/
/*!
\file       TestGameObject.cs
\author     Jonathan Fam Qian Wei [100% - 55lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

using CSharpScript;

namespace GameScripts
{
    public class TestGameObject
    {
        public void OnStart()
        {
            // Create the red square
            CSharpScript.GameObject.CreateShape(
                "RedSquare",
                (int)CSharpScript.ShapeType.Square,
                500, 300,
                50, 50,
                1.0f, 0.0f, 0.0f, 1.0f,
                (int)CSharpScript.CollisionLayer.Default,
                false, false, false
            );

            // Create the green square
            CSharpScript.GameObject.CreateShape(
                "GreenSquare",
                (int)CSharpScript.ShapeType.Square,
                -300, 300,
                50, 50,
                0.0f, 1.0f, 0.0f, 1.0f,
                (int)CSharpScript.CollisionLayer.Default,
                false, false, false
            );

            // Attach the RaycastTest script to the RedSquare
            CSharpScript.Scripting.AttachScript("RedSquare", "RaycastTest");
        }

        public void OnUpdate(float deltaTime)
        {
            if (Input.GetKeyPressed(Input.KeyCode.K))
            {
                CSharpScript.Debug.Log("button Pressed");
            }
            else if (Input.GetKeyReleased(Input.KeyCode.K))
            {
                CSharpScript.Debug.Log("Key Have Been Released");
            }
            else if (Input.GetKeyHeld(Input.KeyCode.K))
            {
                CSharpScript.Debug.Log("Key Have Been holding");
            }
        }

        public void OnDestroy()
        {
            CSharpScript.Debug.Log("TestGameObject: Cleaning up...");
        }
    }
}