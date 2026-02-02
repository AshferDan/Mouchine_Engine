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
            Falling,
            Land,
            Dashing,
            GroundPounding,
            Rolling,
            WallKicking,
            Ziplining,
            Gliding,
            WallSliding,
            WallJumping
        };

        private PlayerState currState = PlayerState.Idle;
        private float dt => Utils.GetDelta_Time();
        private float fixeddt => Utils.GetFixed_Delta_Time();

        private string audioFilepath = "..Assets/Audio/";

        public PlayerMovementStats MoveStats;

        #region Variables
        // This property should be set by the C++ engine after the script is attached.
        // changed to using OwnerName
        //public string OwnerName = "Hero";

        // movement variables
        private vec2 _moveVelocity = new vec2(0, 0);
        private bool _isFacingRight = true;
        private bool _isRunning;
        private bool _isRunToggleActive = false;     // internal state to track if player is running (used for Toggle Mode)
        private float movespeed;
        public float CurrentSpeed => movespeed;

        // public facing right for other scripts
        public bool IsFacingRight => _isFacingRight;        // public facing right for other scripts
        private int _queuedDirection = 0;                   // remembers input while airborne
        private int _facingDirection = 1;                   // visual direction

        // collision check variables
        private bool _isGrounded = false;
        private bool _bumpedHead = false;
        private bool _islanding = false;

        // jump variables
        public float VerticalVelocity { get; private set; } = 0;
        private bool _isJumping = false;
        private bool _isFastFalling = false;
        private bool _isFalling = false;
        private float _fastFallTime = 0;
        private float _fastFallReleaseSpeed = 0;
        private int _numberOfJumpsUsed = 0;

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

        // dash variables
        private bool _isDashing = false;
        private float _dashTimer = 0;
        private float _dashCooldownTimer = 0;

        // wall kick variables
        private bool _isWallKicking = false;
        private int wallKickFrameCounter = 0;           // [WIP] for cayote time 
        private bool canWallKick = false;                   // Checks if touching a wall, and can perform a wall kick
        private bool blockMovement = false;         // Prevents player from controlling character
        private float _wallKickTimer = 0;
        private float _wallKickCooldownTimer = 0;
        private bool _startedWallKickInAir = false;
        private float _wallKickTotalDuration = 0;

        // wall slide variables
        private bool _isTouchingWall = false;
        private bool _isWallSliding = false;

        // ground pound variables
        private bool _isGroundPounding = false;     // checks if player is currently in groundpound state
        vec2 lastPos;                            // stores pos of when the player initiated groundpound (for conversion to horizontal dist)
        private float _groundPoundPauseTimer = 0;

        // roll variables
        private bool _isRolling = false;            // checks if player is in rolling state
        private float _rollTimer = 0;

        // glide variables
        private bool _isGliding = false;            // checks if player is in gliding state
        private float _glideCooldownTimer = 0;
        private float _timeSinceLastGlideDeduction = 0;  // timer for periodic deduction

        // zipline variables
        private bool _isZiplining = false;
        private bool _isBeingLaunched = false;
        private float _ziplineCoyoteTimer = 0;
        private float _ziplineHeadOffset = 0;

        // knockback
        // private KnockBack knockback; //

        // movement penalties - bonk & skid
        private float _bonkTimer = 0f;
        private bool _isSkidding;
        private int _skidTargetDirection;
        private float _skidTimer = 0f;
        private float _calculatedSkidDecel;
        #endregion


        public PlayerMovement()
        {
            // When the script is created, instantiate its stats
            MoveStats = new PlayerMovementStats();
            Debug.Log("PlayerMovement instantiated with new stats.");
        }

        public void OnStart()
        {
            // It's crucial that OwnerName is set by the C++ engine before OnStart is called.
            if (OwnerName == null)
            {
                Debug.Log("ERROR: OwnerName is not set in PlayerMovement script!");
                return;
            }

            // Add sounds
            Audio.AddSound("jump_sfx", "../Assets/Audio/Player/Jump.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("footstep_sfx", "../Assets/audio/Player/Footstep.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("groundpound_sfx", "../Assets/audio/Player/Groundpound.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("roll_sfx", "../Assets/audio/Player/Roll.wav", false, 0.2f, 1.0f, "sfx");
            Audio.AddSound("dash_sfx", "../Assets/audio/Player/Dash.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("wallkick_sfx", "../Assets/audio/Player/Wallkick.wav", false, 0.8f, 1.0f, "sfx");
            Audio.AddSound("land_sfx", "../Assets/audio/Player/Land.wav", false, 0.3f, 1.0f, "sfx");

            Animation.RegisterAnimation("Hero", "Idle", "../Assets/spritesheet_R.png", 13, 10, 7, 0, 7, 2, 4.0f);
            Animation.RegisterAnimation("Hero", "Run", "../Assets/spritesheet_R.png", 13, 10, 12, 0, 12, 7, 16.0f);
            Animation.RegisterAnimation("Hero", "Jump", "../Assets/spritesheet_R.png", 13, 10, 11, 4, 11, 5, 12.0f);
            Animation.RegisterAnimation("Hero", "Glide", "../Assets/spritesheet_R.png", 13, 10, 1, 0, 1, 2, 8.0f);
            Animation.RegisterAnimation("Hero", "Roll", "../Assets/spritesheet_R.png", 13, 10, 8, 0, 8, 5, 12.0f);
            Animation.RegisterAnimation("Hero", "Groundpound", "../Assets/spritesheet_R.png", 13, 10, 9, 3, 9, 4, 10.0f);
            Animation.RegisterAnimation("Hero", "Dash", "../Assets/spritesheet_R.png", 13, 10, 3, 0, 3, 2, 4.0f);
            Animation.RegisterAnimation("Hero", "Wallkick", "../Assets/spritesheet_R.png", 13, 10, 4, 3, 4, 4, 4.0f);
            Animation.RegisterAnimation("Hero", "Wallslide", "../Assets/spritesheet_L.png", 13, 10, 4, 0, 4, 1, 4.0f);
            Animation.RegisterAnimation("Hero", "Falling", "../Assets/spritesheet_L.png", 13, 10, 10, 0, 10, 1, 4.0f);
            Animation.RegisterAnimation("Hero", "Land", "../Assets/spritesheet_L.png", 13, 10, 10, 3, 10, 4, 4.0f);

            // Attach the player movement script to the Hero
            CSharpScript.Scripting.AttachScript("Hero", "PlayerMovement");

            _isFacingRight = true;

            Physics.SetGravity(OwnerName, true);

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

            vec2 scale = Transform.GetScale(OwnerName);
            vec2 position = Transform.GetPosition(OwnerName);
            //_ziplineHeadOffset = position.y + (scale.y / 2);
            Physics.SetFriction(OwnerName, 0f);
            Physics.SetMass(OwnerName, 0.0001f);
            _moveVelocity = new vec2(0, 0);
        }

        public void OnUpdate(float deltaTime)
        {
            if (OwnerName == null || OwnerName == "") return;

            // additional checks here

            currState = PlayerState.Idle;
            UpdatePlayerState();

            // Handle all timers
            _jumpBufferTimer -= deltaTime;
            _coyoteTimer -= deltaTime;
            _dashCooldownTimer -= deltaTime;
            _wallKickCooldownTimer -= deltaTime;
            _ziplineCoyoteTimer -= deltaTime;
            JumpChecks();
            // Handle Inputs
            HandleGeneralInput();
            CollisionChecks();
            OnFixedUpdate(fixeddt);

            switch(currState)
            {
                case PlayerState.Idle:
                    Animation.PlayAnimation(OwnerName, "Idle");
                    break;
                case PlayerState.Jumping:
                    Animation.PlayAnimation(OwnerName, "Jump");
                    break;
                case PlayerState.Running:
                    Animation.PlayAnimation(OwnerName, "Run");
                    break;
                case PlayerState.Falling:
                    Animation.PlayAnimation(OwnerName, "Falling");
                    break;
                case PlayerState.Land:
                    // commented out because landing is buggy
                    //Animation.PlayAnimation(OwnerName, "Land");
                    break;
                case PlayerState.Dashing:
                    Animation.PlayAnimation(OwnerName, "Dash");
                    break;
                case PlayerState.GroundPounding:
                    Animation.PlayAnimation(OwnerName, "Groundpound");
                    break;
                case PlayerState.Rolling:
                    Animation.PlayAnimation(OwnerName, "Roll");
                    break;
                case PlayerState.WallKicking:
                    Animation.PlayAnimation(OwnerName, "Wallkick");
                    break;
                case PlayerState.Ziplining:
                    Animation.PlayAnimation(OwnerName, "Zipline");
                    break;
                case PlayerState.Gliding:
                    Animation.PlayAnimation(OwnerName, "Glide");
                    break;
                case PlayerState.WallSliding:
                    Animation.PlayAnimation(OwnerName, "Wallslide");
                    break;
                case PlayerState.WallJumping:
                    Animation.PlayAnimation(OwnerName, "Wallkick");
                    break;
            }
        }

        public void OnFixedUpdate(float fixedDeltaTime)
        {
            if (OwnerName == null) return;
            CollisionChecks();

            if (!_isZiplining) //&& !knockback.IsBeingKnockedBack && !_isBeingLaunched)
            {
                Jump(fixedDeltaTime);
                WallKick(fixedDeltaTime);
                GroundPound(fixedDeltaTime);
                Roll(fixedDeltaTime);
                Glide(fixedDeltaTime);
                WallSlide(fixedDeltaTime);

                // only allow regular movement if not dashing
                //if (!_isDashing && !_isWallKicking && !_isWallJumping && !_isGroundPounding)
                //{
                //    if (_isGrounded)
                //    {
                //        Move(MoveStats.GroundAcceleration, MoveStats.GroundDeceleration, InputManager.Movement, fixedDeltaTime);
                //    }
                //    else
                //    {
                //        Move(MoveStats.AirAcceleration, MoveStats.AirDeceleration, InputManager.Movement, fixedDeltaTime);
                //    }
                //}
                //else Debug.Log("aaaa");
            }
        }

        #region Movement
        public void Move(float acceleration, float deceleration, vec2 moveInput, float deltaTime)
        {
            // OLD
            //if (blockMovement) moveInput = new vec2(0, 0);

            //if (moveInput.x != 0 || (_isRolling && _rollTimer >= 0))
            //{
            //    TurnCheck(moveInput);
            //    vec2 targetVelocity = new vec2(moveInput.x * MoveStats.MaxWalkSpeed, 0f);

            //    if (!(_isRolling && _rollTimer >= 0))
            //    {
            //        _moveVelocity = Lerp(_moveVelocity, targetVelocity, acceleration * deltaTime);
            //    }

            //    Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);
            //}
            //else
            //{
            //    _moveVelocity = Lerp(_moveVelocity, new vec2(0, 0), deceleration * deltaTime);

            //    Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);
            //}

            float inputX = moveInput.x;
            bool isRunHeld = _isRunToggleActive; // for run toggle option

            int targetDirection = 0;
            if (inputX != 0) targetDirection = (int)inputX;
            else if (isRunHeld && _isGrounded) targetDirection = _facingDirection;

            // landing skid check - for jump flip
            // if hit groud + fast speed BUT facing opposite direction of movement = landing skid
            //bool movingAgainstFacing = Mathf.Sign(_rb.linearVelocity.x) != _facingDirection;
            bool movingAgainstFacing = Math.Sign(Physics.GetVelocity(OwnerName).x) != _facingDirection;
            //bool isMovingFast = Mathf.Abs(_rb.linearVelocity.x) > 5f;            // small threshold (1f) to ignore tiny jitters when stopping
            bool isMovingFast = Math.Abs(Physics.GetVelocity(OwnerName).x) > 5f;

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

            // movement logic
            if (targetDirection != 0)
            {
                bool isOpposite = (targetDirection > 0 && _facingDirection == -1) || (targetDirection < 0 && _facingDirection == 1);

                // turn check
                if (isOpposite)
                {
                    int tier = GetCurrentTentacleLevel();

                    // air turn (visual flip only) - if in air + run held / toggled + moving fast
                    if (!_isGrounded && isRunHeld && movespeed > MoveStats.T0_MaxSpeed)
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

                        if (tier == 0)
                        {
                            // nstant turn
                            movespeed = MoveStats.TurnResetSpeed;
                            _facingDirection = targetDirection;
                            Turn(targetDirection > 0);
                            _isSkidding = false;
                        }
                        else
                        {
                            _isSkidding = true;
                            _facingDirection = targetDirection;
                            Turn(targetDirection > 0);

                            float duration = MoveStats.T1_SkidDuration;
                            if (tier == 2) duration = MoveStats.T2_SkidDuration;
                            if (tier == 3) duration = MoveStats.T3_SkidDuration;

                            _skidTimer = duration;
                            movespeed *= MoveStats.SkidSpeedRetained;


                            // skid will stop at "exitspeed" so player doesnt drop down to 0
                            float exitSpeed = 4f;

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
            movespeed = Utils.MathfClamp(movespeed, 0, 50f);

            // determine physics direction
            int moveDir = _facingDirection;

            // if in air + moving fast + turned in the opposite direction = flip the physics application
            // use rb's current velocity to detect the "real movement" direction
            if (!_isGrounded && isRunHeld && movespeed > MoveStats.T0_MaxSpeed)
            {
                // if facing right (1) but moving left (vel < -1) -> apply force left (-1)
                if (_facingDirection == 1 && Physics.GetVelocity(OwnerName).x < -1f) moveDir = -1;

                // if facing left (-1) but moving right (vel > 1) -> apply force right (1)
                else if (_facingDirection == -1 && Physics.GetVelocity(OwnerName).x > 1f) moveDir = 1;
            }

            _moveVelocity.x = movespeed * moveDir;
            //_rb.linearVelocity = new Vector2(_moveVelocity.x, Physics.GetVelocity(OwnerName).y);
            Physics.SetVelocity(OwnerName, _moveVelocity.x, Physics.GetVelocity(OwnerName).y);

            //if (_animator != null) _animator.SetBool("isSkidding", _isSkidding);
        }

        private void TurnCheck(vec2 moveInput)
        {
            if (_isFacingRight && moveInput.x < 0)
            {
                Turn(false);
            }
            else if (!_isFacingRight && moveInput.x > 0)
            {
                Turn(true);
            }
        }

        private void Turn(bool turnRight)
        {
            _isFacingRight = turnRight;
            vec2 currentScale = Transform.GetScale(OwnerName);
            // [WIP] to include left and right animations for accurate monocle, for now will just do a basic flip
            Transform.SetScale(OwnerName, Math.Abs(currentScale.x) * (turnRight ? 1 : -1), currentScale.y);
            
            // Unity ver
            //if (turnRight)
            //{
            //    _isFacingRight = true;
            //    transform.Rotate(0f, 180f, 0f);
            //}
            //else
            //{
            //    _isFacingRight = false;
            //    transform.Rotate(0f, -180f, 0f);
            //}
        }

        #endregion

        #region State & Input
        private void UpdatePlayerState()
        {
            // check if in air
            if (!_isGrounded)
            {
                if (_isWallJumping)
                {
                    currState = PlayerState.WallJumping;
                }
                else
                {
                    currState = PlayerState.Jumping;
                }
            }
            else
            {
                // horizontal input = moving - trigger run animation
                if (InputManager.Movement.x != 0)
                {
                    currState = PlayerState.Running;
                }
                else
                {
                    currState = PlayerState.Idle;
                }
            }

            // placed after jump and run, so it can be overriden 
            if (_isRunning) currState = PlayerState.Running;
            if (_isRolling) currState = PlayerState.Rolling;
            if (_isGroundPounding) currState = PlayerState.GroundPounding;
            if (_isGliding) currState = PlayerState.Gliding;
            if (_isWallKicking) currState = PlayerState.WallKicking;
            if (_isZiplining) currState = PlayerState.Ziplining;
            if (_isWallSliding) currState = PlayerState.WallSliding;
            // landing is bugged
            //if (_islanding) { currState = PlayerState.Land; }
        }

        private void HandleGeneralInput()
        {
            // check for dash input
            if (!_isDashing && !_isWallKicking && !_isWallJumping && !_isGroundPounding)
            {
                if (_isGrounded)
                {
                    Move(MoveStats.GroundAcceleration, MoveStats.GroundDeceleration, InputManager.Movement, Utils.GetDelta_Time());
                    //Debug.Log("grounded");
                }
                else
                {
                    Move(MoveStats.AirAcceleration, MoveStats.AirDeceleration, InputManager.Movement, Utils.GetDelta_Time());
                    //Debug.Log("air");
                }
            }

            //if (InputManager.RunWasPressed && _dashCooldownTimer <= 0)
            //{
            //    InitiateDash();
            //}
            if (InputManager.WallKickWasPressed && _wallKickCooldownTimer <= 0 && canWallKick)
            {
                InitiateWallKick();
            }
            if (InputManager.GroundPoundWasPressed && !_isGrounded)
            {
                InitiateGroundPound();
            }
            if (InputManager.RollWasPressed)
            {
                InitiateRoll();
            }
            if (InputManager.GlideIsHeld && !_isGrounded)
            {
                InitiateGlide();
            }
            else if (_isGliding) // if key was let go but still in gliding state, cancel gliding action
            {
                StopGlide();
            }
        }

        #endregion

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
            //_rb.linearVelocity = new Vector2(_moveVelocity.x, _rb.linearVelocity.y);
            Physics.SetVelocity(OwnerName, _moveVelocity.x, Physics.GetVelocity(OwnerName).y);
        }

        private void HandleWallBonk()
        {
            //if (_animator != null) _animator.SetTrigger("Bump");

            // force roll + ground pound states to end immediately
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

            //_rb.linearVelocity = new Vector2(-_facingDirection * force.x, force.y);
            Physics.SetVelocity(OwnerName, -_facingDirection * force.X, force.Y);
            _bonkTimer = MoveStats.T3_BonkDuration;
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
                _wallJumpTimer -= Utils.GetDelta_Time(); // Assumes GetDeltaTime is accessible
            }

            canWallKick = _isTouchingWall;
            if (_isTouchingWall && !_isGrounded) currState = PlayerState.WallSliding;
        }

        private void IsGrounded()
        {
            bool oldIsGrounded = _isGrounded;
            _isGrounded = false;
            vec2 position = Transform.GetPosition(OwnerName);
            vec2 scale = Transform.GetScale(OwnerName);
            float halfWidth = Math.Abs(scale.x) / 2;
            float bottomY = position.y - (scale.y / 2.1f);
            float raycastLength = MoveStats.GroundDetectionRayLength;
            vec2 rayDirection = new vec2(0, -1);

            // Center raycast
            vec2 centerRayOrigin = new vec2(position.x, bottomY);

            Transform.SetScale("Debugline", 5, raycastLength);
            Transform.SetPosition("Debugline", position.x, bottomY);

            if (RaycastForGround(centerRayOrigin, rayDirection, raycastLength))
            {
                _isGrounded = true;
                _coyoteTimer = MoveStats.JumpCoyoteTime;
                if (!oldIsGrounded)
                {
                    Audio.PlaySound("land_sfx", "sfx");
                    currState = PlayerState.Land;
                }
                return;
            }

            // Left raycast
            vec2 leftRayOrigin = new vec2(position.x - halfWidth * 0.9f, bottomY);
            if (RaycastForGround(leftRayOrigin, rayDirection, raycastLength))
            {
                _isGrounded = true;
                _coyoteTimer = MoveStats.JumpCoyoteTime;
                if (!oldIsGrounded)
                {
                    //Audio.PlaySound("land_sfx", "sfx");
                    currState = PlayerState.Land;
                }
                return;
            }

            // Right raycast
            vec2 rightRayOrigin = new vec2(position.x + halfWidth * 0.9f, bottomY);
            if (RaycastForGround(rightRayOrigin, rayDirection, raycastLength))
            {
                _isGrounded = true;
                _coyoteTimer = MoveStats.JumpCoyoteTime;
                if (!oldIsGrounded)
                {
                    //Audio.PlaySound("land_sfx", "sfx");
                    currState = PlayerState.Land;
                }
                return;
            }
        }

        private bool RaycastForGround(vec2 origin, vec2 direction, float length)
        {
            if (Collision.Raycast(OwnerName, origin, direction, length, out string hitObject, out float hitDistance))
            {
                if (Collision.GetCollisionLayer(hitObject) == (int)CollisionLayer.Ground)
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
                _isTouchingWall = true;
                return;
            }

            // Top raycast
            vec2 topRayOrigin = new vec2(position.x, position.y + halfHeight * 0.9f);
            if (RaycastForWall(topRayOrigin, rayDirection, raycastLength))
            {
                _isTouchingWall = true; Debug.Log("TOUCHING");
                return;
            }

            // Bottom raycast
            vec2 bottomRayOrigin = new vec2(position.x, position.y - halfHeight * 0.9f);
            if (RaycastForWall(bottomRayOrigin, rayDirection, raycastLength))
            {
                _isTouchingWall = true; Debug.Log("TOUCHING");
                return;
            }
        }

        private bool RaycastForWall(vec2 origin, vec2 direction, float length)
        {
            if (Collision.Raycast(OwnerName, origin, direction, length, out string hitObject, out float hitDistance))
            {
                // Assuming walls are also on the Ground layer as per the C++ enum comment
                if (Collision.GetCollisionLayer(hitObject) == (int)CollisionLayer.Ground)
                {
                    return true;
                }
            }
            return false;
        }

        #endregion

        #region Jump

        private void Jump(float deltaTime)
        {
            if (_isWallJumping && VerticalVelocity <= 0)
            {
                _isWallJumping = false;
                blockMovement = false; // give back control
            }

            // apply gravity while jumping
            if (_isJumping)
            {
                currState = PlayerState.Jumping;
                if (_isTouchingWall) currState = PlayerState.WallSliding;
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

                        _timePastApexThreshold += deltaTime;

                        if (_timePastApexThreshold < MoveStats.ApexHangTime)
                        {
                            VerticalVelocity = 0f;
                        }
                        else
                        {
                            VerticalVelocity = -0.01f;
                        }
                    }
                    else
                    {
                        VerticalVelocity += MoveStats.Gravity * deltaTime;
                        _isPastApexThreshold = false;
                    }
                }
                else if (!_isFastFalling)
                {
                    // gravity on descending
                    VerticalVelocity += MoveStats.Gravity * deltaTime;
                }
                else if (VerticalVelocity < 0f && !_isFalling)
                {
                    _isFalling = true;
                }
            }

            // jump cut
            if (_isFastFalling)
            {
                currState = PlayerState.Falling;
                if (_fastFallTime < MoveStats.TimeForUpwardsCancel)
                {
                    VerticalVelocity = Utils.MathfLerp(_fastFallReleaseSpeed, 0f, _fastFallTime / MoveStats.TimeForUpwardsCancel);
                }
                else
                {
                    VerticalVelocity += MoveStats.Gravity * MoveStats.GravityOnReleaseMultiplier * deltaTime;
                }

                _fastFallTime += deltaTime;
            }

            if (!_isGrounded && !_isJumping && !_isFastFalling)
            {
                if (!_isFalling)
                    _isFalling = true;

                VerticalVelocity += MoveStats.Gravity * deltaTime;
            }

            // clamp fall speed
            VerticalVelocity = Utils.MathfClamp(VerticalVelocity, -MoveStats.MaxFallSpeed, MoveStats.InitialJumpVelocity);

            Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);
        }

        private void JumpChecks()
        {
            // when jump is pressed
            if (InputManager.JumpWasPressed)
            {
                _jumpBufferTimer = MoveStats.JumpBufferTime;
                _jumpReleasedDuringBuffer = false;
            }

            // when jump is released - with hold to jump enabled
            if (MoveStats.EnableVariableJumpHeight)
            {
                if (InputManager.JumpWasReleased)
                {
                    if (_jumpBufferTimer > 0f)
                    {
                        _jumpReleasedDuringBuffer = true;
                    }

                    if (_isJumping && VerticalVelocity > 0f)
                    {
                        if (_isPastApexThreshold)
                        {
                            _isPastApexThreshold = false;
                            _isFastFalling = true;
                            _fastFallTime = MoveStats.TimeForUpwardsCancel;
                            VerticalVelocity = 0f;
                        }
                        else
                        {
                            _isFastFalling = true;
                            _fastFallReleaseSpeed = VerticalVelocity;
                        }
                    }
                }
            }

            // wall jump
            if (_jumpBufferTimer > 0f && _wallJumpTimer > 0f)
            {
                InitiateWallJump();
                _jumpBufferTimer = 0f; // consume buffer to prevent other jumps
            }
            // initiate jump with jump buffering and coyote time
            else if (_jumpBufferTimer > 0f && !_isJumping && (_isGrounded || _coyoteTimer > 0f))
            {
                InitiateJump(1);

                if (_jumpReleasedDuringBuffer)
                {
                    _isFastFalling = true;
                    _fastFallReleaseSpeed = VerticalVelocity;
                }
            }
            // double jump
            else if (_jumpBufferTimer > 0f && _isJumping && _numberOfJumpsUsed < MoveStats.NumberOfJumpsAllowed)
            {
                _isFastFalling = false;
                InitiateJump(1);
            }
            // handle air jump after the coyote time has lapsed (take off an extra jump so the player does not get a bonus jump)
            else if (_jumpBufferTimer > 0f && _isFalling && _numberOfJumpsUsed < MoveStats.NumberOfJumpsAllowed - 1)
            {
                InitiateJump(2);
                _isFastFalling = false;
            }

            // landing
            if ((_isJumping || _isFalling) && _isGrounded && VerticalVelocity <= 0f)
            {
                //_islanding = true;
                if (!_islanding) _islanding = true;


                if (_isWallKicking)
                {
                    StopWallKick();
                }

                // consume buffered jump upon landing
                _jumpBufferTimer = 0f;

                _isJumping = false;
                _isFalling = false;
                _isFastFalling = false;
                _fastFallTime = 0f;
                _isPastApexThreshold = false;
                _numberOfJumpsUsed = 0;

                _isWallJumping = false;
                blockMovement = false;

                _ziplineCoyoteTimer = 0f;

                VerticalVelocity = -0.5f; // small negative value to keep grounded
            }
        }

        #region Slide
        private void WallSlide()
        {
            if (_isGrounded)// || (knockback != null && knockback.IsBeingKnockedBack))
            {
                if (_isWallSliding)
                {
                    _isWallSliding = false;
                    //AudioManager.instance.StopLoopingPlayerSFX();
                    //if (_wallSoundCoroutine != null) StopCoroutine(_wallSoundCoroutine);
                }
                return;
            }

            // check collision & falling speed
            //if (_isTouchingWall && _rb.linearVelocity.y < 0)
            if (_isTouchingWall && Physics.GetVelocity(OwnerName).y < 0)
            {
                // first frame logic
                if (!_isWallSliding)
                {
                    _isWallSliding = true;

                    // reset jumps if want wall sliding to refresh double jump
                    _numberOfJumpsUsed = 0;

                    //if (_wallSoundCoroutine != null) StopCoroutine(_wallSoundCoroutine);
                    //_wallSoundCoroutine = StartCoroutine(PlayWallSlideAudioSequence(sounds.wallLand, sounds.wallSlide));
                }

                // apply friction/slide speed
                //_rb.linearVelocity = new Vector2(_rb.linearVelocity.x, Mathf.Max(_rb.linearVelocity.y, -MoveStats.WallSlidingSpeed));
                Physics.SetVelocity(OwnerName, Physics.GetVelocity(OwnerName).x, Math.Max(Physics.GetVelocity(OwnerName).y, -MoveStats.WallSlidingSpeed));
            }
            else
            {
                // exit wall slide logic
                if (_isWallSliding)
                {
                    _isWallSliding = false;
                    //AudioManager.instance.StopLoopingPlayerSFX();
                    //if (_wallSoundCoroutine != null) StopCoroutine(_wallSoundCoroutine);
                }
            }
        }

        #endregion

        private void InitiateJump(int numberOfJumpsUsed)
        {
            if (_isRolling)
            {
                StopRoll();
            }

            if (!_isJumping)
            {
                _isJumping = true;
            }

            Audio.PlaySound("jump_sfx", "sfx");
            _jumpBufferTimer = 0f;
            _numberOfJumpsUsed += numberOfJumpsUsed;
            VerticalVelocity = MoveStats.InitialJumpVelocity;
        }
        #endregion

        #region Wall Slide + Jump + Kick

        private void WallSlide(float deltaTime)
        {
            bool isPushingTowardsWall = (_isFacingRight && InputManager.Movement.x > 0) ||
                                        (!_isFacingRight && InputManager.Movement.x < 0);

            //vec2 currentVelocity = Physics.GetVelocity(OwnerName);

            if (_isTouchingWall && !_isGrounded && isPushingTowardsWall)
            {
                if (_moveVelocity.y < 0)
                {
                    _isWallSliding = true;
                    float newYVelocity = Utils.MathfClamp(_moveVelocity.y, -MoveStats.WallSlidingSpeed, 999f);
                    Physics.SetVelocity(OwnerName, _moveVelocity.x, newYVelocity);
                }
            }
            else
            {
                if (_isWallSliding)
                {
                    _isWallSliding = false;
                }
            }
        }

        private void InitiateWallJump()
        {
            // state change
            _isWallJumping = true;
            blockMovement = true;

            // stop previous state
            _isWallSliding = false;
            _isJumping = true;

            // apply forces for wall jump
            VerticalVelocity = MoveStats.WallJumpForce.y;
            float horizontalDirection = _isFacingRight ? -1f : 1f;
            _moveVelocity.x = MoveStats.WallJumpForce.x * horizontalDirection;

            //vec2 currentVelocity = Physics.GetVelocity(OwnerName);
            Physics.SetVelocity(OwnerName, _moveVelocity.x, _moveVelocity.y);

            // turn the player
            Turn(!_isFacingRight);
        }

        private void InitiateWallKick()
        {
            if (_isWallKicking) return;

            _isWallKicking = true;
            blockMovement = true;

            // Play wall kick sound
            //if (sounds != null && sounds.wallKick != null)
            //    AudioManager.Instance.PlayPlayerSFX(sounds.wallKick);

            //fWallKick = true; // Timer manager flag

            _wallKickTotalDuration = MoveStats.DashDuration * MoveStats.WallKickDurationMultiplier;
            _wallKickTimer = _wallKickTotalDuration;
            _wallKickCooldownTimer = MoveStats.DashCooldown;

            // Determine horizontal direction (opposite of facing)
            float wallKickDir = _isFacingRight ? -1f : 1f;

            // Flip player to face new direction
            Turn(!_isFacingRight);

            // Set initial horizontal velocity
            _moveVelocity.x = wallKickDir * MoveStats.WallKickSpeedModifier;
            VerticalVelocity = 0f;

            Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);
            Audio.PlaySound("wallkick_sfx", "sfx");
        }

        private void WallKick(float deltaTime)
        {
            if (!_isWallKicking) return;

            currState = PlayerState.WallKicking;
            blockMovement = true;
            _isJumping = false;

            _wallKickTimer -= deltaTime;

            // Progress: 0.0 at start → 1.0 at end
            float progress = 1f - (_wallKickTimer / _wallKickTotalDuration);

            // Curve multiplier (ease-in/out effect)
            //float curveMultiplier = MoveStats.WallKickEaseCurve.Evaluate(progress);

            // Calculate current horizontal and vertical speeds
            float currentSpeedX = _moveVelocity.x * 1;
            float verticalGain = MoveStats.WallKickVerticalModifier * 1;

            // Apply velocities directly
            Physics.SetVelocity(OwnerName, currentSpeedX, verticalGain);

            // End wall kick when timer finishes
            if (_wallKickTimer <= 0f)
                StopWallKick();
        }

        private void StopWallKick()
        {
            if (!_isWallKicking) return;

            _isWallKicking = false;
            blockMovement = false;

            // Stop horizontal motion after wall kick
            _moveVelocity.x = 0f;

            // Keep current vertical motion (gravity or jump will continue)
            Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);

            wallKickFrameCounter = 0; // Reset wall coyote counter
        }
        #endregion

        #region Roll

        private void InitiateRoll(float speedOverride = -1f, float durationOverride = -1f)
        {
            if (_isRolling || !_isGrounded) return;

            // Play sound if assigned
            //if (sounds != null && sounds.roll != null)
            //    AudioManager.Instance.PlayPlayerSFX(sounds.roll);

            _isRolling = true;
            blockMovement = true;

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

            _isGroundPounding = false;

            // Set roll velocity
            _moveVelocity.x = movespeed * _facingDirection;
            VerticalVelocity = 0f;
            Audio.PlaySound("roll_sfx", "sfx");
        }

        private void Roll(float deltaTime)
        {
            if (!_isRolling) return;

            currState = PlayerState.Rolling;
            // Cancel roll if touching wall in air
            if (_isTouchingWall)
            {
                StopRoll();
                return;
            }

            // End roll on timer or dash
            if (_rollTimer <= 0f)
            {
                blockMovement = false;
                StopRoll();
            }
            else
            {
                _rollTimer -= deltaTime;
                // Keep horizontal roll speed
                Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);
            }
        }

        private void StopRoll()
        {
            if (!_isRolling) return;

            _isRolling = false;
            blockMovement = false;

            // Reset horizontal velocity
            //_moveVelocity.x = 0f;
            //Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);
        }

        #endregion

        #region Ground Pound
        private void InitiateGroundPound()
        {
            if (_isGroundPounding) return;

            // Play sound if assigned
            //if (sounds != null && sounds.groundPound != null)
            //{
            //    AudioManager.Instance.PlayPlayerSFX(sounds.groundPound);
            //}

            _isGroundPounding = true;
            blockMovement = true;                   // Disable horizontal input during pause
            _groundPoundPauseTimer = MoveStats.GroundPoundPauseTime;

            // Stop horizontal movement, start downward velocity
            _moveVelocity.x = 0f;
            VerticalVelocity = -MoveStats.GroundPoundFallSpeed;

            // Store starting position for midair pause
            lastPos = Transform.GetPosition(OwnerName);
            Audio.PlaySound("groundpound_sfx", "sfx");
        }

        private void GroundPound(float deltaTime)
        {
            if (!_isGroundPounding) return;

            currState = PlayerState.GroundPounding;
            if (_groundPoundPauseTimer > 0f)
            {
                // Pause in midair
                _groundPoundPauseTimer -= deltaTime;

                // Keep player in same position
                Transform.SetPosition(OwnerName, lastPos.x, lastPos.y);

                // Stop vertical velocity while paused
                VerticalVelocity = 0f;
            }
            else
            {
                // If player is grounded or dashing, stop ground pound
                if (_isGrounded || _isDashing)
                {
                    blockMovement = false;
                    StopGroundPound();
                }
                else
                {
                    // Move downward by ground pound speed
                    VerticalVelocity = -MoveStats.GroundPoundFallSpeed;
                }
            }

            // Always push velocity to physics
            Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);
        }

        private void StopGroundPound()
        {
            Audio.StopSound("groundpound_sfx");
            if (_isGrounded)
            {
                InitiateRoll();
                return;
            }
            _isGroundPounding = false;
            blockMovement = false;

            // Reset vertical velocity (optional)
            VerticalVelocity = 0f;

            // If grounded, can chain into roll
        }
        #endregion

        #region Air Glide

        private void InitiateGlide()
        {
            if (_isGliding || _glideCooldownTimer > 0f) return;

            // Play glide sound
            //if (sounds != null && sounds.airGlide != null)
            //    AudioManager.Instance.PlayPlayerSFX(sounds.airGlide);

            _isGliding = true;
            blockMovement = true;
        }

        private void Glide(float deltaTime)
        {
            // Update cooldown
            if (_glideCooldownTimer > 0f)
                _glideCooldownTimer -= deltaTime;

            if (!_isGliding) return;
            currState = PlayerState.Gliding;
            // Cancel glide if grounded or dashing
            if (_isGrounded || _isDashing)
            {
                StopGlide();
                return;
            }

            // Periodic glide deductions for timer system
            _timeSinceLastGlideDeduction += deltaTime;
            if (_timeSinceLastGlideDeduction >= MoveStats.GlideDeductionInterval)
            {
                _timeSinceLastGlideDeduction = 0f;
            }

            // Slow vertical fall while gliding
            if (VerticalVelocity < 0f)
                VerticalVelocity *= MoveStats.GlideFallSpeedMultiplier;

            Physics.SetVelocity(OwnerName, _moveVelocity.x, VerticalVelocity);
        }

        private void StopGlide()
        {
            _isGliding = false;
            blockMovement = false;
            _glideCooldownTimer = MoveStats.GlideCooldown;
            _timeSinceLastGlideDeduction = 0f;
        }

        #endregion

        #region Zipline
        // Zipline logic is complex and will be ported later
        #endregion

        public void OnDestroy()
        {
            Debug.Log("PlayerMovement Script Destroyed for object: " + OwnerName);
        }

        private static vec2 Lerp(vec2 a, vec2 b, float t)
        {
            t = Utils.MathfClamp(t, 0, 1);
            return new vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }
        private int GetCurrentTentacleLevel()
        {
            if (movespeed >= MoveStats.T3_Threshold) return 3;
            if (movespeed >= MoveStats.T2_Threshold) return 2;
            if (movespeed >= MoveStats.T1_Threshold) return 1;
            return 0; // walk speed
        }
    }
}
