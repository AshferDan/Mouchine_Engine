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
using System.Collections.Generic;
using System.IO.Pipes;

namespace CSharpScript
{
    public class PlayerMovement : GameScript
    {
        // basically assigns names to numbers that can be used to track states
        enum PlayerState
        {
            Idle = 0,
            Walking,
            Jumping,
            Running,
            Skidding,
            Falling,
            Land,
            GroundPounding,
            GroundPounding_Falling,
            Hurting,
            Bonking,
            Rolling,
            WallKicking,
            Ziplining,
            Gliding,
            WallSliding,
            WallJumping
        };

        private PlayerState currState = PlayerState.Idle;
        private PlayerState[] loopingSounds = { PlayerState.Walking, PlayerState.Falling, PlayerState.GroundPounding_Falling, PlayerState.Ziplining, PlayerState.Gliding, PlayerState.WallSliding };
        private PlayerState[] nonTrackingVFX = { PlayerState.Jumping, PlayerState.WallJumping, PlayerState.WallKicking, PlayerState.Land, PlayerState.Rolling};
        private float[] footstepFrequencies = { 0.47f, 0.3f, 0.25f, 0.2f };       // walking - tier1 - tier2- tier3
        private float minFootstepDistance = 5.0f;
        public static bool triggerSpeedBoost = false;
        private bool speedBoostActive = false;

        // list contains (entityID, position, isAudioPlaying)
        class NPCData
        {
            public uint ID;
            public vec2 pos;
            public bool audioPlaying;
            public NPCData(uint id, vec2 position, bool soundPlaying)
            {
                ID = id;
                pos = position;
                audioPlaying = soundPlaying;
            }
        }
        List<NPCData> NPCs = new List<NPCData>();     
        private float dt => Utils.GetDelta_Time();
        private float fixeddt => Utils.GetFixed_Delta_Time();

        // Not use yet: private string audioFilepath = "..Assets/Audio/";

        public PlayerMovementStats MoveStats;
        public PickupManager pickupManager;

        #region Variables
        public bool fRun, fJump, fIdle, fZipline, fGroundPound, fGlide, fRoll, fWallKick;

        // scissors sound flags
        private bool _lobster1SoundPlaying = false;
        private bool _lobster2SoundPlaying = false;

        // movement variables
        private vec2 _moveVelocity = new vec2(0, 0);
        private bool _isFacingRight = true;
        private bool _isRunning;
        private bool _isRunToggleActive = false;     // internal state to track if player is running (used for Toggle Mode)
        private float movespeed;
        public float CurrentSpeed => movespeed;

        // public facing right for other scripts
        public bool IsFacingRight => _isFacingRight;        // public facing right for other scripts
        // Not use yet: private int _queuedDirection = 0;                   // remembers input while airborne
        private int _facingDirection = 1;                   // visual direction

        // collision check variables
        private bool _isGrounded = false;
        private bool _bumpedHead = false;
        private bool _islanding = false;
        private float _groundedTimer;

        // jump variables
        public float VerticalVelocity { get; private set; } = 0;
        private bool _isJumping = false;
        private bool _isFastFalling = false;
        private bool _isFalling = false;
        private float _fastFallTime = 0;
        private float _fastFallReleaseSpeed = 0;
        private int _numberOfJumpsUsed = 0;
        private float landAnimationLength = 0.0f;

        // wall Jump variables
        private bool _isWallJumping = false;
        private float _wallJumpTimer = 0;
        private float _wallJumpDirection = 0;

        // apex variables
        private float _apexPoint = 0;
        private float _timePastApexThreshold = 0;
        private bool _isPastApexThreshold = false;

        // jump buffer variables
        private float _jumpBufferTimer = 0;
        private bool _jumpReleasedDuringBuffer = false;

        // coyote time variables
        private float _coyoteTimer = 0;

        // wall kick variables
        private bool _isWallKicking = false;
        //private int wallKickFrameCounter = 0;           // [WIP] for cayote time 
        private bool canWallKick = false;                   // Checks if touching a wall, and can perform a wall kick
#pragma warning disable CS0414
        private bool blockMovement = false;         // Prevents player from controlling character
#pragma warning restore CS0414
        private float _wallKickTimer = 0;
        private float _wallKickCooldownTimer = 0;
        // Not use yet: private bool _startedWallKickInAir = false;
        private float _wallKickTotalDuration = 0;

        // wall slide variables
        //private float _wallSlideRestickCooldown = 0;
        //private bool canWallSlide = false;
        private bool _isTouchingWall = false;
        private bool _isWallSliding = false;
        private int _wallDirection = 0;

        // ground pound variables
        private bool _isGroundPounding = false;     // checks if player is currently in groundpound state
        vec2 lastPos;                            // stores pos of when the player initiated groundpound (for conversion to horizontal dist)
        private float _groundPoundPauseTimer = 0;
        private float _groundPoundCooldownTimer = 0;

        // roll variables
        private bool _isRolling = false;            // checks if player is in rolling state
        private float _rollTimer = 0;
        private float _rollCooldownTimer = 0;

        // glide variables
        private bool _isGliding = false;            // checks if player is in gliding state
        private float _glideCooldownTimer = 0;
        private float _timeSinceLastGlideDeduction = 0;  // timer for periodic deduction

        // zipline variables
        private bool _isZiplining = false;
        // Not use yet: private bool _isBeingLaunched = false;
        private float _ziplineCoyoteTimer = 0;
        // Not use yet: private float _ziplineHeadOffset = 0;

        // knockback
        bool IsBeingKnockedBack = false;
        public float hitDirectionForce = 10f;
        public float constForce = 5f;
        public float inputForce = 7.5f;
        public static bool isControlLocked { get; set; }

        // movement penalties - bonk & skid & hurt
        private float _bonkTimer = 0f;
        private bool _isSkidding;
        // Not use yet: private int _skidTargetDirection;
        private float _skidTimer = 0f;
        private float _calculatedSkidDecel;
        private float _hurtTimer = 0f;

        // camera
        vec2 cam_defaultOffset = new vec2();

        public static string vfx = "Hero_vfx";
        public static string pu_vfx = "Pickup_vfx";
        public static string vfx_nontracking = "Hero_vfx_nontracking";
        #endregion
        public override void OnAwake()
        {

            Debug.Log("\n\ntesting ON AWAKE\n\n");

        }

        public PlayerMovement()
        {
            // When the script is created, instantiate its stats
            MoveStats = new PlayerMovementStats();
            pickupManager = new PickupManager();
            Debug.Log("PlayerMovement instantiated with new stats.");
        }

        public override void OnStart()
        {
            // It's crucial that OwnerName is set by the C++ engine before OnStart is called.
            if (OwnerName == null)
            {
                Debug.Log("ERROR: OwnerName is not set in PlayerMovement script!");
                return;
            }
            ResetVars();

            if (GameObject.FindGameObjectByName(vfx) == "")
            {
                CSharpScript.GameObject.CreateAnimatedImage(
                    vfx,
                    0, 0,
                    Transform.GetScale(OwnerName).x, Transform.GetScale(OwnerName).y,
                    "../Assets/spritesheet_player_vfx.png",
                    15, 11,
                    0, 0, 0, 1,
                    12.0f,
                    (int)CSharpScript.CollisionLayer.Player,
                    false,
                    false,
                    false
                );
            }
            if (GameObject.FindGameObjectByName(pu_vfx) == "")
            {
                CSharpScript.GameObject.CreateAnimatedImage(
                    pu_vfx,
                    0, 0,
                    Transform.GetScale(OwnerName).x, Transform.GetScale(OwnerName).y,
                    "../Assets/spritesheet_player_vfx.png",
                    15, 11,
                    14, 10, 14, 10,
                    12.0f,
                    (int)CSharpScript.CollisionLayer.Player,
                    false,
                    false,
                    false
                );
            }

            // replace with onawake when it works
            //Debug.Log("\n\n\n\n\n");
            //int totalGOs = GameObject.CountAll();
            //for (int i = 0; i < totalGOs; i++)
            //{
            //    string currGO = GameObject.FindGameObjectByName("NPC_Lobster");
            //    Debug.Log("findbyname: " + currGO);
            //    if (currGO != "")
            //    {
            //        NPCs.Add((uint.Parse(currGO), Transform.GetPosition(currGO), false));
            //        Debug.Log(currGO);
            //    }
            //}

            // wip to fix the vfx issue
            //CSharpScript.GameObject.CreateAnimatedImage(
            //    vfx_nontracking,
            //    0, 0,
            //    Transform.GetScale(OwnerName).x, Transform.GetScale(OwnerName).y,
            //    "../Assets/spritesheet_player_vfx.png",
            //    15, 11,
            //    0, 0, 0, 1,
            //    12.0f,
            //    (int)CSharpScript.CollisionLayer.Player,
            //    false,
            //    false,
            //    false
            //);

            // Add sounds
            Audio.AddSound("jump_sfx", "../Assets/Audio/Player/Jump.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("groundpound_sfx", "../Assets/audio/Player/Groundpound.wav", false, 0.5f, 1.5f, "sfx");
            Audio.AddSound("groundpound_falling_sfx", "../Assets/audio/Player/Groundpound_falling.wav", false, 0.5f, 2.0f, "sfx_loop");
            Audio.AddSound("roll_sfx", "../Assets/audio/Player/Roll.wav", false, 0.1f, 1.25f, "sfx");
            Audio.AddSound("wallslide_impact_sfx", "../Assets/audio/Player/Wallkick.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("wallslide_sfx", "../Assets/audio/Player/WallSlide.wav", false, 0.1f, 0.9f, "sfx");
            Audio.AddSound("wallkick_fast_sfx", "../Assets/audio/Player/Dash.wav", false, 0.5f, 1.35f, "sfx");
            Audio.AddSound("airglide_sfx", "../Assets/audio/Player/AirGlide.wav", false, 0.5f, 1.5f, "sfx_loop");
            Audio.AddSound("turnskid_sfx", "../Assets/audio/Player/TurnSkid.wav", false, 0.05f, 1.1f, "sfx");
            Audio.AddSound("bonk_sfx", "../Assets/audio/Player/WallBonk.wav", false, 0.05f, 1.1f, "sfx");
            Audio.AddSound("hurt_sfx", "../Assets/audio/Player/Hurt-Damage.wav", false, 0.3f, 1.0f, "sfx");
            Audio.AddSound("pickup_sfx", "../Assets/audio/Player/Pickup.wav", false, 0.3f, 1.0f, "sfx");

            // load 3D spatial sound for NPCs (scissors snip for lobster)
            Audio.Load3DSound("scissors_snip", "../Assets/Audio/hairscissors_snip1.wav", true, 500.0f, 3000.0f, "sfx", 0.4f, 1.0f);

            //footsteps
            Audio.AddSound("footstep_sfx1", "../Assets/audio/Player/Footstep/Footstep1.wav", false, 0.15f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx2", "../Assets/audio/Player/Footstep/Footstep2.wav", false, 0.15f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx3", "../Assets/audio/Player/Footstep/Footstep3.wav", false, 0.15f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx4", "../Assets/audio/Player/Footstep/Footstep4.wav", false, 0.15f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx5", "../Assets/audio/Player/Footstep/Footstep5.wav", false, 0.15f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx6", "../Assets/audio/Player/Footstep/Footstep6.wav", false, 0.15f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx7", "../Assets/audio/Player/Footstep/Footstep7.wav", false, 0.15f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx8", "../Assets/audio/Player/Footstep/Footstep8.wav", false, 0.15f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx9", "../Assets/audio/Player/Footstep/Footstep9.wav", false, 0.15f, 1.0f, "sfx");
            //landing
            Audio.AddSound("land_sfx1", "../Assets/audio/Player/Land/Land1.wav", false, 0.3f, 1.0f, "sfx");
            Audio.AddSound("land_sfx2", "../Assets/audio/Player/Land/Land2.wav", false, 0.3f, 1.0f, "sfx");
            Audio.AddSound("land_sfx3", "../Assets/audio/Player/Land/Land3.wav", false, 0.3f, 1.0f, "sfx");
            Audio.AddSound("land_sfx4", "../Assets/audio/Player/Land/Land4.wav", false, 0.3f, 1.0f, "sfx");

            // KEEP IN MIND! ALL SHOULD BE SPRITESHEET_R EXCEPT THOSE THAT REQUIRE FLIPPING (WALLSLIDE ETC)
            Animation.RegisterAnimation("Hero", "Idle", "../Assets/spritesheet_R.png", 13, 10, 7, 0, 7, 2, 4.0f);
            Animation.RegisterAnimation("Hero", "Walk", "../Assets/spritesheet_R.png", 13, 10, 12, 0, 12, 7, 8.0f);
            Animation.RegisterAnimation("Hero", "Run1", "../Assets/spritesheet_R.png", 13, 10, 12, 0, 12, 7, 12.0f);
            Animation.RegisterAnimation("Hero", "Run2", "../Assets/spritesheet_R.png", 13, 10, 12, 0, 12, 7, 16.0f);
            Animation.RegisterAnimation("Hero", "Run3", "../Assets/spritesheet_R.png", 13, 10, 12, 0, 12, 7, 22.0f);
            Animation.RegisterAnimation("Hero", "Skidding", "../Assets/spritesheet_R.png", 13, 10, 3, 0, 3, 2, 6.0f);
            Animation.RegisterAnimation("Hero", "Jump", "../Assets/spritesheet_R.png", 13, 10, 11, 4, 11, 5, 12.0f);
            Animation.RegisterAnimation("Hero", "Glide", "../Assets/spritesheet_R.png", 13, 10, 1, 0, 1, 2, 8.0f);
            Animation.RegisterAnimation("Hero", "Roll", "../Assets/spritesheet_R.png", 13, 10, 8, 0, 8, 5, 14.0f);
            Animation.RegisterAnimation("Hero", "Groundpound_Init", "../Assets/spritesheet_R.png", 13, 10, 9, 0, 9, 1, 10.0f);
            Animation.RegisterAnimation("Hero", "Groundpound", "../Assets/spritesheet_R.png", 13, 10, 9, 3, 9, 4, 10.0f);
            Animation.RegisterAnimation("Hero", "Dash", "../Assets/spritesheet_R.png", 13, 10, 0, 0, 0, 3, 4.0f);
            Animation.RegisterAnimation("Hero", "Wallkick", "../Assets/spritesheet_R.png", 13, 10, 4, 3, 4, 4, 4.0f);
            Animation.RegisterAnimation("Hero", "Wallslide", "../Assets/spritesheet_L.png", 13, 10, 4, 0, 4, 1, 4.0f);
            Animation.RegisterAnimation("Hero", "Falling", "../Assets/spritesheet_R.png", 13, 10, 10, 0, 10, 1, 4.0f);
            Animation.RegisterAnimation("Hero", "Land", "../Assets/spritesheet_R.png", 13, 10, 10, 3, 10, 4, 4.0f);
            Animation.RegisterAnimation("Hero", "Hurt", "../Assets/spritesheet_R.png", 13, 10, 0, 0, 0, 2, 3.0f);
            Animation.RegisterAnimation("Hero", "Bonk", "../Assets/spritesheet_R.png", 13, 10, 2, 0, 2, 2, 4.0f);
            Animation.RegisterAnimation("Hero", "Zipline", "../Assets/spritesheet_R.png", 13, 10, 6, 0, 6, 1, 4.0f);
            landAnimationLength = 0.15f;         // can calculate roughly how long it shud be with fps * amnt of frames / 60fps. 4.0f * 2 / 60;

            //VFX
            Animation.RegisterAnimation("Hero_vfx", "Clear", "../Assets/spritesheet_player_vfx.png",        15, 11, 14, 10, 14, 10, 0f);        // corner w nothing
            Animation.RegisterAnimation("Hero_vfx", "Walk", "../Assets/spritesheet_player_vfx.png",         15, 11, 10, 0, 10, 4, 8.0f);
            Animation.RegisterAnimation("Hero_vfx", "Run1", "../Assets/spritesheet_player_vfx.png",         15, 11, 9, 0, 9, 3, 8.0f);
            Animation.RegisterAnimation("Hero_vfx", "Run2", "../Assets/spritesheet_player_vfx.png",         15, 11, 9, 0, 9, 3, 12.0f); 
            Animation.RegisterAnimation("Hero_vfx", "Run3", "../Assets/spritesheet_player_vfx.png",         15, 11, 9, 5, 9, 8, 16.0f);
            Animation.RegisterAnimation("Hero_vfx", "Skidding", "../Assets/spritesheet_player_vfx.png",     15, 11, 0, 0, 0, 4, 6.0f);         // for other dir is 0,6,0,10
            Animation.RegisterAnimation("Hero_vfx", "Jump", "../Assets/spritesheet_player_vfx.png",         15, 11, 13, 0, 13, 4, 12.0f);
            Animation.RegisterAnimation("Hero_vfx", "Glide", "../Assets/spritesheet_player_vfx.png",        15, 11, 4, 0, 4, 2, 8.0f);
            Animation.RegisterAnimation("Hero_vfx", "Roll", "../Assets/spritesheet_player_vfx.png",         15, 11, 11, 0, 11, 5, 8.0f);
            Animation.RegisterAnimation("Hero_vfx", "Groundpound", "../Assets/spritesheet_player_vfx.png",  15, 11, 14, 0, 14, 2, 10.0f);
            Animation.RegisterAnimation("Hero_vfx", "Wallkick", "../Assets/spritesheet_player_vfx.png",     15, 11, 8, 0, 8, 3, 4.0f);
            Animation.RegisterAnimation("Hero_vfx", "Dash", "../Assets/spritesheet_player_vfx.png",         15, 11, 6, 0, 6, 2, 4.0f);
            //Animation.RegisterAnimation("Hero_vfx", "Wallslide", "../Assets/spritesheet_player_vfx.png", 15, 11, 4, 0, 4, 1, 4.0f);
            Animation.RegisterAnimation("Hero_vfx", "Falling", "../Assets/spritesheet_player_vfx.png",      15, 11, 12, 0, 12, 1, 4.0f);
            Animation.RegisterAnimation("Hero_vfx", "Land", "../Assets/spritesheet_player_vfx.png",         15, 11, 13, 6, 13, 8, 4.0f);
            Animation.RegisterAnimation("Hero_vfx", "Hurt", "../Assets/spritesheet_player_vfx.png",         15, 11, 3, 0, 3, 2, 3.0f);
            Animation.RegisterAnimation("Hero_vfx", "Bonk", "../Assets/spritesheet_player_vfx.png",         15, 11, 5, 0, 5, 2, 4.0f);
            Animation.RegisterAnimation("Hero_vfx", "Zipline", "../Assets/spritesheet_player_vfx.png",      15, 11, 7, 0, 7, 1, 4.0f);

            // Pickup VFX
            Animation.RegisterAnimation(pu_vfx, "Speed", "../Assets/spritesheet_player_vfx.png",            15, 11, 2, 4, 2, 10, 8.0f);
            Animation.RegisterAnimation(pu_vfx, "Time", "../Assets/spritesheet_player_vfx.png",             15, 11, 1, 4, 1, 9, 8.0f);
            Animation.RegisterAnimation(pu_vfx, "Clear", "../Assets/spritesheet_player_vfx.png",            15, 11, 14, 10, 14, 10, 0f);

            _isFacingRight = true;

            Physics.SetGravity(OwnerName, true);

            vec2 scale = Transform.GetScale(OwnerName);
            vec2 position = Transform.GetPosition(OwnerName);
            //_ziplineHeadOffset = position.y + (scale.y / 2);
            Physics.SetFriction(OwnerName, 0f);
            Physics.SetMass(OwnerName, 0.0001f);
            _moveVelocity = new vec2(0, 0);
            cam_defaultOffset = Camera.GetFollowOffset();
            Camera.SetZoom(0.5f);

            // starting anims
            //Animation.PlayAnimation(pu_vfx, "Clear");
        }

        bool vfxTracksPlayer;
        bool vfxInitialized;
        vec2 staticVfxPos;
        public override void OnUpdate(float deltaTime)
        {
            if (OwnerName == null || OwnerName == "") return;

            if (Input.GetKeyPressed(Input.KeyCode.L))
            {
                triggerSpeedBoost = true;
            }
            if (Input.GetKeyPressed(Input.KeyCode.Left))
            {
                Camera.SetFollowOffset(cam_defaultOffset.x + 200, 0);
            }
            if (Input.GetKeyPressed(Input.KeyCode.Right))
            {
                Camera.SetFollowOffset(cam_defaultOffset.x - 200, 0);
            }
            if (Input.GetKeyPressed(Input.KeyCode.P))
            {
                Animation.PlayAnimation(pu_vfx, "Clear");

                //clearing here bc in update
                NPCs.Clear();
                int totalGOs = GameObject.CountAll();
                for (int i = 1; i <= totalGOs; i++)
                {
                    string currGO = GameObject.FindGameObjectByID(i);       // gets name of GO
                    if (currGO.Contains("NPC_Lobster"))
                    {
                        NPCData temp = new NPCData((uint)i, Transform.GetPosition(currGO), false);
                        NPCs.Add(temp);
                    }
                }


                #region LOBSTER 3D SCISSORS SOUND
                // HARDCODDED LOBSTER SNIPPING SOUND FOR NOW
                // update 3D audio listener position to follow player

                vec2 playerPos = Transform.GetPosition(OwnerName);
                Audio.Set3DListenerPosition(playerPos.x, playerPos.y);
                for (int i = 0; i < NPCs.Count(); i++)
                {
                    vec2 npcPos = NPCs[i].pos;
                    Debug.Log("Pos of " + i.ToString() + "'s NPC: (" + npcPos.x.ToString() + ", " + npcPos.y.ToString() + ")");

                    float distX = playerPos.x - npcPos.x;
                    float distY = playerPos.y - npcPos.y;
                    float distanceSq = (distX * distX + distY * distY);

                    if (distanceSq < MoveStats.Audio3D_ActiveDistance * MoveStats.Audio3D_ActiveDistance)
                    {
                        if (!NPCs[i].audioPlaying)
                        {
                            Audio.PlaySound("scissors_snip", "lobster1_scissors");
                            NPCs[i].audioPlaying = true;
                            Debug.Log("=== LOBSTER 1 SCISSORS SOUND STARTED ===");
                        }

                        // update position, no spatial if above or below
                        float horizontalDist1 = Math.Abs(distX);
                        if (horizontalDist1 < 500.0f * 500.0f)
                        {
                            Audio.Set3DSoundPosition("lobster1_scissors", playerPos.x, NPCs[i].pos.y);
                        }
                        else
                        {
                            Audio.Set3DSoundPosition("lobster1_scissors", NPCs[i].pos.x, NPCs[i].pos.y);
                        }
                    }
                    else
                    {
                        if (NPCs[i].audioPlaying)
                        {
                            Audio.StopSound("lobster1_scissors");
                            NPCs[i].audioPlaying = false;
                            Debug.Log("=== LOBSTER 1 SCISSORS SOUND STOPPED ===");
                        }
                    }
                }

                #endregion

            }

            // Handle all timers
            _jumpBufferTimer -= deltaTime;
            _coyoteTimer -= deltaTime;
            _wallKickCooldownTimer -= deltaTime;
            _ziplineCoyoteTimer -= deltaTime;
            _groundPoundCooldownTimer -= deltaTime;
            _rollCooldownTimer -= deltaTime;
            _hurtTimer -= deltaTime;

            // additional checks here
            UpdatePlayerState();

            // toggle run
            if (MoveStats.UseToggleRun)
            {
                // toggle mode
                if (InputManager.RunWasPressed)
                {
                    _isRunToggleActive = !_isRunToggleActive;
                }
            }
            else
            {
                // hold mode
                _isRunToggleActive = InputManager.RunIsHeld;
            }


            JumpChecks();
            PlayFootsteps();
            // Handle Inputs
            HandleGeneralInput();
            OnFixedUpdate(fixeddt);

            switch(currState)
            {
                case PlayerState.Idle:
                    Animation.PlayAnimation(OwnerName, "Idle");
                    Animation.PlayAnimation(vfx, "Clear", false);
                    break;
                case PlayerState.Jumping:
                    Animation.PlayAnimation(OwnerName, "Jump");
                    Animation.PlayAnimation(vfx, "Jump", false);
                break;
                case PlayerState.Walking:
                    Animation.PlayAnimation(OwnerName, "Walk");
                    Animation.PlayAnimation(vfx, "Walk");
                break;
                case PlayerState.Running:
                    int index = GetCurrentTentacleLevel();
                    Animation.PlayAnimation(OwnerName, "Run" + index.ToString());
                    Animation.PlayAnimation(vfx, "Run" + index.ToString());
                    break;
                case PlayerState.Skidding:
                    Animation.PlayAnimation(OwnerName, "Skidding");
                    Animation.PlayAnimation(vfx, "Skidding");
                    break;
                case PlayerState.Falling:
                    Animation.PlayAnimation(OwnerName, "Falling");
                    Animation.PlayAnimation(vfx, "Falling");
                    break;
                case PlayerState.Land:
                    // commented out because landing is buggy
                    Animation.PlayAnimation(OwnerName, "Land");
                    Animation.PlayAnimation(vfx, "Land", false);
                    break;
                case PlayerState.GroundPounding:
                    Animation.PlayAnimation(OwnerName, "Groundpound_Init", false);
                    Animation.PlayAnimation(vfx, "Groundpound");
                    break;
                case PlayerState.GroundPounding_Falling:
                    Animation.PlayAnimation(OwnerName, "Groundpound");
                    Animation.PlayAnimation(vfx, "Groundpound");
                    break;
                case PlayerState.Rolling:
                    Animation.PlayAnimation(OwnerName, "Roll");
                    Animation.PlayAnimation(vfx, "Roll", true);
                    break;
                case PlayerState.WallKicking:
                    Animation.PlayAnimation(OwnerName, "Wallkick");
                    Animation.PlayAnimation(vfx, "Wallkick", false);
                    break;
                case PlayerState.Ziplining:
                    Animation.PlayAnimation(OwnerName, "Zipline");
                    Animation.PlayAnimation(vfx, "Zipline");
                    break;
                case PlayerState.Gliding:
                    Animation.PlayAnimation(OwnerName, "Glide");
                    Animation.PlayAnimation(vfx, "Glide");
                    break;
                case PlayerState.WallSliding:
                    Animation.PlayAnimation(OwnerName, "Wallslide");
                    Animation.PlayAnimation(vfx, "Wallslide");
                    Animation.PlayAnimation(vfx, "Clear", false);
                    break;
                case PlayerState.WallJumping:
                    Animation.PlayAnimation(OwnerName, "Wallkick");
                    Animation.PlayAnimation(vfx, "Wallkick", false);
                    break;
                case PlayerState.Hurting:
                    Animation.PlayAnimation(OwnerName, "Hurt");
                    Animation.PlayAnimation(vfx, "Hurt");
                    break;
                case PlayerState.Bonking:
                    Animation.PlayAnimation(OwnerName, "Bonk");
                    Animation.PlayAnimation(vfx, "Bonk");
                    break;
            }

            // WAS ORIGINALLY IN FIXEDUPDATE BUT HAD ISSUES WITH FRAME TIMINGS SO MOVED HERE!
            // set vfx pos to player
            vfxTracksPlayer = nonTrackingVFX.Contains(currState) ? false : true;
            vec2 pos = Transform.GetPosition(OwnerName);
            if (!vfxInitialized)
            {
                if (!vfxTracksPlayer)
                {
                    staticVfxPos = pos;
                }

                vfxInitialized = true;
            }

            vec2 finalPos = vfxTracksPlayer ? pos : staticVfxPos;
            if (currState == PlayerState.Running || currState == PlayerState.Skidding)
                finalPos.x -= 100f * _facingDirection;
            else if (currState == PlayerState.Falling)
                finalPos.y += 100f;


            Transform.SetPosition(vfx, finalPos.x, finalPos.y - 10f);
            Transform.SetPosition(pu_vfx, pos.x, pos.y);
        }

        static bool resetSkid = false;
        public void OnFixedUpdate(float fixedDeltaTime)
        {
            //Debug.Log(currState.ToString());
            //Debug.Log(_isGrounded.ToString());
            //if (_isFacingRight) Debug.Log("right");
            //else Debug.Log("left");

            // check if player is taking damage
            bool isHurting = IsBeingKnockedBack; // = (_playerHealth != null && _playerHealth.IsHurting);

            // wall bonk stun
            if (_bonkTimer > 0)
            {
                _bonkTimer -= fixeddt;
                if (_bonkTimer <= 0)
                {
                    currState = PlayerState.Idle;
                }
                else
                {
                    currState = PlayerState.Bonking;
                }
                CollisionChecks();
                Jump();

                // stop sliding if player hits the ground while stunned - apply massive friction
                if (_isGrounded)
                {
                    float bonkFriction = 1000f;
                    float newX = Utils.MathfMoveTowards(Physics.GetVelocity(OwnerName).x, 0, bonkFriction * fixeddt);
                    Physics.SetVelocity(OwnerName, newX, Physics.GetVelocity(OwnerName).y);
                }
                return;
            }

            CollisionChecks();

            if (_isGrounded) _groundedTimer += fixeddt;
            else _groundedTimer = 0f;

            HandleBlinkEffect();

            if (!_isZiplining || !IsBeingKnockedBack) // && !_isBeingLaunched)
            {
                //speed boost
                if (triggerSpeedBoost)
                {
                    triggerSpeedBoost = false;
                    speedBoostActive = true;
                    speedTimer = MoveStats.Speed_Duration;
                }
                if (speedBoostActive)
                {
                    HandleSpeedBoost();
                }

                // gravity logic
                Jump();

                HandleKnockback();
                HandleHazardCollision();
                // hurt movement lock & friction
                if (isHurting)
                {
                    // reset internal speed so player cannot instantly sprint after recovering
                    movespeed = 0;
                    _isSkidding = false;

                    // only apply friction on ground
                    if (_isGrounded)
                    {
                        float newX = Utils.MathfMoveTowards(Physics.GetVelocity(OwnerName).x, 0, MoveStats.KnockbackFriction.x * fixeddt);
                        Physics.SetVelocity(OwnerName, newX, Physics.GetVelocity(OwnerName).y);
                    }

                    return;
                }

                // wall bonk trigger
                if (_isTouchingWall && _isGrounded && movespeed >= MoveStats.T3_Threshold && currState != PlayerState.WallJumping && currState != PlayerState.WallSliding)
                {
                    HandleWallBonk();
                    return;
                }

                // skid lock
                if (_isSkidding)
                {
                    if (!resetSkid)
                    {
                        currState = PlayerState.Skidding;
                        EnterState(currState);
                        resetSkid = true;
                    }

                    HandleLockedSkid();
                    return;
                }
                else
                {
                    resetSkid = false;
                }

                // normal abilities
                if (!isHurting)
                {
                    WallKick();
                    GroundPound();
                    Roll();
                    Glide();
                    WallSlide();
                }

                // player movement
                if (!_isRunning && !_isWallKicking && !_isWallJumping && !_isGroundPounding && !_isWallSliding && !_isRolling && !isHurting)
                {
                    Move(InputManager.Movement);
                }
            }
        }

        private float landAnimTimerBuffer;
        #region State & Input
        private void UpdatePlayerState()
        {
            if (currState == PlayerState.Land)
            {
                landAnimTimerBuffer -= fixeddt;

                if (landAnimTimerBuffer < 0f)
                {
                    currState = PlayerState.Idle;
                }

                return; // stay locked in Land
            }
            else
            {
                landAnimTimerBuffer = landAnimationLength;
            }
            if (Physics.GetVelocity(OwnerName).x == 0 && currState != PlayerState.Jumping && currState != PlayerState.WallSliding && currState != PlayerState.Bonking && currState != PlayerState.Falling)
            {
                currState = PlayerState.Idle;
            }

            // horizontal input = moving - trigger run animation
            if ((InputManager.Movement.x != 0 || InputManager.RunIsHeld) && currState != PlayerState.Jumping && currState != PlayerState.WallSliding && currState != PlayerState.Rolling && currState != PlayerState.Falling)
            {
                int index = GetCurrentTentacleLevel();
                if (index == 0)
                {
                    currState = PlayerState.Walking;
                }
                else
                {
                    currState = PlayerState.Running;
                }
            }
            if (_isSkidding)
            {
                //Turn(InputManager.Movement.x < 0);
                currState = PlayerState.Skidding;
            }
        }

        static bool jumpFullyReleased = false;
        private void HandleGeneralInput()
        {
            bool isControlLocked = _bonkTimer > 0 || _isSkidding || currState == PlayerState.Hurting;
            if (isControlLocked)
            {
                return;
            }
            //if (InputManager.WallKickWasPressed && _wallKickCooldownTimer <= 0 && canWallKick)
            //{
            //    InitiateWallKick();
            //}
            if (InputManager.GroundPoundWasPressed && !_isGrounded)
            {
                InitiateGroundPound();
            }
            if (InputManager.RollWasPressed)
            {
                //InitiateRoll();
            }

            // oh my god gliding and jumping with space is so damn mafan
            // sets flag to enable 1 input of InitGlide()
            if (InputManager.JumpWasReleased && _numberOfJumpsUsed >= MoveStats.NumberOfJumpsAllowed)
            {
                jumpFullyReleased = true;
            }

            // only enable gliding when exhausted jumps
            if (jumpFullyReleased && InputManager.JumpWasPressed && !_isGrounded && !_isGliding)
            {
                //InitiateGlide();
                jumpFullyReleased = false; 
            }

            // release space - cancel glide
            if (_isGliding && InputManager.JumpWasReleased)
            {
                //StopGlide();
            }

            if (_isGrounded)
            {
                jumpFullyReleased = false;
            }
        }

        #endregion

        #region Movement
        public void Move(vec2 moveInput)
        {
            //if (IsBeingKnockedBack || currState == PlayerState.Bonking)       // removed bc causing bug
            //{
            //    return;
            //}
            if (_isWallJumping || _wallJumpTimer > 0f && InputManager.JumpWasPressed || currState == PlayerState.WallSliding) return;

            float inputX = moveInput.x;
            bool isRunHeld = _isRunToggleActive; // for run toggle option

            int targetDirection = 0;
            if (inputX != 0) targetDirection = (int)inputX;
            else if (isRunHeld && _isGrounded) targetDirection = _facingDirection;

            // landing skid check - for jump flip
            // if hit groud + fast speed BUT facing opposite direction of movement = landing skid
            bool movingAgainstFacing = Math.Sign(Physics.GetVelocity(OwnerName).x) != _facingDirection;
            bool isMovingFast = Math.Abs(Physics.GetVelocity(OwnerName).x) >= MoveStats.T1_Threshold;            // small threshold (1f) to ignore tiny jitters when stopping

            if (_isGrounded && isMovingFast && movingAgainstFacing && !_isSkidding)
            {
                // direct skid trigger
                _isSkidding = true;
                _facingDirection = targetDirection != 0 ? targetDirection : _facingDirection;

                // standard skid setup
                int landingTier = GetCurrentTentacleLevel();
                if (isRunHeld && landingTier == 0) landingTier = 1;

                if (landingTier > 0)
                {
                    float duration = MoveStats.T1_SkidDuration;
                    if (landingTier == 2) duration = MoveStats.T2_SkidDuration;
                    if (landingTier == 3) duration = MoveStats.T3_SkidDuration;
                    _skidTimer = duration;

                    movespeed *= MoveStats.SkidSpeedRetained;
                    _calculatedSkidDecel = (duration > 0) ? (movespeed / duration) : 0;
                    return;
                }
            }
            // determine max speed
            float currentMaxSpeed = isRunHeld ? MoveStats.T3_MaxSpeed : MoveStats.T0_MaxSpeed;
            if (speedBoostActive) 
                currentMaxSpeed *= MoveStats.Speed_Multiplier;

            // movement logic
            if (targetDirection != 0)
            {
                bool isOpposite = (targetDirection > 0 && _facingDirection == -1) || (targetDirection < 0 && _facingDirection == 1);

                // turn check
                if (isOpposite)
                {
                    int tier = GetCurrentTentacleLevel();

                    // air turn (visual flip only) - if in air + run held / toggled + moving fast
                    // CHANGE : if (!_isGrounded && isRunHeld && movespeed > MoveStats.T0_MaxSpeed)
                    if (!_isGrounded && movespeed > MoveStats.T0_MaxSpeed)
                    {
                        // flip sprite
                        Turn(targetDirection > 0);
                        _facingDirection = targetDirection;
                    }

                    // ground skid
                    else
                    {
                        // force skid if holding / toggling run
                        if (isRunHeld && movespeed > 5f && tier == 0) tier = 1;

                        if (tier == 0 || !isMovingFast) //Math.Abs(Physics.GetVelocity(OwnerName).x) >= MoveStats.T1_Threshold * 0.9f;  
                        {
                            // nstant turn
                            movespeed = MoveStats.TurnResetSpeed;
                            _facingDirection = targetDirection;
                            Turn(targetDirection > 0);
                            _isSkidding = false;
                        }
                        else
                        {
                            Debug.Log("here");
                            _isSkidding = true;
                            _facingDirection = targetDirection;
                            Turn(targetDirection > 0);

                            float duration = MoveStats.T1_SkidDuration;
                            if (tier == 2) duration = MoveStats.T2_SkidDuration;
                            if (tier == 3) duration = MoveStats.T3_SkidDuration;

                            _skidTimer = duration;
                            movespeed *= MoveStats.SkidSpeedRetained;


                            // skid will stop at "exitspeed" so player doesnt drop down to 0
                            float exitSpeed = MoveStats.SkidExitSpeed;

                            if (movespeed > exitSpeed)
                            {
                                // cecelerate until hit 4
                                _calculatedSkidDecel = (movespeed - exitSpeed) / duration;
                            }
                            else
                            {
                                // if already slow = stop normally
                                _calculatedSkidDecel = movespeed / duration;
                            }

                            return;
                        }
                    }
                }

                _isSkidding = false;

                // snap Logic
                float startSnapSpeed = isRunHeld ? MoveStats.RunBaseSpeed : MoveStats.T0_BaseSpeed;
                if (movespeed < startSnapSpeed) movespeed = startSnapSpeed;

                // adjust speed
                if (movespeed < currentMaxSpeed)
                {
                    float accel = isRunHeld ? MoveStats.TentAcceleration : MoveStats.GroundAcceleration;
                    movespeed += accel * fixeddt;
                }
                else if (movespeed > currentMaxSpeed)
                {
                    // coasting - smooth decel to walk speed
                    float coastDrag = _isGrounded ? MoveStats.RunCoastDeceleration : MoveStats.AirDeceleration;
                    movespeed -= coastDrag * fixeddt;
                    if (movespeed < currentMaxSpeed) movespeed = currentMaxSpeed;
                }
            }
            else
            {
                _isSkidding = false;
                float currDeceleration = _isGrounded ? MoveStats.GroundDeceleration : MoveStats.AirDeceleration;
                movespeed -= currDeceleration * fixeddt;
                if (movespeed < 0) movespeed = 0;
            }

            // physics application 
            movespeed = Utils.MathfClamp(movespeed, 0, MoveStats.Clamp_MaxRunSpeed);

            // determine physics direction
            int moveDir = _facingDirection;

            // if in air + moving fast + turned in the opposite direction = flip the physics application
            // use rb's current velocity to detect the "real movement" direction

            // CHANGE : if (!_isGrounded && isRunHeld && movespeed > MoveStats.T0_MaxSpeed)
            if (!_isGrounded && movespeed > MoveStats.T0_MaxSpeed)
            {
                // if facing right (1) but moving left (vel < -1) -> apply force left (-1)
                if (_facingDirection == 1 && Physics.GetVelocity(OwnerName).x < -1f)
                    moveDir = -1;
                else if (_facingDirection == -1 && Physics.GetVelocity(OwnerName).x > 1f)
                    moveDir = 1;
            }

            _moveVelocity.x = movespeed * moveDir;
            if (speedBoostActive)
            {
                _moveVelocity.x *= MoveStats.Speed_Multiplier;
                _moveVelocity.y *= MoveStats.Speed_Multiplier;
            }
            Physics.SetVelocity(OwnerName, _moveVelocity.x, Physics.GetVelocity(OwnerName).y);
        }

        private void Turn(bool turnRight)
        {
            _isFacingRight = turnRight;
            vec2 currentScale = Transform.GetScale(OwnerName);
            // [WIP] to include left and right animations for accurate monocle, for now will just do a basic flip
            Transform.SetScale(OwnerName, Math.Abs(currentScale.x) * (turnRight ? 1 : -1), currentScale.y);

            //vfx
            vec2 scale = Transform.GetScale(vfx);
            Transform.SetScale_XY(vfx, Math.Abs(scale.x) * (turnRight ? 1 : -1), scale.y);
        }

        private int GetCurrentTentacleLevel()
        {
            if (movespeed >= MoveStats.T3_Threshold) return 3;
            if (movespeed >= MoveStats.T2_Threshold) return 2;
            if (movespeed >= MoveStats.T1_Threshold) return 1;
            return 0; // walk speed
        }

        private void HandleLockedSkid()
        {
            // skid timer
            _skidTimer -= fixeddt;

            // friction
            movespeed -= _calculatedSkidDecel * fixeddt;

            // safety clamp to not go negative during skid
            if (movespeed < 0) movespeed = 0;

            // end check
            if (_skidTimer <= 0)
            {
                //movespeed = 0;
                _isSkidding = false;
            }

            // apply backwards velocity
            // flipped _facingDirection to look where player wants to go but have momentum drag them in the opposite way (multiply by negative facing direction)
            _moveVelocity.x = movespeed * -_facingDirection;
            Physics.SetVelocity(OwnerName, _moveVelocity.x, Physics.GetVelocity(OwnerName).y);
        }

        private void HandleWallBonk()
        {
            currState = PlayerState.Bonking;
            EnterState(currState);

            // force roll + ground pound states to end immediately
            if (_isRolling) StopRoll();
            _isRolling = false;
            _isGroundPounding = false;

            // unlock movement block from roll / ground pound
            blockMovement = false;

            movespeed = 0;
            _isSkidding = false;

            // auto disable run toggle on crash
            if (MoveStats.UseToggleRun) _isRunToggleActive = false;

            // use T3 variable directly since bonk only happens at top speed
            Vector2 force = MoveStats.T3_BonkForce;

            vec2 currentPos = Transform.GetPosition(OwnerName);
            Transform.SetPosition(OwnerName,
                currentPos.x + (-_facingDirection * 5f),   // nudge opposite to facing (away from wall)
                currentPos.y);

            Physics.SetVelocity(OwnerName, -_facingDirection * force.X, force.Y);
            _bonkTimer = MoveStats.T3_BonkDuration;
        }

        #endregion

        #region Jump

        private void Jump()
        {
            //if (_isWallJumping && VerticalVelocity <= 0)
            //{
            //    _isWallJumping = false;
            //    blockMovement = false; // give back control
            //}

            // apply gravity while jumping
            if (_isJumping)
            {
                currState = PlayerState.Jumping;
                if (_isTouchingWall)
                {
                    currState = PlayerState.WallSliding;
                }
                // Head bump detection – unchanged
                if (_bumpedHead)
                {
                    _isFastFalling = true;
                }

                // gravity on ascending 
                if (VerticalVelocity >= 0f)
                {
                    // apex controls
                    _apexPoint = Utils.MathfInverseLerp(MoveStats.InitialJumpVelocity, 0f, VerticalVelocity);

                    if (_apexPoint > MoveStats.ApexThreshold)
                    {
                        if (!_isPastApexThreshold)
                        {
                            _isPastApexThreshold = true;
                            _timePastApexThreshold = 0f;
                        }

                        if (_isPastApexThreshold)
                        {
                            _timePastApexThreshold += fixeddt;

                            if (_timePastApexThreshold < MoveStats.ApexHangTime)
                            {
                                VerticalVelocity = 0f;
                            }
                            else
                            {
                                VerticalVelocity = -0.01f;
                            }
                        }
                    }

                    // gravity on descending but not past apex threshold
                    else
                    {
                        VerticalVelocity += MoveStats.Gravity * fixeddt;
                        if (_isPastApexThreshold)
                        {
                            _isPastApexThreshold = false;
                        }
                    }
                }
                else
                {
                    if (!_isWallSliding)
                        currState = PlayerState.Falling;

                    // gravity on descending
                    if (!_isFastFalling)
                    {
                        VerticalVelocity += MoveStats.Gravity * MoveStats.GravityOnReleaseMultiplier * fixeddt;
                    }

                    if (VerticalVelocity < 0f)
                    {
                        if (!_isFalling)
                        {
                            _isFalling = true;
                        }
                    }
                }
            }

            // jump cut
            if (_isFastFalling)
            {
                if (_fastFallTime >= MoveStats.TimeForUpwardsCancel)
                {
                    VerticalVelocity += MoveStats.Gravity * MoveStats.GravityOnReleaseMultiplier * fixeddt;
                }
                else if (_fastFallTime < MoveStats.TimeForUpwardsCancel)
                {
                    VerticalVelocity = Utils.MathfLerp(_fastFallReleaseSpeed, 0f, (_fastFallTime / MoveStats.TimeForUpwardsCancel));
                }

                _fastFallTime += fixeddt;
            }

            // normal gravity while falling
            if (!_isGrounded && !_isJumping)
            {
                if (!_isFalling)
                {
                    _isFalling = true;
                }

                VerticalVelocity += MoveStats.Gravity * fixeddt;
            }

            // clamp fall speed
            VerticalVelocity = Utils.MathfClamp(VerticalVelocity, -MoveStats.MaxFallSpeed, MoveStats.Clamp_MaxJumpHeight);

            Physics.SetVelocity(OwnerName, Physics.GetVelocity(OwnerName).x, VerticalVelocity);
        }

        private void JumpChecks()
        {
            // button press buffer
            if (InputManager.JumpWasPressed)
            {
                _jumpBufferTimer = MoveStats.JumpBufferTime;
                _jumpReleasedDuringBuffer = false;
            }

            // variable jump height logic
            if (MoveStats.EnableVariableJumpHeight && InputManager.JumpWasReleased)
            {
                if (_jumpBufferTimer > 0f) _jumpReleasedDuringBuffer = true;
                if (_isJumping && VerticalVelocity > 0f)
                {
                    _isFastFalling = true;
                    _fastFallReleaseSpeed = VerticalVelocity;
                }
            }

            // wall logic - if buffered jump exists & within coyote time of wall
            if (_jumpBufferTimer > 0f && _wallJumpTimer > 0f)
            {
                //// determine if pressing opposite to current facing direction
                //// facing right (1) - input left (-1) = true
                //// facing right (1) - input neutral (0) = false
                //bool isPressingOpposite = (InputManager.Movement.x != 0) && (InputManager.Movement.x != _facingDirection);

                //if (isPressingOpposite)
                //{
                //    // input = away from wall = wall kick
                //    InitiateWallKick();
                //}
                //else
                //{
                //    // input = neutral / towards wall = wall jump
                //    InitiateWallJump();
                //}

                //_jumpBufferTimer = 0f; // consume input

                InitiateWallJump();
                _jumpBufferTimer = 0f;
            }

            // normal ground jump
            else if (_jumpBufferTimer > 0f && !_isJumping && (_isGrounded || _coyoteTimer > 0f))
            {
                InitiateJump(1);
                if (_jumpReleasedDuringBuffer)
                {
                    _isFastFalling = true;
                    _fastFallReleaseSpeed = VerticalVelocity;
                }
            }

            // double jump / fall jump logic
            else if (_jumpBufferTimer > 0f && _isJumping && _numberOfJumpsUsed < MoveStats.NumberOfJumpsAllowed)
            {
                _isFastFalling = false;
                InitiateJump(1);
            }
            else if (_jumpBufferTimer > 0f && _isFalling && _numberOfJumpsUsed < MoveStats.NumberOfJumpsAllowed - 1)
            {
                InitiateJump(2);
                _isFastFalling = false;
            }

            // landing
            if ((_isJumping || _isFalling) && _isGrounded && VerticalVelocity <= 0f)
            {
                if (currState != PlayerState.Rolling)
                {
                    currState = PlayerState.Land;
                    EnterState(currState);
                }

                // wallkick() grounded check
                if (_isWallKicking) StopWallKick();

                // consume buffered jump upon landing
                _jumpBufferTimer = 0f;

                _isJumping = false;
                _isFalling = false;
                _isFastFalling = false;
                _fastFallTime = 0f;
                _isPastApexThreshold = false;
                _numberOfJumpsUsed = 0;

                if (!_isWallJumping)
                {
                    blockMovement = false;
                }
                _isWallJumping = false;

                _ziplineCoyoteTimer = 0f;

                VerticalVelocity = Physics.gravity.y;
            }
        }

        private void InitiateJump(int numberOfJumpsUsed)
        {
            if (!_isGrounded || numberOfJumpsUsed > MoveStats.NumberOfJumpsAllowed || _isSkidding)
            {
                _isJumping = false;
                return;
            }

            currState = PlayerState.Jumping;
            EnterState(currState);

            if (_isRolling)
            {
                StopRoll();
            }

            if (!_isJumping)
            {
                fJump = true;
                _isJumping = true;
            }

            _jumpBufferTimer = 0f;
            _numberOfJumpsUsed += numberOfJumpsUsed;

            // calculate jump velocity based on tent speed
            float jumpMultiplier = 1f; // default (walk / idle)
            int tent = GetCurrentTentacleLevel();

            if (tent == 1)
            {
                jumpMultiplier = MoveStats.T1_JumpMultiplier;
            }
            else if (tent == 2)
            {
                jumpMultiplier = MoveStats.T2_JumpMultiplier;
            }
            else if (tent == 3)
            {
                jumpMultiplier = MoveStats.T3_JumpMultiplier;
            }

            // apply base velocity * tent multiplier
            VerticalVelocity = MoveStats.InitialJumpVelocity * jumpMultiplier;

            // CHANGE - MAX SPEED = HIGHER JUMP VerticalVelocity = MoveStats.InitialJumpVelocity;

        }

        #endregion

        #region Slide
        static bool wallslideTrigger = false;
        static float wallslideTime = 0f;
        private void WallSlide()
        {
            if (_isGrounded)                    // flip when grounded on wall slide
            {
                if (_isWallSliding)
                {
                    _isWallSliding = false;
                    _facingDirection = _facingDirection == 1 ? -1 : 1;
                    Turn(_facingDirection == 1);
                    Physics.SetVelocity(OwnerName, 0f,0f);
                    movespeed = 0;
                }
                return;
            }

            if (IsBeingKnockedBack || InputManager.FastDropWasPressed)
            {
                if (_isWallSliding)
                {
                    _isWallSliding = false;
                }
                return;
            }

            // check collision & falling speed
            if (_isTouchingWall && Physics.GetVelocity(OwnerName).y < 0)
            {
                // first frame logic
                if (!_isWallSliding)
                {
                    _isWallSliding = true;

                    currState = PlayerState.WallSliding;
                    EnterState(currState);
                    StopRoll();
                    Physics.SetVelocity(OwnerName, 0f, 0f);

                    // reset jumps if want wall sliding to refresh double jump
                    _numberOfJumpsUsed = 0;
                }
                
                if (wallslideTime >= MoveStats.WallSlidingTimeToTriggerSound)         // magic number replace with a
                {
                    if (wallslideTrigger == false)
                    {
                        Audio.PlaySound("wallslide_sfx", "sfx");
                        wallslideTrigger = true;
                    }
                }
                wallslideTime += fixeddt;

                // apply friction/slide speed
                Physics.SetVelocity(OwnerName, 0f, Math.Max(Physics.GetVelocity(OwnerName).y, -MoveStats.WallSlidingSpeed));
            }
            else
            {
                // exit wall slide logic
                if (_isWallSliding)
                {
                    _isWallSliding = false;
                }
                wallslideTrigger = false;
                wallslideTime = 0;
            }
        }

        #endregion


        #region Wall Jump + Kick

        private void InitiateWallJump()
        {
            //play wall jump sound

            _isWallJumping = true;
            blockMovement = true;
            _isWallSliding = false;
            _isJumping = true;
            _isSkidding = false;
            _numberOfJumpsUsed = 1; // CHANGE

            //testing
            _skidTimer = 0f;
            _calculatedSkidDecel = 0f;
            resetSkid = false;

            currState = PlayerState.WallJumping;
            EnterState(currState);

            // direction - calculate away from wall [facing right (1) - wall on right - jump left (-1)]
            int wallSide = _isFacingRight ? 1 : -1;
            int jumpDir = -wallSide;

            // flip to jump away from wall
            Turn(jumpDir == 1);
            _facingDirection = jumpDir;
            _isFacingRight = jumpDir == 1;

            // force - restore stats values
            VerticalVelocity = MoveStats.WallJumpForce.y;

            // momentum - set speed so Move() picks it up upon landing
            movespeed = MoveStats.WallJumpForce.x;

            // apply
            _moveVelocity.x = movespeed * jumpDir;

            vec2 currentPos = Transform.GetPosition(OwnerName);
            Transform.SetPosition(OwnerName,
                currentPos.x + (_facingDirection * 5f),   // 5f = small pixel nudge away from wall
                currentPos.y);

            Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);
        }

        // NOT UPDATED SINCE M5!!! BC NOT IN USE ANYMORE
        private void InitiateWallKick()
        {
            if (_isWallKicking) return;     // remove this if causing issues - not in unity

            _isWallKicking = true;
            currState = PlayerState.WallKicking;
            EnterState(currState);

            fWallKick = true;
            movespeed = MoveStats.WallJumpForce.x;
            //blockMovement = true; // lock movement

            // timers
            _wallKickTotalDuration = MoveStats.WKBaseDuration * MoveStats.WKDurationMultiplier;
            _wallKickTimer = _wallKickTotalDuration;
            _wallKickCooldownTimer = MoveStats.WKCooldown;

            // direction - kick away from wall
            int kickDir = _isFacingRight ? -1 : 1;

            // sync visuals with kick direction (immediate flip)
            Turn(kickDir == 1);
            _facingDirection = kickDir;

            // set speed (high speed wall kick)
            movespeed = MoveStats.WKSpeedModifier;

            // apply velocity
            _moveVelocity.x = movespeed * _facingDirection;

            // add a little vertical hop
            _moveVelocity.y = MoveStats.WKVerticalModifier;

            Physics.SetVelocity(OwnerName, _moveVelocity.x, _moveVelocity.y);
        }

        private void WallKick()
        {
            if (!_isWallKicking) return;            // remove if causing issues

            currState = PlayerState.WallKicking;
            blockMovement = true;
            _isJumping = false;
            _wallKickTimer -= fixeddt;

            // calculate progress through the kick (from 0.0 @ start to 1.0 @ end)
            float progress = 1f - (_wallKickTimer / _wallKickTotalDuration);

            // we dont have this :(
            // get speed multiplier from curve @ current progress point
            //float curveMultiplier = MoveStats.WKEaseCurve.Evaluate(progress);

            // actual speed is peak speed multiplied by curve's value
            float currentSpeed = _moveVelocity.x;               // * curveMultiplier;
            float verticalGain = MoveStats.WKVerticalModifier;  // * curveMultiplier;

            // apply new eased speed
            // to fix the jank vert momentum, we just ignore gravity for now, and give back control to the grav after wall kicks
            // can change this to a var!
            Physics.SetVelocity(OwnerName, currentSpeed, verticalGain);

            // check if the kick's duration has finished
            if (_wallKickTimer <= 0)
            {
                StopWallKick();
            }
        }

        private void StopWallKick()
        {
            if (!_isWallKicking) return;        // remove if causing issues

            _isWallKicking = false;
            blockMovement = false; // allow movement

            // reset Y velocity but keep X momentum
            Physics.SetVelocity(OwnerName, Physics.GetVelocity(OwnerName).x, Physics.GetVelocity(OwnerName).y);

            //wallKickFrameCounter = 0;
        }
        #endregion

        #region Roll

        private void InitiateRoll(float speedOverride = -1f, float durationOverride = -1f)
        {
            if (_isRolling || !_isGrounded || _rollCooldownTimer > 0) return;         // remove if causing issue

            currState = PlayerState.Rolling;
            EnterState(currState);

            fRoll = true;
            _isRolling = true;
            blockMovement = true;
            _rollCooldownTimer = MoveStats.RollCooldown;

            int tent = GetCurrentTentacleLevel();

            // speed logic
            if (speedOverride > 0)
            {
                // ground pound force
                movespeed = speedOverride;
            }
            else
            {
                // normal roll - tent speeds 0 to 3
                if (tent == 3)
                {
                    movespeed = MoveStats.MaxRollSpeed;
                }
                else if (tent >= 1) // Tent 1 or 2
                {
                    movespeed = MoveStats.MidRollSpeed;
                }
                else // tent 0
                {
                    movespeed = MoveStats.BaseRollSpeed;
                }
            }

            // duration logic
            if (durationOverride > 0)
            {
                _rollTimer = durationOverride;
            }
            else
            {
                if (tent == 3) _rollTimer = MoveStats.MaxRollDuration;
                else if (tent >= 1) _rollTimer = MoveStats.MidRollDuration;
                else _rollTimer = MoveStats.BaseRollDuration;
            }

            //_isGroundPounding = false;            // wasnt inside unity so idk

            // apply velocity
            _moveVelocity.x = movespeed * _facingDirection;
            _moveVelocity.y = 0;
            Physics.SetVelocity(OwnerName, _moveVelocity.x, 0);
        }

        private void Roll()
        {
            // Cancel roll if touching wall in air
            if (_isRolling)
            {
                if (_isTouchingWall || _rollTimer <= 0 || _isWallJumping || _isWallKicking)
                {
                    StopRoll();
                    return;
                }

                currState = PlayerState.Rolling;
                _rollTimer -= fixeddt;

                // constant velocity (no friction) - just keep moving at set movespeed for whole duration
                _moveVelocity.x = movespeed * _facingDirection;
                Physics.SetVelocity(OwnerName, _moveVelocity.x, Physics.GetVelocity(OwnerName).y);
            }
        }

        private void StopRoll()
        {
            if (!_isRolling) return;

            _isRolling = false;
            blockMovement = false; // allow movement

            if (_isTouchingWall || _isWallJumping || _isWallKicking)
            {
                _moveVelocity = new vec2(0, 0);
            }
        }

        #endregion

        #region Ground Pound
        private void InitiateGroundPound()
        {
            if (_isGroundPounding || _isGliding || _isWallSliding || _isGrounded || _groundPoundCooldownTimer > 0) return;

            currState = PlayerState.GroundPounding;
            EnterState(currState);

            fGroundPound = true; // time deduction
            _isGroundPounding = true;
            _groundPoundCooldownTimer = MoveStats.GPCooldown;

            _groundPoundPauseTimer = MoveStats.GPPauseTime;

            // kill all momentum for in air freeze
            _moveVelocity.x = 0;
            _moveVelocity.y = 0;

            // set vel to vector.zero
            Physics.SetVelocity(OwnerName, 0, 0);

            // store position where groundpound initiated
            lastPos = transform.position;
        }

        static bool gp_falling = false;
        private void GroundPound()
        {
            if (_isGroundPounding)
            {
                if (_groundPoundPauseTimer > 0 && !_isWallSliding && !_isGrounded)
                {
                    // freeze in air
                    // maybe pause sound effect?
                    currState = PlayerState.GroundPounding;
                    _groundPoundPauseTimer -= fixeddt;
                    Physics.SetVelocity(OwnerName, 0,0);
                    transform.position = lastPos;
                    gp_falling = false;
                }
                else
                {
                    if (_isGrounded || _isWallSliding)
                    {
                        blockMovement = false;
                        gp_falling = false;
                        StopGroundPound();
                    }
                    else
                    {
                        if (!gp_falling)
                        {
                            currState = PlayerState.GroundPounding_Falling;
                            EnterState(currState);
                        }
                        gp_falling = true;
                        currState = PlayerState.GroundPounding_Falling;
                        _moveVelocity.y = -MoveStats.GPFallSpeed;
                        Physics.SetVelocity(OwnerName, 0, _moveVelocity.y);
                        CollisionChecks();
                    }
                }
            }
        }

        private void StopGroundPound()
        {
            // reset vertical velocity
            _moveVelocity.y = 0;
            Physics.SetVelocity(OwnerName, 0, 0);

            // calculate height
            float fallHeight = lastPos.y - transform.position.y;
            Debug.Log(fallHeight.ToString());

            // roll speed logic
            float targetRollSpeed;

            // exception: if falling from very high (no matter what tent speed), use max roll speed
            if (fallHeight > MoveStats.GroundPoundMaxSpeedHeightThreshold)
            {
                targetRollSpeed = MoveStats.MaxRollSpeed;
            }
            else
            {
                // standard - use tent speed player had before ground pounding
                int tier = GetCurrentTentacleLevel();

                if (tier == 3)
                {
                    targetRollSpeed = MoveStats.MaxRollSpeed;
                }
                else if (tier >= 1) // tent 1 or 2
                {
                    targetRollSpeed = MoveStats.MidRollSpeed;
                }
                else // tent 0 (still / walk)
                {
                    // ensure player rolls forward a little @ base speed
                    targetRollSpeed = MoveStats.BaseRollSpeed;
                }
            }

            // duration logic - base time + (height / divisor)  
            float targetDuration = MoveStats.BaseRollDuration + (fallHeight / MoveStats.RollHeightToDurationDivisor);
            targetDuration = targetDuration > MoveStats.MaxRollDuration ? MoveStats.MaxRollDuration : targetDuration;           // clamp at maxrollduration

            // direction - if pressing A/D = roll that way : if pressing nothing = roll facing direction
            if (InputManager.Movement.x != 0)
            {
                int inputDir = (int)InputManager.Movement.x;
                _facingDirection = inputDir;
                Turn(inputDir > 0);
            }

            _isGroundPounding = false;
            Debug.Log("\n\ntargetRollSpeed: " + targetRollSpeed + "targetDuration: " + targetDuration + "\n\n");

            // trigger roll with calculated speed + duration
            InitiateRoll(targetRollSpeed, targetDuration);
        }
        #endregion

        #region Air Glide

        private void InitiateGlide()
        {
            // If cooldown not ended
            if (_isGliding || _glideCooldownTimer > 0 || _isWallKicking || _isGroundPounding || _isGrounded || gp_falling || _isTouchingWall) return;

            currState = PlayerState.Gliding;
            EnterState(currState);
            _isGliding = true;
        }

        private void Glide()
        {
            if (_glideCooldownTimer > 0)
            {
                _glideCooldownTimer -= fixeddt;
            }

            // if cooldown ready + input pressed
            if (_isGliding)
            {
                currState = PlayerState.Gliding;

                // cooldown ready
                // cancel if hit ground or wallsliding
                if (_isGrounded || _isRunning || _isWallSliding)
                {
                    StopGlide();
                }
                else
                {
                    // Timer for periodic time deduction
                    _timeSinceLastGlideDeduction += fixeddt;
                    if (_timeSinceLastGlideDeduction >= MoveStats.GlideDeductionInterval)
                    {
                        fGlide = true; // Set the flag for the TimerManager
                        _timeSinceLastGlideDeduction = 0f; // Reset the timer
                    }

                    // only apply the glide effect when falling
                    if (VerticalVelocity < 0)
                    {
                        VerticalVelocity *= MoveStats.GlideFallSpeedMultiplier;
                    }
                }
            }
        }

        private void StopGlide()
        {
            _glideCooldownTimer = MoveStats.GlideCooldown;
            _timeSinceLastGlideDeduction = 0f;
            _isGliding = false;
            blockMovement = false;
        }

        #endregion

        #region Zipline
        // Zipline logic is complex and will be ported later
        #endregion

        #region Knockback

        static float elapsedTime = 0f;
        static float stunTime = 0f;
        static float currX = 340;
        static float currY = 500f;
        static bool HurtTrigger = false;
        private void HandleKnockback()
        {
            if (IsBeingKnockedBack)
            {
                currState = PlayerState.Hurting;
                // trigger once
                if (HurtTrigger == false)
                {
                    EnterState(currState);
                    HurtTrigger = true;
                }

                if (elapsedTime < MoveStats.knockbackTime)
                {
                    elapsedTime += fixeddt;
                    currX = Utils.MathfMoveTowards(currX, MoveStats.KnockbackForce.x, 2500f * fixeddt);
                    currY = Utils.MathfMoveTowards(currY, MoveStats.KnockbackForce.y, 3500f * fixeddt);
                    float xDir = _isFacingRight ? -currX : currX;
                    Physics.SetVelocity(OwnerName, xDir, currY);
                    return;
                }

                if (stunTime < MoveStats.hurtStunDuration)
                {
                    stunTime += fixeddt;
                }
                else
                {
                    if (!_isGrounded)
                    {
                        _isFastFalling = true;
                    }
                    else
                    {
                        IsBeingKnockedBack = false;
                        elapsedTime = stunTime = 0f;
                        currX = 340f;
                        currY = 500f;
                        _isFastFalling = false;
                        HurtTrigger = false;
                    }
                }
            }
        }

        private void HandleHazardCollision()
        {
            if (Collision.IsCollidingWithLayer(OwnerName, (int)(CollisionLayer.Enemy)))
            {
                if (_hurtTimer < 0)
                {
                    // take dmg
                    IsBeingKnockedBack = true;
                    _hurtTimer = MoveStats.HurtImmunityLength;
                    isVisible = true;
                    intervalTimer = 0f;
                }
            }
        }

        #endregion

        #region PickUp Effects

        float speedTimer = 0.0f;
        void HandleSpeedBoost()
        {
            if (speedTimer > 0f)
            {
                speedTimer -= fixeddt;
                Animation.PlayAnimation(pu_vfx, "Speed", true);
            }
            else
            {
                speedBoostActive = false;
                Animation.PlayAnimation(pu_vfx, "Clear");
            }
        }

        #endregion

        public override void OnDestroy()
        {
            Debug.Log("PlayerMovement Script Destroyed for object: " + OwnerName);
        }

        private static vec2 Lerp(vec2 a, vec2 b, float t)
        {
            t = Utils.MathfClamp(t, 0, 1);
            return new vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }

        // when the player takes damage and become invuln for a few seconds
        private float intervalTimer = 0f;
        private bool isVisible = false;
        private bool _blinkRestoredAlpha = true;    // tracks whether alpha has been restored after immunity ended
        private void HandleBlinkEffect()
        {
            float defaultAlpha = Graphics.GetColor(OwnerName).z;    //1.0f

            if (_hurtTimer >= 0 && _hurtTimer < MoveStats.HurtImmunityLength - MoveStats.hurtStunDuration)
            {
                intervalTimer += fixeddt;

                if (intervalTimer >= MoveStats.hurtBlinkInterval)
                {
                    isVisible = !isVisible;
                    Graphics.SetAlpha(OwnerName, isVisible ? defaultAlpha : defaultAlpha * 0.4f);
                    intervalTimer = 0f;
                }
            }
            else
            {
                // Immunity ended, restore alpha
                isVisible = false;
                intervalTimer = 0f;
                Graphics.SetAlpha(OwnerName, defaultAlpha);
            }
        }

        float footstepTimer = 0.0f;
        // depending on running tiers, play randomised footsteps at different frequencies
        private void PlayFootsteps()
        {
            if (_isGrounded && movespeed > minFootstepDistance && !_isRolling && !_isSkidding && !_isGroundPounding && !_islanding)
            {
                footstepTimer += fixeddt;
                int index = GetCurrentTentacleLevel();
                if (footstepTimer >= footstepFrequencies[index])
                {
                    //playfootstep
                    Random rnd = new Random();
                    int variation = rnd.Next(1, 10);             // 9 diff running sounds
                    Audio.PlaySound("footstep_sfx" + variation.ToString(), "sfx");
                    footstepTimer = 0.0f;
                }
            }
            else
            {
                footstepTimer = 0.0f; // reset when stopping or airborne
            }
        }

        private void EnterState(PlayerState state)
        {
            Debug.Log(state.ToString());

            Audio.StopSound("sfx_loop");
            if (_isGrounded || nonTrackingVFX.Contains(state))
                staticVfxPos = Transform.GetPosition(OwnerName);

            bool enableDebug = false;           // change this to true if u wanna test states

            // handles playing of sound effects
            switch (state)
            {
                case PlayerState.Idle:
                    if (enableDebug) Debug.Log("CurrState: " + "Idle");
                    break;
                case PlayerState.Jumping:
                    if (enableDebug) Debug.Log("CurrState: " + "Jumping");
                    Audio.PlaySound("jump_sfx", "sfx");
                    break;
                case PlayerState.Running:
                    if (enableDebug) Debug.Log("CurrState: " + "Running");
                    break;
                case PlayerState.Skidding:
                    if (enableDebug) Debug.Log("CurrState: " + "Skidding");
                    Audio.PlaySound("turnskid_sfx", "sfx");
                    break;
                case PlayerState.Falling:
                    // need find out how do looping sound till out of state
                    break;
                case PlayerState.Land:
                    if (enableDebug) Debug.Log("CurrState: " + "Landed");
                    Random rnd = new Random();
                    int variation = rnd.Next(1, 5);                     // 4 diff landing sounds
                    Audio.PlaySound("land_sfx" + variation.ToString(), "sfx");
                    break;
                case PlayerState.GroundPounding:
                    if (enableDebug) Debug.Log("CurrState: " + "Groundpound");
                    Audio.PlaySound("groundpound_sfx", "sfx");
                    break;
                case PlayerState.GroundPounding_Falling:
                    if (enableDebug) Debug.Log("CurrState: " + "Groundpound_falling");
                    Audio.PlaySound("groundpound_falling_sfx", "sfx_loop");
                break;
                case PlayerState.Rolling:
                    if (enableDebug) Debug.Log("CurrState: " + "Roll");
                    Audio.PlaySound("roll_sfx", "sfx");
                    break;
                case PlayerState.WallJumping:
                    if (enableDebug) Debug.Log("CurrState: " + "Wallkick");
                    Audio.PlaySound("jump_sfx", "sfx");
                    break;
                case PlayerState.WallKicking:
                    if (enableDebug) Debug.Log("CurrState: " + "Wallkick");
                    Audio.PlaySound("wallkick_fast_sfx", "sfx");
                    break;
                case PlayerState.Ziplining:
                    if (enableDebug) Debug.Log("CurrState: " + "Zipline");
                    // need find out how do looping sound till out of state
                    break;
                case PlayerState.Gliding:
                    if (enableDebug) Debug.Log("CurrState: " + "Glide");
                    Audio.PlaySound("airglide_sfx", "sfx");
                    break;
                case PlayerState.Bonking:
                    if (enableDebug) Debug.Log("CurrState: " + "Bonking");
                    Audio.PlaySound("bonk_sfx", "sfx");
                    break;
                case PlayerState.WallSliding:
                    if (enableDebug) Debug.Log("CurrState: " + "Wallslide");
                    Audio.PlaySound("wallslide_impact_sfx", "sfx");
                    // need find out how do looping sound till out of state
                    break;
                case PlayerState.Hurting:
                    if (enableDebug) Debug.Log("CurrState: " + "Hurting");
                    Audio.PlaySound("hurt_sfx", "sfx");
                    Camera.CameraFlash(0.75f, 1, 0, 0, 0.4f, true, 1.0f);
                    break;
            }
        }

        #region Collison Checks

        private void CollisionChecks()
        {
            IsGrounded();
            BumpedHead();
            IsTouchingWall();

            if (_isTouchingWall && !_isGrounded)
            {
                // while on wall keep timer running
                _wallJumpTimer = MoveStats.WallCoyoteTime;

                // determine which way is away from the wall
                _wallJumpDirection = _isFacingRight ? -1f : 1f;
            }
            else
            {
                // if not on wall - start counting down for coyote time
                _wallJumpTimer -= fixeddt; // Assumes GetDeltaTime is accessible
            }

            canWallKick = _isTouchingWall;
        }

        private void IsGrounded()
        {
            bool triggeredLand = _isGrounded;                       // flag to play sound once
            vec2 position = Transform.GetPosition(OwnerName);
            vec2 scale = Transform.GetScale(OwnerName);
            float halfWidth = Math.Abs(scale.x) / 2;
            float bottomY = position.y;                         // need to start raycast from middle of player and add extra length for raycast bc of bug where player gets stuck in ground
            float raycastLength = MoveStats.GroundDetectionRayLength + (scale.y / 2.1f);
            vec2 rayDirection = new vec2(0, -1);

            // Center raycast
            vec2 leftRayOrigin = new vec2(position.x - halfWidth * 0.8f, bottomY);
            vec2 centerRayOrigin = new vec2(position.x, bottomY);
            vec2 rightRayOrigin = new vec2(position.x + halfWidth * 0.8f, bottomY);

            // debug for ground check raycast
            //Transform.SetScale("DebugLine", 5, raycastLength);
            //Transform.SetPosition("DebugLine", position.x, bottomY - raycastLength/2);

            if (RaycastForGround(centerRayOrigin, rayDirection, raycastLength)
                || RaycastForGround(leftRayOrigin, rayDirection, raycastLength)
                || RaycastForGround(rightRayOrigin, rayDirection, raycastLength))
            {
                _isGrounded = true;
                _coyoteTimer = MoveStats.JumpCoyoteTime;
                if (!triggeredLand)
                {
                    //Audio.PlaySound("land_sfx", "sfx");
                    //currState = PlayerState.Land;
                    //EnterState(currState);
                }
                return;
            }
            else
            {
                _isGrounded = false;
            }
        }

        private bool RaycastForGround(vec2 origin, vec2 direction, float length)
        {
            if (Collision.Raycast(OwnerName, origin, direction, length, out string hitObject, out float hitDistance))
            {
                if (string.IsNullOrEmpty(hitObject)) return false;
                if (Collision.GetCollisionLayer(hitObject) == (int)CollisionLayer.Ground || Collision.GetCollisionLayer(hitObject) == (int)CollisionLayer.Enemy)
                {
                    return true;
                }
            }
            return false;
        }

        private void BumpedHead()
        {
            // This needs to be reimplemented with multiple raycasts
            _bumpedHead = false;
        }

        private void IsTouchingWall()
        {
            _isTouchingWall = false;
            vec2 position = Transform.GetPosition(OwnerName);
            vec2 scale = Transform.GetScale(OwnerName);
            float halfHeight = scale.y / 2;
            float raycastLength = MoveStats.WallDetectionBoxLength;
            vec2 rayDirection = new vec2(_isFacingRight ? 1 : -1, 0);


            // Center raycast
            vec2 centerRayOrigin = new vec2(position.x, position.y);
            if (RaycastForWall(centerRayOrigin, rayDirection, raycastLength))
            {
                //Debug.Log("[Raycast] TOUCHING");
                _isTouchingWall = true;
                _wallDirection = _isFacingRight ? 1 : -1;
                return;
            }

            // Top raycast
            vec2 topRayOrigin = new vec2(position.x, position.y + halfHeight * 0.9f);
            if (RaycastForWall(topRayOrigin, rayDirection, raycastLength))
            {
                //Debug.Log("[Raycast] TOUCHING");
                _isTouchingWall = true;
                _wallDirection = _isFacingRight ? 1 : -1;
                return;
            }

            // Bottom raycast
            vec2 bottomRayOrigin = new vec2(position.x, position.y - halfHeight * 0.75f);
            if (RaycastForWall(bottomRayOrigin, rayDirection, raycastLength))
            {
                _isTouchingWall = true;
                _wallDirection = _isFacingRight ? 1 : -1;
                Debug.Log("[Raycast] TOUCHING");
                return;
            }
        }

        private bool RaycastForWall(vec2 origin, vec2 direction, float length)
        {
            if (Collision.Raycast(OwnerName, origin, direction, length, out string hitObject, out float hitDistance))
            {
                if (string.IsNullOrEmpty(hitObject)) return false;
                // Assuming walls are also on the Ground layer as per the C++ enum comment
                if (Collision.GetCollisionLayer(hitObject) == (int)CollisionLayer.Ground)
                {
                    return true;
                }
            }
            return false;
        }

        #endregion

        public void ResetVars()
        {
            fRun = fJump = fIdle = fZipline = fGroundPound = fGlide = fRoll = fWallKick = false;

            // movement variables
            _moveVelocity = new vec2(0, 0);
            _isFacingRight = true;
            _isRunning = false;
            _isRunToggleActive = false;
            movespeed = 0;

            // public facing right for other scripts
            // Not use yet: _queuedDirection = 0;
            _facingDirection = 1;

            // collision check variables
            _isGrounded = false;
            _bumpedHead = false;
            _islanding = false;
            _groundedTimer = 0;

            // jump variables
            VerticalVelocity = 0;
            _isJumping = false;
            _isFastFalling = false;
            _isFalling = false;
            _fastFallTime = 0;
            _fastFallReleaseSpeed = 0;
            _numberOfJumpsUsed = 0;

            // wall jump variables
            _isWallJumping = false;
            _wallJumpTimer = 0;
            _wallJumpDirection = 0;

            // apex variables
            _apexPoint = 0;
            _timePastApexThreshold = 0;
            _isPastApexThreshold = false;

            // jump buffer variables
            _jumpBufferTimer = 0;
            _jumpReleasedDuringBuffer = false;

            // coyote time variables
            _coyoteTimer = 0;

            // wall kick variables
            _isWallKicking = false;
            //wallKickFrameCounter = 0;
            canWallKick = false;
            blockMovement = false;
            _wallKickTimer = 0;
            _wallKickCooldownTimer = 0;
            // Not use yet: _startedWallKickInAir = false;
            _wallKickTotalDuration = 0;

            // wall slide variables
            _isTouchingWall = false;
            _isWallSliding = false;

            // ground pound variables
            _isGroundPounding = false;
            _groundPoundPauseTimer = 0;

            // roll variables
            _isRolling = false;
            _rollTimer = 0;

            // glide variables
            _isGliding = false;
            _glideCooldownTimer = 0;
            _timeSinceLastGlideDeduction = 0;

            // zipline variables
            _isZiplining = false;
            // Not use yet: _isBeingLaunched = false;
            _ziplineCoyoteTimer = 0;
            // Not use yet: _ziplineHeadOffset = 0;

            // movement penalties - bonk & skid
            _bonkTimer = 0;
            _isSkidding = false;
            // Not use yet: _skidTargetDirection = 0;
            _skidTimer = 0;
            _calculatedSkidDecel = 0;
        }

    }
}
