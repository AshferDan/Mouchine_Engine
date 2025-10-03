/* Start Header ************************************************************************/
/*!
\file       tranform.cpp
\author     Leck Xin Yee [53% - 48 lines]
\co-author	Lee Hwee Min [47% - 43 lines]
\par        xinyee.leck@digipen.edu, l.hweemin@digipen.edu
\date       October, 3, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

 #include "transform.h"
 #include "GameObject.h"
 #include <iostream>
 #include <cmath>

 /**
  * @brief Constructs a TransformComponent with initial position and default rotation/scale
  */
 TransformComponent::TransformComponent(GameObject* owner, float x, float y)
     : IComponent(owner), posX(x), posY(y), rotation(0), scaleX(1), scaleY(1) {
 }

 /**
 * @brief Updates the transform and tracks position changes for debugging
 */
 void TransformComponent::Update(float deltaTime) {
     // Debug output for transform changes
     static float lastPosX = posX, lastPosY = posY;

     if (posX != lastPosX || posY != lastPosY) {
         //std::cout << owner->GetName() << " Transform Update - Position changed to ("
                  // << posX << ", " << posY << ")" << std::endl;
         lastPosX = posX; lastPosY = posY;
     }
     //throw std::runtime_error("Test crash: runtime error");
 }

 /**
 * @brief Creates a deep copy of this TransformComponent for a new GameObject
 */
 std::unique_ptr<IComponent> TransformComponent::Clone(GameObject* NewClone) const {
     auto clonedComponents = std::make_unique<TransformComponent>(NewClone, this->posX, this->posY);
     clonedComponents->rotation=this->rotation;
     clonedComponents->scaleX = this->scaleX;
     clonedComponents->scaleY = this->scaleY;

     clonedComponents->SetEnabled(this->IsEnabled());
     return clonedComponents;
 }

 /**
 * @brief Sets the absolute position of the transform
 */
 void TransformComponent::SetPosition(float x, float y) {
     posX = x; 
     posY = y;
 }

 /**
 * @brief Gets the current position coordinates
 */
 void TransformComponent::GetPosition(float& x, float& y) const {
     x = posX; 
     y = posY;
 }

 /**
 * @brief Sets the absolute rotation angle in radians
 */
 void TransformComponent::SetRotation(float angle) {
     rotation = angle;
 }

 /**
 * @brief Gets the current rotation angle in radians
 */
 float TransformComponent::GetRotation() const {
     return rotation;
 }

 /**
 * @brief Sets separate scale factors for X and Y axes
 */
 void TransformComponent::SetScale(float x, float y) {
     scaleX = x; 
     scaleY = y;
 }

 /**
 * @brief Sets uniform scale factor for both axes
 */
 void TransformComponent::SetScale(float uniformScale) {
     scaleX = scaleY = uniformScale;
 }

 /**
 * @brief Gets the current scale factors for both axes
 */
 void TransformComponent::GetScale(float& x, float& y) const {
     x = scaleX; 
     y = scaleY;
 }

 /**
 * @brief Moves the transform by relative delta values
 */
 void TransformComponent::Move(float deltaX, float deltaY) {
     posX += deltaX;
     posY += deltaY;
 }

 /**
 * @brief Rotates the transform by a relative delta angle
 */
 void TransformComponent::Rotate(float deltaAngle) {
     rotation += deltaAngle;
 }

 /**
 * @brief Calculates and sets rotation to face a target position using atan2
 */
 void TransformComponent::LookAt(float targetX, float targetY) {
     float deltaX = targetX - posX;
     float deltaY = targetY - posY;
     rotation = std::atan2(deltaY, deltaX);
 }

 /**
 * @brief Moves forward in the direction of current rotation
 */
 void TransformComponent::MoveForward(float distance) {
     posX += std::cos(rotation) * distance;
     posY += std::sin(rotation) * distance;
 }

 /**
 * @brief Moves right perpendicular to current rotation direction
 */
 void TransformComponent::MoveRight(float distance) {
     posX += std::cos(rotation + 1.5708f) * distance; // +90 degrees
     posY += std::sin(rotation + 1.5708f) * distance;
 }