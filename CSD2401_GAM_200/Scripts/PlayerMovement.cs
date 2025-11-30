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

namespace GameScripts
{
    public class PlayerMovement
    {
        // basically assigns names to numbers that can be used to track states
        enum PlayerState
        {
            Idle = 0,
            Running,
            Jumping,
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
        public string gameObjectName = "Hero";

        // movement variables
        private vec2 _moveVelocity = new vec2(0, 0);
        private bool _isFacingRight = true;

        // public facing right for other scripts
        public bool IsFacingRight => _isFacingRight;

        // collision check variables
        private bool _isGrounded = false;
        private bool _bumpedHead = false;

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
        #endregion


        public PlayerMovement()
        {
            // When the script is created, instantiate its stats
            MoveStats = new PlayerMovementStats();
            Debug.Log("PlayerMovement instantiated with new stats.");
        }

        public void OnStart()
        {
            // It's crucial that gameObjectName is set by the C++ engine before OnStart is called.
            if (gameObjectName == null)
            {
                Debug.Log("ERROR: gameObjectName is not set in PlayerMovement script!");
                return;
            }

            _isFacingRight = true;

            Debug.Log("PlayerMovement Script OnStart for object: " + gameObjectName);
            Physics.SetGravity(gameObjectName, true);
            Debug.Log("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n\n\n\n\n\n\n");

            vec2 scale = Transform.GetScale(gameObjectName);
            vec2 position = Transform.GetPosition(gameObjectName);
            //_ziplineHeadOffset = position.y + (scale.y / 2);
            Physics.SetFriction(gameObjectName, 0f);
            Physics.SetMass(gameObjectName, 0.0001f);
            _moveVelocity = new vec2(0, 0);

            Audio.AddSound("jump", "..Assets/Audio/jump.wav", false, 1, 1, "sfx");
        }

        public void OnUpdate(float deltaTime)
        {
            if (gameObjectName == null || gameObjectName == "") return;

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
            //Jump(fixeddt);
            OnFixedUpdate(fixeddt);
        }

        public void OnFixedUpdate(float fixedDeltaTime)
        {
            if (gameObjectName == null) return;
            CollisionChecks();

            if (!_isZiplining) //&& !knockback.IsBeingKnockedBack && !_isBeingLaunched)
            {
                Jump(fixedDeltaTime);
                Dash(fixedDeltaTime);
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
            if (_isDashing) currState = PlayerState.Dashing;
            if (_isRolling) currState = PlayerState.Rolling;
            if (_isGroundPounding) currState = PlayerState.GroundPounding;
            if (_isGliding) currState = PlayerState.Gliding;
            if (_isWallKicking) currState = PlayerState.WallKicking;
            if (_isZiplining) currState = PlayerState.Ziplining;
            if (_isWallSliding) currState = PlayerState.WallSliding;
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

            if (InputManager.DashWasPressed && _dashCooldownTimer <= 0)
            {
                InitiateDash();
            }
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
        }

        private void IsGrounded()
        {
            _isGrounded = false;
            vec2 position = Transform.GetPosition(gameObjectName);
            vec2 scale = Transform.GetScale(gameObjectName);
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
                return;
            }

            // Left raycast
            vec2 leftRayOrigin = new vec2(position.x - halfWidth * 0.9f, bottomY);
            if (RaycastForGround(leftRayOrigin, rayDirection, raycastLength))
            {
                _isGrounded = true;
                _coyoteTimer = MoveStats.JumpCoyoteTime;
                return;
            }

            // Right raycast
            vec2 rightRayOrigin = new vec2(position.x + halfWidth * 0.9f, bottomY);
            if (RaycastForGround(rightRayOrigin, rayDirection, raycastLength))
            {
                _isGrounded = true;
                _coyoteTimer = MoveStats.JumpCoyoteTime;
                return;
            }
        }

        private bool RaycastForGround(vec2 origin, vec2 direction, float length)
        {
            if (Collision.Raycast(gameObjectName, origin, direction, length, out string hitObject, out float hitDistance))
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
            vec2 position = Transform.GetPosition(gameObjectName);
            vec2 scale = Transform.GetScale(gameObjectName);
            float halfHeight = scale.y / 2;
            float raycastLength = MoveStats.WallDetectionBoxLength;
            vec2 rayDirection = new vec2(_isFacingRight ? 1 : -1, 0);

            // Center raycast
            vec2 centerRayOrigin = new vec2(position.x, position.y);
            if (RaycastForWall(centerRayOrigin, rayDirection, raycastLength))
            {
                _isTouchingWall = true;
                Debug.Log("TOUCHING");
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
            if (Collision.Raycast(gameObjectName, origin, direction, length, out string hitObject, out float hitDistance))
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

        #region Movement
        public void Move(float acceleration, float deceleration, vec2 moveInput, float deltaTime)
        {
            if (blockMovement) moveInput = new vec2(0, 0);

            if (moveInput.x != 0 || (_isRolling && _rollTimer >= 0))
            {
                TurnCheck(moveInput);
                vec2 targetVelocity = new vec2(moveInput.x * MoveStats.MaxWalkSpeed, 0f);

                if (!(_isRolling && _rollTimer >= 0))
                {
                    _moveVelocity = Lerp(_moveVelocity, targetVelocity, acceleration * deltaTime);
                }

                Physics.SetVelocity(gameObjectName, _moveVelocity.x, VerticalVelocity);
            }
            else
            {
                _moveVelocity = Lerp(_moveVelocity, new vec2(0, 0), deceleration * deltaTime);

                Physics.SetVelocity(gameObjectName, _moveVelocity.x, VerticalVelocity);
            }
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
            vec2 currentScale = Transform.GetScale(gameObjectName);
            Transform.SetScale(gameObjectName, Math.Abs(currentScale.x) * (turnRight ? 1 : -1), currentScale.y);
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

            Physics.SetVelocity(gameObjectName, _moveVelocity.x, VerticalVelocity);
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

            Audio.PlaySound("jump", "sfx");
            _jumpBufferTimer = 0f;
            _numberOfJumpsUsed += numberOfJumpsUsed;
            VerticalVelocity = MoveStats.InitialJumpVelocity;
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

            //vec2 currentVelocity = Physics.GetVelocity(gameObjectName);
            Physics.SetVelocity(gameObjectName, _moveVelocity.x, _moveVelocity.y);

            // turn the player
            Turn(!_isFacingRight);
        }
        #endregion

        #region Dash

        private void InitiateDash()
        {
            if (_isDashing) return;

            _isWallSliding = false;
            _isWallJumping = false;
            blockMovement = false;

            _isDashing = true;
            _dashTimer = MoveStats.DashDuration;
            _dashCooldownTimer = MoveStats.DashCooldown;

            // determine dash direction
            float dashDirection = _isFacingRight ? 1f : -1f;

            // set horizontal velocity
            _moveVelocity.x = dashDirection * MoveStats.DashSpeed;

            // cancel vertical movement
            VerticalVelocity = 0f;

            // ensure jump velocity does not interfere
            Physics.SetVelocity(gameObjectName, _moveVelocity.x, 0);
        }

        private void Dash(float deltaTime)
        {
            if (_isDashing)
            {
                // maintain dash speed and zero vertical velocity while dashing
                VerticalVelocity = 0f;
                float direction = _isFacingRight ? 1 : -1;
                float move = MoveStats.DashSpeed * deltaTime;

                // Move manually instead of overriding physics velocity
                Vector2 newVec = new Vector2(Transform.GetPosition(gameObjectName).x, Transform.GetPosition(gameObjectName).y);
                newVec += new Vector2(move * direction, 0);
                Transform.SetPosition(gameObjectName, newVec.X, newVec.Y);

                _dashTimer -= deltaTime;

                // check if dash has finished
                if (_dashTimer <= 0)
                {
                    StopDash();
                }
            }
        }

        private void StopDash()
        {
            _isDashing = false;
            //vec2 currentVelocity = Physics.GetVelocity(gameObjectName);
            Physics.SetVelocity(gameObjectName, _moveVelocity.x, 0f);
        }

        #endregion

        #region Wall Kick
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

            Physics.SetVelocity(gameObjectName, _moveVelocity.x, VerticalVelocity);
        }

        private void WallKick(float deltaTime)
        {
            if (!_isWallKicking) return;

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
            Physics.SetVelocity(gameObjectName, currentSpeedX, verticalGain);

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
            Physics.SetVelocity(gameObjectName, _moveVelocity.x, VerticalVelocity);

            wallKickFrameCounter = 0; // Reset wall coyote counter
        }
        #endregion

        #region Wall Slide
        private void WallSlide(float deltaTime)
        {
            bool isPushingTowardsWall = (_isFacingRight && InputManager.Movement.x > 0) ||
                                        (!_isFacingRight && InputManager.Movement.x < 0);

            //vec2 currentVelocity = Physics.GetVelocity(gameObjectName);

            if (_isTouchingWall && !_isGrounded && isPushingTowardsWall)
            {
                if (_moveVelocity.y < 0)
                {
                    _isWallSliding = true;
                    float newYVelocity = Utils.MathfClamp(_moveVelocity.y, -MoveStats.WallSlidingSpeed, 999f);
                    Physics.SetVelocity(gameObjectName, _moveVelocity.x, newYVelocity);
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
            lastPos = Transform.GetPosition(gameObjectName);
        }

        private void GroundPound(float deltaTime)
        {
            if (!_isGroundPounding) return;

            if (_groundPoundPauseTimer > 0f)
            {
                // Pause in midair
                _groundPoundPauseTimer -= deltaTime;

                // Keep player in same position
                Transform.SetPosition(gameObjectName, lastPos.x, lastPos.y);

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
            Physics.SetVelocity(gameObjectName, _moveVelocity.x, VerticalVelocity);
        }

        private void StopGroundPound()
        {
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

        #region Roll

        private void InitiateRoll()
        {
            if (_isRolling || !_isGrounded) return;

            // Play sound if assigned
            //if (sounds != null && sounds.roll != null)
            //    AudioManager.Instance.PlayPlayerSFX(sounds.roll);

            _isRolling = true;
            blockMovement = true;

            // Roll timer; longer if coming from ground pound
            _rollTimer = (!_isGroundPounding) ? MoveStats.BaseRollDuration : MoveStats.BaseRollDuration + (lastPos.y - Transform.GetPosition(gameObjectName).y) / MoveStats.RollHeightToDurationDivisor;
            _rollTimer = Utils.MathfClamp(_rollTimer, 0f, 2f);

            Debug.Log(_rollTimer.ToString());

            _isGroundPounding = false;

            // Set roll velocity
            _moveVelocity.x = _isFacingRight ? MoveStats.RollSpeed : -MoveStats.RollSpeed;
            VerticalVelocity = 0f;
        }

        private void Roll(float deltaTime)
        {
            if (!_isRolling) return;

            // Cancel roll if touching wall in air
            if (canWallKick)
            {
                StopRoll();
                return;
            }

            // End roll on timer or dash
            if (_rollTimer <= 0f || _isDashing)
            {
                blockMovement = false;
                StopRoll();
            }
            else
            {
                _rollTimer -= deltaTime;
                // Keep horizontal roll speed
                Physics.SetVelocity(gameObjectName, _moveVelocity.x, VerticalVelocity);
            }
        }

        private void StopRoll()
        {
            if (!_isRolling) return;

            _isRolling = false;
            blockMovement = false;

            // Reset horizontal velocity
            _moveVelocity.x = 0f;
            Physics.SetVelocity(gameObjectName, _moveVelocity.x, VerticalVelocity);
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

            Physics.SetVelocity(gameObjectName, _moveVelocity.x, VerticalVelocity);
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
            Debug.Log("PlayerMovement Script Destroyed for object: " + gameObjectName);
        }

        private static vec2 Lerp(vec2 a, vec2 b, float t)
        {
            t = Utils.MathfClamp(t, 0, 1);
            return new vec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }
    }
}
