/* Start Header ************************************************************************/
/*!
\file       Forces.h
\author     Force System
\date       November, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <memory>
#include <cmath>

// Forward declaration
class Force;

/**
 * @brief Types of forces available in the system
 */
enum class ForceType {
    Jump,
    MoveRight,
    MoveLeft,
    Wind,
    Boost
};

/**
 * @brief Force constants - all force values defined here
 */
namespace ForceConstants {
    // Gravity
    /*constexpr float GRAVITY_MAGNITUDE = 500.0f;*/

    // Jump
    constexpr float JUMP_FORCE = 1500.0f;
    constexpr float JUMP_DURATION = 0.5f;  // Very short impulse for realistic jump arc

    // Movement
    constexpr float MOVE_FORCE = 800.0f;

    // Drag
    constexpr float DRAG_COEFFICIENT = 5.0f;
}

/**
 * @brief Base class for all force types
 */
struct Force {
    bool isActive;
    float lifetime;  // -1 for infinite
    float age;       // Time since creation

    Force() : isActive(true), lifetime(-1.0f), age(0.0f) {}
    virtual ~Force() = default;

    virtual void CalculateAcceleration(float mass, float velocityX, float velocityY,
        float& accelX, float& accelY) = 0;
};

/**
 * @brief Linear directional force with unit direction and magnitude
 */
struct LinearDirectionalForce : public Force {
    float unitDirectionX;
    float unitDirectionY;
    float magnitude;
    ForceType type;

    LinearDirectionalForce(float dirX, float dirY, float mag, float life, ForceType forceType)
        : magnitude(mag), type(forceType) {
        float length = std::sqrt(dirX * dirX + dirY * dirY);
        if (length > 0.0001f) {
            unitDirectionX = dirX / length;
            unitDirectionY = dirY / length;
        }
        else {
            unitDirectionX = 0.0f;
            unitDirectionY = 0.0f;
        }
        lifetime = life;
    }

    void CalculateAcceleration(float mass, float velocityX, float velocityY,
        float& accelX, float& accelY) override {
        if (!isActive) {
            accelX = 0.0f;
            accelY = 0.0f;
            return;
        }

        float forceX = unitDirectionX * magnitude;
        float forceY = unitDirectionY * magnitude;

        accelX = forceX / mass;
        accelY = forceY / mass;
    }
};

/**
 * @brief Drag force opposing motion
 */
struct DragForce : public Force {
    float dragCoefficient;

    DragForce(float drag) : dragCoefficient(drag) {
        lifetime = -1.0f;
    }

    void CalculateAcceleration(float mass, float velocityX, float velocityY,
        float& accelX, float& accelY) override {
        if (!isActive) {
            accelX = 0.0f;
            accelY = 0.0f;
            return;
        }

        float dragForceX = -dragCoefficient * velocityX;
        float dragForceY = -dragCoefficient * velocityY;

        accelX = dragForceX / mass;
        accelY = dragForceY / mass;
    }
};