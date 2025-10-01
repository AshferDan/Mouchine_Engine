#include "Physics.h"
#include "Transform.h"
#include "GameObject.h"
#include <iostream>
#include <cmath>

PhysicsComponent::PhysicsComponent(GameObject* owner, float m, bool static_)
    : IComponent(owner),
    velocityX(0), velocityY(0),
    mass(m), isStatic(static_), useGravity(true),
    isGrounded(true), isJumping(false), wasGrounded(true),
    gravityX(0), gravityY(-9.81f) {
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

void PhysicsComponent::Jump(float jumpForce) {
    if (isGrounded) {
        velocityY = jumpForce;
        isJumping = true;
        isGrounded = false;

        std::cout << owner->GetName() << " jumped with force " << jumpForce
            << "! New VelY: " << velocityY << std::endl;
    }
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

void PhysicsComponent::UpdatePosition(float deltaTime) {
    auto* transform = owner->GetComponent<TransformComponent>();
    if (transform) {
        transform->Move(velocityX * deltaTime, velocityY * deltaTime);
    }
}