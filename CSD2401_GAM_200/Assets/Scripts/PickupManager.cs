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
            if (OwnerName == null || OwnerName == "") { Debug.Log("no name"); return; }

            OnFixedUpdate(fixeddt);
        }

        public void OnFixedUpdate(float fixedDeltaTime)
        {
            HandlePickup();
        }

        public void HandlePickup()
        {
            int coinID = Scripting.GetCollidingObjectID(OwnerName, (int)CollisionLayer.Pickup);
            if (coinID != 0)
            {
                string coinName = GameObject.FindGameObjectByID(coinID);
                Debug.Log("Picked up: " + coinName);
                GameObject.Destroy(coinName);
            }
        }

        public void OnDestroy()
        {
            Debug.Log("PlayerMovement Script Destroyed for object: " + OwnerName);
        }
    }
}
