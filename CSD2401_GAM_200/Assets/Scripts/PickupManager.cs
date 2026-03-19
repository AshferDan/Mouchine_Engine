/* Start Header ************************************************************************/
/*!
\file       PlayerMovement.cs
\author     Ashfer [100% - 748 lines]
\par        m.binafzalkhan@digipen.edu
\date       Novenber, 26, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

using System;
using CSharpScript;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Linq;
using System.Diagnostics;
using System.Security.Cryptography;
using System.Resources;
using System.Runtime.ExceptionServices;

namespace CSharpScript
{
    public class PickupManager : GameScript
    {
        public string Pickup_Coin_Name = "PU_Coin";
        public string Pickup_Shell_Name = "PU_Shell";
        public string Pickup_Speed_Name = "PU_Speed";
        public string Pickup_Time_Name = "PU_Time";

        private float dt => Utils.GetDelta_Time();
        private float fixeddt => Utils.GetFixed_Delta_Time();

        public PickupManager()
        {
            // When the script is created, instantiate its stats
            Debug.Log("PickupManager instantiated with new stats.");
        }

        public override void OnStart()
        {
            // It's crucial that OwnerName is set by the C++ engine before OnStart is called.
            if (OwnerName == null)
            {
                Debug.Log("ERROR: OwnerName is not set in PlayerMovement script!");
                return;
            }

            Audio.AddSound("PU_Time_sfx", "../Assets/Audio/Pickups/PU_Time.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("PU_Speed_sfx", "../Assets/Audio/Pickups/PU_Speed.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("PU_Shell_sfx", "../Assets/Audio/Pickups/PU_Shell.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("PU_Coin_sfx1", "../Assets/Audio/Pickups/PU_Coin1.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("PU_Coin_sfx2", "../Assets/Audio/Pickups/PU_Coin2.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("PU_Coin_sfx3", "../Assets/Audio/Pickups/PU_Coin3.wav", false, 0.8f, 1.0f, "sfx");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (OwnerName == null || OwnerName == "") { Debug.Log("[PickupManager Script] no name"); return; }

            OnFixedUpdate(fixeddt);
        }

        public void OnFixedUpdate(float fixedDeltaTime)
        {
            HandlePickup();
        }

        // Script will only handle collision and effect. Tracking and counting will be handled in c++ because its easier to access from other systems, save to file etc.
        public void HandlePickup()
        {
            if (Collision.IsCollidingWithLayer(OwnerName, (int)CollisionLayer.Pickup))
            {
                int goID = Scripting.GetCollidingObjectID(OwnerName, (int)CollisionLayer.Pickup);
                if (goID <= 0) return;

                string goName = GameObject.FindGameObjectByID(goID);
                if (string.IsNullOrEmpty(goName)) return;   // object already destroyed or invalid

                if (goName.Contains(Pickup_Coin_Name))
                {
                    // coin effect
                    Pickups.CollectCoin();

                    Random rnd = new Random();
                    int variation = rnd.Next(1, 4);
                    Audio.PlaySound("PU_Coin_sfx" + variation.ToString(), "sfx");
                }
                else if (goName.Contains(Pickup_Shell_Name))
                {
                    // shell effect
                    Pickups.CollectShell();
                    Audio.PlaySound("PU_Shell_sfx", "sfx");
                }
                else if (goName.Contains(Pickup_Speed_Name))
                {
                    // powerup effect
                    Audio.PlaySound("PU_Speed_sfx", "sfx");
                    PlayerMovement.triggerSpeedBoost = true;
                }
                else if (goName.Contains(Pickup_Time_Name))
                {
                    // powerup effect
                    Audio.PlaySound("PU_Time_sfx", "sfx");
                }
                else
                {
                    // Mismatched name but still of type Pickup?
                }

                // destroy regardless of pickup type
                GameObject.DestroyByID(goID);
                Debug.Log("destroyed go id: " + goID.ToString());            // works
            }
        }

        public override void OnDestroy()
        {
            Debug.Log("PlayerMovement Script Destroyed for object: " + OwnerName);
        }
    }
}
