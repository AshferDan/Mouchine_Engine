//using System.Numerics;

//namespace CSharpScript
//{
//    public class Knockback : GameScript
//    {
//        private float dt => Utils.GetDelta_Time();
//        private float fixeddt => Utils.GetFixed_Delta_Time();

//        public float knockbackTime = 0.2f;
//        public float hitDirectionForce = 10f;
//        public float constForce = 5f;
//        public float inputForce = 7.5f;

//        // animation curve
//        //public AnimationCurve knockbackForceCurve;

//        //private Rigidbody2D rb;

//        //private Coroutine knockbackCoroutine;

//        public bool IsBeingKnockedBack { get; private set; }

//        private void OnStart()
//        {
//            //rb = GetComponent<Rigidbody2D>();
//        }

//        public void KnockbackAction(vec2 hitDirection, vec2 constantForceDirection, float inputDirection)
//        {
//            IsBeingKnockedBack = true;

//            vec2 _hitForce;
//            vec2 _constantForce;
//            vec2 _knockbackForce;
//            vec2 _combinedForce;
//            float _time = 0f;

//            _constantForce = constantForceDirection * constForce;

//            float _elapsedTime = 0f;
//            while (_elapsedTime < knockbackTime)
//            {
//                // iterate the timer
//                _elapsedTime += fixeddt;
//                _time += fixeddt;

//                // update hit force
//                _hitForce = hitDirection * hitDirectionForce * knockbackForceCurve.Evaluate(_time);

//                // combine _hitForce and _constantForce
//                _knockbackForce = _hitForce + _constantForce;

//                // combine knockBackForce with Input Force
//                if (inputDirection != 0)
//                {
//                    _combinedForce = _knockbackForce + new Vector2(inputDirection * inputForce, 0f);
//                }

//                else
//                {
//                    _combinedForce = _knockbackForce;
//                }

//                // apply knockback
//                rb.linearVelocity = _combinedForce;
//                Physics.SetVelocity(OwnerName, _combinedForce.x, _combinedForce.y)

//                yield return new WaitForFixedUpdate();
//            }

//            IsBeingKnockedBack = false;
//        }

//        public void CallKnockback(vec2 hitDirection, vec2 constantForceDirection, float inputDirection)
//        {
//            //knockbackCoroutine = StartCoroutine(KnockbackAction(hitDirection, constantForceDirection, inputDirection));
//        }
//    }
//}