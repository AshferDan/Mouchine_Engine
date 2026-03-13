/* Start Header ************************************************************************/
/*!
\file       PlayerMovement.cs
\author     Ashfer [100% - ????????????????????????????lines]
\par        m.binafzalkhan@digipen.edu
\date       Novenber, 26, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

using CSharpScript;
using System;
using System.Numerics;
using static System.Math;

// This file contains values that will be able to be adjusted via the editor.
namespace CSharpScript
{
    public class PlayerMovementStats : GameScript
    {
        //private static float mod = 31.25f;          // 12.5f * 2.5f
        // CLAMPS - Max values 
        public float Clamp_MaxRunSpeed = 1562.5f;   // 50.0f * mod
        public float Clamp_MaxJumpHeight = 2500f;   // 80f * mod

        // walking tiers
        public float T0_BaseSpeed = 31.25f;         // 1f * mod
        public float T0_MaxSpeed = 375f;            // 12f * mod

        // running tiers
        public float T1_Threshold = 562.5f;         // 18f * mod
        public float T2_Threshold = 750f;           // 24f * mod
        public float T3_Threshold = 937.5f;         // 30f * mod
        public float T3_MaxSpeed = 1000f;           // 32f * mod

        // Running
        public bool UseToggleRun = false;
        public float RunBaseSpeed = 468.75f;            // 15f * mod
        public float RunCoastDeceleration = 468.75f;    //15f * mod

        // Accel/ Decel
        public float GroundAcceleration = 625f;             // 20f * mod
        public float GroundDeceleration = 1562.5f;          // 50f * mod
        //[Range(0.25f, 50f)] 
        public float AirAcceleration = 625f;                // 20f * mod
        //[Range(0.25f, 50f)] 
        public float AirDeceleration = 156.25f;             // 5f * mod
        public float TurnResetSpeed = 156.25f;              // 5f * mod
        public float TentAcceleration = 312.5f;             // 10f * mod

        // Turn Skid
        //[Range(0.1f, 1f)] 
        public float SkidSpeedRetained = 0.25f;
        public float T1_SkidDuration = 0.45f;
        public float T2_SkidDuration = 0.55f;
        public float T3_SkidDuration = 0.8f;
        public float SkidExitSpeed = 100f;

        // Wall Bonk
        public float T3_BonkDuration = 0.65f;
        public Vector2 T3_BonkForce = new Vector2(562.5f, 312.5f);        // 18f * mod, 10f * mod

        // Grounded/Collision Checks
        public float GroundDetectionRayLength = 10.0f;
        public float HeadDetectionRayLength = 0.02f;
        public float HeadWidth = 0.75f;

        // Wall Checks
        public float WallDetectionBoxLength = 50.0f * 2f;            // unity is 0.05 - 0.02 (since our raycast is v diff)

        // Jump
        public bool EnableVariableJumpHeight = false;
        public float JumpHeight = 185f;                         // unity is 5.7 * mod???                        // unity is 5.7f * mod
        public float T1_JumpMultiplier = 1.05f;
        public float T2_JumpMultiplier = 1.08f;
        public float T3_JumpMultiplier = 1.15f;
        public float JumpHeightCompensationFactor = 1.054f;
        public float TimeTillJumpApex = 0.35f;
        public float GravityOnReleaseMultiplier = 3.0f;         // was 1.4
        public float MaxFallSpeed = 1250f;                      // 40f * mod
        public int NumberOfJumpsAllowed = 1;

        // Jump Cut
        public float TimeForUpwardsCancel = 0.02f;

        // land timer
        public float LandAnimationLength = 0.2f;

        // Jump Apex
        public float ApexThreshold = 0.9f;                      // was 0.97
        public float ApexHangTime = 0.04f;

        // Jump Buffer
        public float JumpBufferTime = 0.1f;

        // Jump Coyote Time
        public float JumpCoyoteTime = 0.1f;

        // Cooldowns
        public float GPCooldown = 1.0f;
        public float RollCooldown = 1.0f;
        public float WKCooldown = 0.5f;
        public float GlideCooldown = 0.2f;

        // Wall Jump
        public vec2 WallJumpForce = new vec2(625f, 937.5f);             // 20f * mod, 30f * mod
        public float WallCoyoteTime = 0.1f;

        // Wall Kick
        public float WallSlidingSpeed = 125f;                           // 4f * mod
        public float WallSlidingTimeToTriggerSound = 0.25f;             // very long name i dont care i dont care i don
        public float WallSlideRestickCooldown = 0.5f;                   // cooldown for remagnetising back to wall aft pressing S to cancel wallslide
        public float WKBaseDuration = 0.25f;
        public float WKSpeedModifier = 1093.75f;                        // 35f * mod
        public float WKVerticalModifier = 218.75f;                      // 7f * mod
        public float WKDurationMultiplier = 0.75f;
        // dont have tht WKEase graph thing in unity

        // Roll
        public float BaseRollDuration = 0.3f;       // minimum time
        public float MidRollDuration = 0.45f;
        public float MaxRollDuration = 0.7f;
        public float BaseRollSpeed = 1000f;         // 32f * mod
        public float MidRollSpeed = 1093.75f;       // 35f * mod
        public float MaxRollSpeed = 1250f;          // 40f * mod


        // Ground Pound
        public float GPPauseTime = 0.3f;
        public float GPFallSpeed = 1562.5f;                             // 50f * mod
        public float RollHeightToDurationDivisor = 2343.75f;            // 75f * mod
        public float GroundPoundMaxSpeedHeightThreshold = 80f;
        public float GPMaxDuration = 2.5f;

        // Air Glide
        public float GlideFallSpeedMultiplier = 0.5f;

        // Knockback
        public vec2 KnockbackForce = new vec2(937.5f, 1250f);           // 30f * mod, 40f * mod
        public vec2 KnockbackFriction = new vec2(2656.25f, 0f);         // 85f * mod
        public float knockbackTime = 0.3f;
        public float hurtStunDuration = 0.75f;

        // Zipline
        public float ZiplineSpeed = 937.5f;                             // 30f * mod
        public float ZiplineAttachDuration = 0.15f;

        // Zipline Launch
        public float ZiplineLaunchDuration = 0.25f;
        public vec2 ZiplineLaunchForce = new vec2(1250f, 500f);            // 40f * mod, 16f * mod
        public float ZiplineCoyoteTime = 0.2f;

        // Damage Taken/ Hurt
        public float HazardTimeDeduction = 1.0f;
        public float HurtImmunityLength = 2.0f;

        // Time Deduction Penalties
        public float JumpTimeDeduction = 1.0f;
        public float RunTimeDeduction = 0.1f;
        public float ZiplineTimeDeduction = 3.0f;
        public float ZiplineDeductionInterval = 0.2f;
        public float GroundPoundTimeDeduction = 3.0f;
        public float RollTimeDeduction = 2.0f;
        public float GlideTimeDeduction = 1.0f;
        public float GlideDeductionInterval = 0.5f;
        public float WallKickTimeDeduction = 2.5f;

        // Calculated values
        public float Gravity { get; private set; }
        public float InitialJumpVelocity { get; private set; }
        public float AdjustedJumpHeight { get; private set; }
        public float KnockbackInitialVerticalVelocity { get; private set; }

        public PlayerMovementStats()
        {
            CalculateValues();
        }

        private void CalculateValues()
        {
            AdjustedJumpHeight = JumpHeight * JumpHeightCompensationFactor;
            Gravity = -(2f * AdjustedJumpHeight) / (float)(Pow(TimeTillJumpApex, 2f));
            InitialJumpVelocity = Abs(Gravity) * TimeTillJumpApex;
        }
    }
}