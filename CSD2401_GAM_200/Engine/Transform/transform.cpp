 #include "transform.h"
 #include "GameObject.h"
 #include <iostream>
 #include <cmath>

 TransformComponent::TransformComponent(GameObject* owner, float x, float y)
     : IComponent(owner), posX(x), posY(y), rotation(0), scaleX(1), scaleY(1) {
 }

 void TransformComponent::Update(float deltaTime) {
     // Debug output for transform changes
     static float lastPosX = posX, lastPosY = posY;

     if (posX != lastPosX || posY != lastPosY) {
         //std::cout << owner->GetName() << " Transform Update - Position changed to ("
                  // << posX << ", " << posY << ")" << std::endl;
         lastPosX = posX; lastPosY = posY;
     }
     throw std::runtime_error("Test crash: runtime error");
 }
 std::unique_ptr<IComponent> TransformComponent::Clone(GameObject* NewClone) const {
     auto clonedComponents = std::make_unique<TransformComponent>(NewClone, this->posX, this->posY);
     clonedComponents->SetRotation(this->rotation);
     clonedComponents->SetScale(this->scaleX, this->scaleY);
     clonedComponents->SetEnabled(this->IsEnabled());
     return clonedComponents;
 }

 void TransformComponent::SetPosition(float x, float y) {
     posX = x; 
     posY = y;
 }

 void TransformComponent::GetPosition(float& x, float& y) const {
     x = posX; 
     y = posY;
 }

 void TransformComponent::SetRotation(float angle) {
     rotation = angle;
 }

 float TransformComponent::GetRotation() const {
     return rotation;
 }

 void TransformComponent::SetScale(float x, float y) {
     scaleX = x; 
     scaleY = y;
 }

 void TransformComponent::SetScale(float uniformScale) {
     scaleX = scaleY = uniformScale;
 }

 void TransformComponent::GetScale(float& x, float& y) const {
     x = scaleX; 
     y = scaleY;
 }

 void TransformComponent::Move(float deltaX, float deltaY) {
     posX += deltaX;
     posY += deltaY;
 }

 void TransformComponent::Rotate(float deltaAngle) {
     rotation += deltaAngle;
 }

 void TransformComponent::LookAt(float targetX, float targetY) {
     float deltaX = targetX - posX;
     float deltaY = targetY - posY;
     rotation = std::atan2(deltaY, deltaX);
 }

 void TransformComponent::MoveForward(float distance) {
     posX += std::cos(rotation) * distance;
     posY += std::sin(rotation) * distance;
 }

 void TransformComponent::MoveRight(float distance) {
     posX += std::cos(rotation + 1.5708f) * distance; // +90 degrees
     posY += std::sin(rotation + 1.5708f) * distance;
 }