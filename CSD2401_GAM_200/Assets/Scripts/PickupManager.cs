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
        public string Pickup_PowerUp_Name = "PU_Powerup";

        private float dt => Utils.GetDelta_Time();
        private float fixeddt => Utils.GetFixed_Delta_Time();

        public PickupManager()
        {
            // When the script is created, instantiate its stats
            Debug.Log("PickupManager instantiated with new stats.");
        }

        public void OnStart()
        {
            // It's crucial that OwnerName is set by the C++ engine before OnStart is called.
            if (OwnerName == null)
            {
                Debug.Log("ERROR: OwnerName is not set in PlayerMovement script!");
                return;
            }
        }

        public void OnUpdate(float deltaTime)
        {
            if (OwnerName == null || OwnerName == "") { Debug.Log("[PickupManager Script] no name"); return; }

            OnFixedUpdate(fixeddt);
            //likely can remove
            //if (Input.GetKeyPressed(Input.KeyCode.L))
            //{
            //    int test = Scripting.GetIntField(OwnerName, "pickupSys", "GetTotalCoins()");
            //    int datamem = Scripting.GetIntField(OwnerName, "pickupSys", "_coins");
            //    Debug.Log("test: " + test.ToString());
            //    Debug.Log("datamem: " + datamem.ToString());
            //}
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

                if (GameObject.FindGameObjectByID(goID).Contains(Pickup_Coin_Name))
                {
                    // coin effect
                }
                else if (GameObject.FindGameObjectByID(goID).Contains(Pickup_Shell_Name))
                {
                    // shell effect
                }
                else if (GameObject.FindGameObjectByID(goID).Contains(Pickup_PowerUp_Name))
                {
                    // powerup effect
                }
                else
                {
                    // Mismatched name but still of type Pickup?
                }

                Audio.PlaySound("pickup_sfx", "sfx");               // abit bugged bc of stopsound(sfx) in playermovement, need jon to take a look at c# api
                // destroy regardless of pickup type
                GameObject.DestroyByID(goID);
                Debug.Log("destroyed go id: " + goID.ToString());            // works
            }
        }

        public void OnDestroy()
        {
            Debug.Log("PlayerMovement Script Destroyed for object: " + OwnerName);
        }
    }
}
