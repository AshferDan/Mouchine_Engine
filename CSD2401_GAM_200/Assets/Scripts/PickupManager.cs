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
using System.Diagnostics.Contracts;

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

        public PlayerMovementStats MoveStats;
        private bool triggeredTimePU = false;
        private bool triggeredShellPU = false;
        private vec2 cachedPlayerSize;

        public PickupManager()
        {
            // When the script is created, instantiate its stats
            MoveStats = new PlayerMovementStats();
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

            cachedPlayerSize = Transform.GetScale(PlayerMovement.vfx);              // since vfx scaled to player

            Audio.AddSound("PU_Time_sfx", "../Assets/Audio/Pickups/PU_Time.wav", false, 0.7f, 1.0f, "pu_sfx");
            Audio.AddSound("PU_Time_Hum_sfx", "../Assets/Audio/Pickups/PU_Time_Hum.wav", false, 0.3f, 1.2f, "pu_sfx");
            Audio.AddSound("PU_Speed_sfx", "../Assets/Audio/Pickups/PU_Speed.wav", false, 0.9f, 1.0f, "pu_sfx");
            Audio.AddSound("PU_Shell_sfx", "../Assets/Audio/Pickups/PU_Shell.wav", false, 0.9f, 1.0f, "pu_sfx");
            Audio.AddSound("PU_Coin_sfx1", "../Assets/Audio/Pickups/PU_Coin1.wav", false, 0.4f, 1.0f, "pu_sfx");
            Audio.AddSound("PU_Coin_sfx2", "../Assets/Audio/Pickups/PU_Coin2.wav", false, 0.4f, 1.0f, "pu_sfx");
            Audio.AddSound("PU_Coin_sfx3", "../Assets/Audio/Pickups/PU_Coin3.wav", false, 0.4f, 1.0f, "pu_sfx");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (OwnerName == null || OwnerName == "") { Debug.Log("[PickupManager Script] no name"); return; }

            OnFixedUpdate(fixeddt);
        }

        public void OnFixedUpdate(float fixedDeltaTime)
        {
            HandlePickup();
            HandleTimePU();
            HandleShellPU();
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
                    Audio.PlaySound("PU_Coin_sfx" + variation.ToString(), "pu_sfx");
                    Transform.SetScale(PlayerMovement.pu_vfx, cachedPlayerSize.x * 1.5f, cachedPlayerSize.y * 1.5f);
                }
                else if (goName.Contains(Pickup_Shell_Name))
                {
                    // shell effect
                    Pickups.CollectShell();
                    //Audio.StopSound("shell_glow");
                    //Audio.FadeOutCategory("shell_glow", 2);                   // waiting for xy to add fade by sound
                    //Debug.Log("\n\n\n\n\n\n\n\n\n");
                    Audio.FadeOutSound("shell_glow", 1);
                    Audio.PlaySound("PU_Shell_sfx", "pu_sfx");
                    triggeredShellPU = true;
                    Transform.SetScale(PlayerMovement.pu_vfx, cachedPlayerSize.x * 2.0f, cachedPlayerSize.y * 2.0f);
                }
                else if (goName.Contains(Pickup_Speed_Name))
                {
                    // powerup effect
                    Audio.PlaySound("PU_Speed_sfx", "pu_sfx");
                    PlayerMovement.triggerSpeedBoost = true;
                    Transform.SetScale(PlayerMovement.pu_vfx, cachedPlayerSize.x * 1.5f, cachedPlayerSize.y * 1.5f);
                }
                else if (goName.Contains(Pickup_Time_Name))
                {
                    // powerup effect
                    Audio.PlaySound("PU_Time_sfx", "pu_sfx");
                    Audio.PlaySound("PU_Time_Hum_sfx", "pu_sfx");
                    triggeredTimePU = true;
                    Transform.SetScale(PlayerMovement.pu_vfx, cachedPlayerSize.x * 1.5f, cachedPlayerSize.y * 1.5f);
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

        float timePUTimer = 0.0f;
        bool timePUActive = false;
        private void HandleTimePU()
        {
            if (triggeredTimePU)
            {
                GameTimer.AddTime(MoveStats.Time_PickupAmount);
                timePUTimer = MoveStats.Time_PickupAmount;
                Animation.PlayAnimation(PlayerMovement.pu_vfx, "Time", true);
                triggeredTimePU = false;
                timePUActive = true;
            }

            if (timePUActive)
            {
                timePUTimer -= fixeddt;
                if (timePUTimer <= 0.0f)
                {
                    Animation.PlayAnimation(PlayerMovement.pu_vfx, "Clear");
                    timePUActive = false;
                    timePUTimer = 0.0f;
                }

                if (!string.IsNullOrEmpty(GameObject.FindGameObjectByName("LevelTimerBar")))
                {
                    if (timePUActive)
                        Graphics.SetTexture("LevelTimerBar", "../Assets/Sprites/Icons/Time bar_green.png");
                    else
                        Graphics.SetTexture("LevelTimerBar", "../Assets/Sprites/Icons/Time bar_blue.png");
                }
            }
        }

        float shellPUTimer = 0.0f;
        bool shellPUActive = false;
        public void HandleShellPU()
        {

            if (triggeredShellPU)
            {
                GameTimer.AddTime(MoveStats.Time_PickupAmount);
                timePUTimer = MoveStats.Time_PickupAmount;
                shellPUTimer = (1.0f / 8f) * 10f;                    // 8 fps and 10 frames
                Animation.PlayAnimation(PlayerMovement.pu_vfx, "Shell", false);
                triggeredShellPU = false;
                shellPUActive = true;
            }

            if (shellPUActive)
            {
                shellPUTimer -= fixeddt;
                if (shellPUTimer <= 0.0f)
                {
                    Animation.PlayAnimation(PlayerMovement.pu_vfx, "Clear");
                    shellPUActive = false;
                    shellPUTimer = 0.0f;
                }
            }
        }

        public override void OnDestroy()
        {
            Debug.Log("PlayerMovement Script Destroyed for object: " + OwnerName);
        }
    }
}
