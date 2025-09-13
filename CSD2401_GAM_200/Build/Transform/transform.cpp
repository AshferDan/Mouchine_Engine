 #include "transform.h"
 #include "GameObject.h"
 #include <iostream>

 TransformComponent::TransformComponent(GameObject* owner, float x, float y, float z)
     : IComponent(owner), posX(x), posY(y), posZ(z),
       rotX(0), rotY(0), rotZ(0),
       scaleX(1), scaleY(1), scaleZ(1) {
 }

 void TransformComponent::Update(float deltaTime) {
     // Debug output for transform changes
     static float lastPosX = posX, lastPosY = posY, lastPosZ = posZ;

     if (posX != lastPosX || posY != lastPosY || posZ != lastPosZ) {
         std::cout << owner->GetName() << " Transform Update - Position changed to ("
                   << posX << ", " << posY << ", " << posZ << ")" << std::endl;
         lastPosX = posX; lastPosY = posY; lastPosZ = posZ;
     }
 }

 void TransformComponent::SetPosition(float x, float y, float z) {
     posX = x; posY = y; posZ = z;
 }

 void TransformComponent::GetPosition(float& x, float& y, float& z) const {
     x = posX; y = posY; z = posZ;
 }

 void TransformComponent::SetRotation(float x, float y, float z) {
     rotX = x; rotY = y; rotZ = z;
 }

 void TransformComponent::GetRotation(float& x, float& y, float& z) const {
     x = rotX; y = rotY; z = rotZ;
 }

 void TransformComponent::SetScale(float x, float y, float z) {
     scaleX = x; scaleY = y; scaleZ = z;
 }

 void TransformComponent::GetScale(float& x, float& y, float& z) const {
     x = scaleX; y = scaleY; z = scaleZ;
 }

 void TransformComponent::Move(float deltaX, float deltaY, float deltaZ) {
     posX += deltaX;
     posY += deltaY;
     posZ += deltaZ;
 }

 void TransformComponent::Rotate(float deltaX, float deltaY, float deltaZ) {
     rotX += deltaX;
     rotY += deltaY;
     rotZ += deltaZ;
 }