 #include "Physics.h"
 #include "Transform.h"
 #include "GameObject.h"
 #include <iostream>
 #include <cmath>

 PhysicsComponent::PhysicsComponent(GameObject* owner, float m, bool static_)
     : IComponent(owner), 
       velocityX(0), velocityY(0),
       forceX(0), forceY(0),
       angularVelocity(0), torque(0),
       mass(m), drag(0.98f), isStatic(static_), useGravity(true),
       gravityX(0), gravityY(-9.81f) {
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

     // Apply torque to angular velocity
     angularVelocity += (torque / mass) * deltaTime;

     // Reset forces and torque
     forceX = forceY = torque = 0;

     // Apply drag
     ApplyDrag(deltaTime);

     // Update position and rotation
     UpdatePosition(deltaTime);
     UpdateRotation(deltaTime);

     // Debug output
     auto* transform = owner->GetComponent<TransformComponent>();
     if (transform) {
         std::cout << "Pos(" << transform->posX << ", " << transform->posY << ") ";
         std::cout << "Vel(" << velocityX << ", " << velocityY << ") ";
         std::cout << "Rot(" << transform->rotation << ")" << std::endl;
     }
 }

 void PhysicsComponent::SetVelocity(float x, float y) {
     velocityX = x;
     velocityY = y;
 }

 void PhysicsComponent::GetVelocity(float& x, float& y) const {
     x = velocityX;
     y = velocityY;
 }

 void PhysicsComponent::AddVelocity(float x, float y) {
     velocityX += x;
     velocityY += y;
 }

 void PhysicsComponent::AddForce(float x, float y) {
     forceX += x;
     forceY += y;
 }

 void PhysicsComponent::AddImpulse(float x, float y) {
     velocityX += x / mass;
     velocityY += y / mass;
 }

 void PhysicsComponent::AddTorque(float t) {
     torque += t;
 }

 float PhysicsComponent::GetSpeed() const {
     return std::sqrt(velocityX * velocityX + velocityY * velocityY);
 }

 float PhysicsComponent::GetDirection() const {
     return std::atan2(velocityY, velocityX);
 }

 void PhysicsComponent::SetDirection(float angle, float speed) {
     velocityX = std::cos(angle) * speed;
     velocityY = std::sin(angle) * speed;
 }

 void PhysicsComponent::ApplyGravity(float deltaTime) {
     velocityX += gravityX * deltaTime;
     velocityY += gravityY * deltaTime;
 }

 void PhysicsComponent::ApplyDrag(float deltaTime) {
     velocityX *= std::pow(drag, deltaTime);
     velocityY *= std::pow(drag, deltaTime);
     angularVelocity *= std::pow(drag, deltaTime);
 }

 void PhysicsComponent::UpdatePosition(float deltaTime) {
     auto* transform = owner->GetComponent<TransformComponent>();
     if (transform) {
         transform->Move(velocityX * deltaTime, velocityY * deltaTime);
     }
 }

 void PhysicsComponent::UpdateRotation(float deltaTime) {
     auto* transform = owner->GetComponent<TransformComponent>();
     if (transform) {
         transform->Rotate(angularVelocity * deltaTime);
     }
 }