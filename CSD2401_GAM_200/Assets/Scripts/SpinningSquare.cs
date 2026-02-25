using System;
using CSharpScript;

namespace CSharpScript
{
    public class SpinningSquare : GameScript
    {
        public float RotationSpeed = 2.0f;
        public float MoveSpeed = 150.0f;

        public override void OnStart()
        {
            Components.AddPhysicsComponent(OwnerName);
            Components.AddCollisionComponent(OwnerName);

            Physics.SetMass(OwnerName, 1.0f);
            Physics.SetGravity(OwnerName, false);
            Physics.SetStatic(OwnerName, false);

            Collision.SetLayer(OwnerName, CollisionLayer.Player);

            Debug.Log($"[SpinningSquare] Ready. Move down into a Ground object to destroy.");
        }

        public override void OnUpdate(float dt)
        {
            vec2 dir = Input.Movement();
            Physics.SetVelocity(OwnerName, dir.x * MoveSpeed, dir.y * MoveSpeed);
            Transform.AddRotation(OwnerName, RotationSpeed * dt);

            // Destroy only when moving downward and hitting Ground
            bool movingDown = Physics.GetVelocity(OwnerName).y < 0f;
            if (movingDown && Collision.IsCollidingWithLayer(OwnerName, (int)CollisionLayer.Ground))
            {
                Debug.Log($"[SpinningSquare] Hit Ground while moving down — destroying {OwnerName}.");
                GameObject.Destroy(OwnerName);
            }
        }

        public override void OnDestroy()
        {
            Debug.Log($"[SpinningSquare] OnDestroy called for {OwnerName}.");
        }
    }
}