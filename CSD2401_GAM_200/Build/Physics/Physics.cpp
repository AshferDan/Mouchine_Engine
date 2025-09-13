#include "Physics.h"
 #include "Transform.h"
 #include "GameObject.h"
 #include <iostream>

 PhysicsComponent::PhysicsComponent(GameObject* owner, float m, bool static_)
     : IComponent(owner), 
       velocityX(0), velocityY(0), velocityZ(0),
       forceX(0), forceY(0), forceZ(0),
       mass(m), isStatic(static_), useGravity(true) {
 }

 void PhysicsComponent::Update(float deltaTime) {
     if (isStatic) return;

     std::cout << owner->GetName() << " Physics Update - ";

     // Apply gravity
     if (useGravity) {
         ApplyGravity(deltaTime);
     }

     // Apply forces to velocity
     velocityX += (forceX / mass) * deltaTime;
     velocityY += (forceY / mass) * deltaTime;
     velocityZ += (forceZ / mass) * deltaTime;

     // Reset forces
     forceX = forceY = forceZ = 0;

     // Update position
     UpdatePosition(deltaTime);

     // Debug output
     auto* transform = owner->GetComponent<TransformComponent>();
     if (transform) {
         std::cout << "Pos(" << transform->posX << ", " << transform->posY << ", " << transform->posZ << ") ";
         std::cout << "Vel(" << velocityX << ", " << velocityY << ", " << velocityZ << ")" << std::endl;
     }
 }

 void PhysicsComponent::SetVelocity(float x, float y, float z) {
     velocityX = x;
     velocityY = y;
     velocityZ = z;
 }

 void PhysicsComponent::AddForce(float x, float y, float z) {
     forceX += x;
     forceY += y;
     forceZ += z;
 }

 void PhysicsComponent::ApplyGravity(float deltaTime) {
     const float gravity = 0.0f;
     velocityY += gravity * deltaTime;
 }

 void PhysicsComponent::UpdatePosition(float deltaTime) {
     auto* transform = owner->GetComponent<TransformComponent>();
     if (transform) {
         transform->Move(velocityX * deltaTime, velocityY * deltaTime, velocityZ * deltaTime);
     }
 }