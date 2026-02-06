/* Start Header ************************************************************************/
/*!
\file       ParticleEmitterComponent.h
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       February 16 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/


#pragma once
#include "Component.h"
#include "MathLib.h"
#include <vector>
#include "GL/glew.h"
#include "transform.h"
#include "gameObject.h"

/**
 * @brief Represents a single particle in the simulation.
 * Uses a struct for CPU-side data storage before being batched for rendering.
 */
struct Particle {
    vec2 position;
    vec2 velocity;
    vec4 color;

    float size;
    float rotation;
    float angularVelocity;
    float lifetime;     // Current age of the particle
    float maxLifeTime;  // How long the particle lives before recycling
    bool active;        // False if the particle is available for reuse

    Particle() : position(0, 0), velocity(0, 0), color(1, 1, 1, 1),
        size(1.0f), rotation(0.0f), angularVelocity(0.0f),
        lifetime(0.f), maxLifeTime(1.0f), active(false) {
    }
};

struct RenderBatch;
struct BatchVertex;

/**
 * @brief A flexible 2D particle system component.
 * Supports burst emissions, continuous streams, and various shapes (circle/box).
 */
class ParticleEmitterComponent : public IComponent {
public:
    ParticleEmitterComponent(GameObject* owner);

    ParticleEmitterComponent(GameObject* owner, int maxParticles);

    virtual ~ParticleEmitterComponent() = default;

    // --- Core Component Overrides ---
    virtual void Update(float dt) override;

    virtual std::string GetName() const override { return "Particle Emitter Component"; }

    virtual std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override;

    // --- Emission Control ---
    void StartEmission(); // Begins spawning particles based on emissionRate

    void StopEmission();  // Stops spawning new particles but lets existing ones finish

    void Burst(int count); // Instantly spawns a specific number of particles

    void Clear();         // Deactivates all current particles immediately

    // --- Configuration: Timing & Limits ---
    void SetEmissionRate(float particlesPerSecond) { emissionRate = particlesPerSecond; };

    void SetMaxParticles(int max); // Resizes the particle pool

    void SetLoop(bool shouldLoop) { loop = shouldLoop; };

    void SetDuration(float seconds) { duration = seconds; }; // Total time the emitter runs

    void SetLifetime(float seconds, float variation = 0.f) {

        particleLifetime = seconds;

        lifetimeVariation = variation;
    }

    // --- Configuration: Spawn Shape ---
    void SetSpawnOffset(const vec2& offset) { spawnOffset = offset; };

    void SetSpawnRadius(float radius) { spawnRadius = radius; }; // Spawns within a circle

    void SetSpawnBox(const vec2& halfExtents) { spawnBox = halfExtents; }; // Spawns within a rectangle

    // --- Configuration: Movement ---
    void SetVelocity(const vec2& direction, float magnitude, float variation = 0.0f) {
        velocityDirection = direction.normalized();
        velocityMagnitude = magnitude;
        velocityVariation = variation;
    }

    void SetAngleSpread(float degrees) { angleSpread = degrees; }; // Randomizes velocity direction

    void SetGravity(const vec2& gravityAccel) { gravity = gravityAccel; };

    void SetDrag(float dragCoefficient) { drag = dragCoefficient; }; // Linear air resistance

    // --- Configuration: Visuals ---
    void SetTexture(GLuint texID) { textureID = texID; };

    void SetStartColor(const vec4& color) { startColor = color; };

    void SetEndColor(const vec4& color) { endColor = color; }; // Lerps from start to end over lifetime

    void SetStartSize(float size) { startSize = size; };

    void SetEndSize(float size) { endSize = size; };

    void SetLayer(int renderLayer) { layer = renderLayer; }; // Controls rendering order depth

    void SetSizeOverLifetimeCurve(bool useStartEnd) { useSizeCurve = useStartEnd; };

    void SetFadeIn(float fadeInTime) { fadeInDuration = fadeInTime; };

    void SetFadeOut(float fadeOutTime) { fadeOutDuration = fadeOutTime; };

    // --- Configuration: Rotation ---
    void SetStartRotation(float degrees, float variation = 0.f) {
        startRotation = degrees;
        startRotationVariation = variation;
    }

    void SetAngularVelocity(float degreesPerSecond, float variation = 0.f) {
        angularVelocity = degreesPerSecond;
        angularVelocityVariation = variation;
    }

    // --- Getters ---
    bool IsEmitting() const { return isEmitting; };

    int GetActiveParticleCount() const;

    int GetMaxParticles() const { return maxParticles; };

    GLuint GetTextureID() const { return textureID; };

    int GetLayer() const { return layer; };

    // --- Rendering ---
    /** Transforms particle data into vertex data for the Graphics System */
    void AddParticlesToBatch(std::vector<RenderBatch>& batches, const mat4& viewProj);

    // --- Presets: Quick configurations for common gameplay effects ---
    void ConfigureDashTrail();

    void ConfigureJumpDust();

    void ConfigureWallSlideParticles();

    void ConfigureSpeedLines();

    void ConfigureDeathExplosion();

private:
    /** Logic for initializing a single particle's properties */
    void SpawnParticle(const vec2& emitterPosition);

    /** Object pooling: Finds the first !active particle in the vector */
    Particle* GetInactiveParticle();

    // Helper math functions
    float RandomRange(float min, float max) const;

    vec2 RandomInCircle(float radius) const;

    vec2 RandomInBox(const vec2& halfExtents) const;

    // --- Internal State ---
    std::vector<Particle> particles; // The particle pool
    int maxParticles;
    bool isEmitting;
    bool loop;
    float duration;      // How long the emitter stays active
    float emissionTimer; // Tracks time between individual particle spawns
    float durationTimer; // Tracks total time spent emitting

    // --- Emission Parameters ---
    float emissionRate;
    float particleLifetime;
    float lifetimeVariation;

    // --- Spatial Parameters ---
    vec2 spawnOffset;
    float spawnRadius;
    vec2 spawnBox;
    float useSpawnBox; // Acts as a toggle/threshold for box vs circle spawning

    // --- Movement Parameters ---
    vec2 velocityDirection;
    float velocityMagnitude;
    float velocityVariation;
    float angleSpread;

    // --- Visual Parameters ---
    GLuint textureID;
    vec4 startColor;
    vec4 endColor;
    float startSize;
    float endSize;
    int layer;
    bool screenSpace = false; // If true, ignore camera transform

    // --- Rotation & Physics Parameters ---
    float startRotation;
    float startRotationVariation;
    float angularVelocity;
    float angularVelocityVariation;
    vec2 gravity;
    float drag;

    // --- Animation Toggles ---
    bool useSizeCurve;
    float fadeInDuration;
    float fadeOutDuration;
};