/* Start Header ************************************************************************/
/*!
\file       ParticleEmitterComponent.cpp
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       February 16 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ParticleEmitterComponent.h"
#include "GraphicsSystem.h"
#include <algorithm>
#include <cmath>
#include <random>

// Static random number generators for particle randomization
static std::random_device rd;      // Random seed from hardware
static std::mt19937 gen(rd());     // Mersenne Twister random generator

/**
 * @brief Constructor for ParticleEmitterComponent
 * @param owner The GameObject that owns this component
 *
 * Initializes all particle system parameters with default values.
 * Creates a particle pool of the default maximum size (100 particles).
 */
ParticleEmitterComponent::ParticleEmitterComponent(GameObject* owner) :
	IComponent(owner),
	maxParticles(100),              // Default particle pool size
	isEmitting(false),              // Not emitting initially
	loop(true),                     // Loop emission by default
	duration(-1.f),                 // Infinite duration by default
	emissionTimer(0.0f),            // Timer for emission rate
	durationTimer(0.0f),            // Timer for emission duration
	emissionRate(10.0f),            // 10 particles per second
	particleLifetime(1.0f),         // 1 second lifetime
	lifetimeVariation(0.0f),        // No variation initially
	spawnOffset(0, 0),              // Spawn at owner position
	spawnRadius(0.0f),              // No circular spawn area
	spawnBox(0, 0),                 // No box spawn area
	useSpawnBox(false),             // Use point spawn by default
	velocityDirection(0, 1),        // Upward direction
	velocityMagnitude(50.0f),       // Medium speed
	velocityVariation(0.0f),        // No velocity variation
	angleSpread(0.0f),              // No angle spread (single direction)
	textureID(0),                   // Default texture (none or white)
	startColor(1, 1, 1, 1),         // White, fully opaque
	endColor(1, 1, 1, 0),           // White, fully transparent
	startSize(10.0f),               // Starting size
	endSize(5.0f),                  // Ending size
	layer(0),                       // Render layer
	startRotation(0.0f),            // No rotation
	startRotationVariation(0.0f),   // No rotation variation
	angularVelocity(0.0f),          // No spinning
	angularVelocityVariation(0.0f), // No spin variation
	gravity(0, 0),                  // No gravity
	drag(0.0f),                     // No drag/resistance
	useSizeCurve(true),             // Interpolate size over lifetime
	fadeInDuration(0.0f),           // No fade in
	fadeOutDuration(0.3f)           // 0.3 second fade out
{
	particles.resize(maxParticles); // Allocate particle pool
}

/**
 * @brief Constructor with custom maximum particles
 * @param owner The GameObject that owns this component
 * @param maxParts Maximum number of particles in the pool
 *
 * Delegates to default constructor, then sets custom particle count.
 */
ParticleEmitterComponent::ParticleEmitterComponent(GameObject* owner, int maxParts)
	: ParticleEmitterComponent(owner)
{
	SetMaxParticles(maxParts);
}

/**
 * @brief Updates the particle system
 * @param dt Delta time in seconds since last update
 *
 * Handles particle emission, lifetime updates, physics simulation,
 * and visual property interpolation (color, size, rotation).
 */
void ParticleEmitterComponent::Update(float dt) {
	if (!IsEnabled()) {
		return;  // Skip if component is disabled
	}

	// Get emitter position from owner's transform
	auto* transform = owner->GetComponent<TransformComponent>();
	if (!transform) {
		return;  // Can't emit without a transform
	}

	float x, y;
	transform->GetPosition(x, y);
	vec2 emitterPos(x, y);

	// Handle emission duration/timing
	if (isEmitting && duration > 0.0f) {
		durationTimer += dt;
		if (durationTimer >= duration) {
			if (loop) {
				durationTimer = 0.0f;  // Reset for looping
			}
			else {
				isEmitting = false;    // Stop emission
			}
		}
	}

	// Emit new particles based on emission rate
	if (isEmitting) {
		emissionTimer += dt;
		float timePerParticle = 1.0f / emissionRate;  // Time between particle spawns

		// Spawn particles if enough time has accumulated
		while (emissionTimer >= timePerParticle) {
			SpawnParticle(emitterPos);
			emissionTimer -= timePerParticle;
		}
	}

	// Update all active particles
	for (auto& p : particles) {
		if (!p.active) continue;  // Skip inactive particles

		p.lifetime += dt;  // Age particle

		// Check if particle has expired
		if (p.lifetime >= p.maxLifeTime) {
			p.active = false;
			continue;
		}

		// Apply physics
		p.velocity += gravity * dt;        // Apply gravity
		p.velocity *= (1.0f - drag * dt);  // Apply drag (air resistance)
		p.position += p.velocity * dt;     // Update position
		p.rotation += p.angularVelocity * dt;  // Update rotation

		// Calculate lifetime progress (0 to 1)
		float t = p.lifetime / p.maxLifeTime;

		// Interpolate color from start to end
		p.color.x = startColor.x + (endColor.x - startColor.x) * t;
		p.color.y = startColor.y + (endColor.y - startColor.y) * t;
		p.color.z = startColor.z + (endColor.z - startColor.z) * t;
		p.color.w = startColor.w + (endColor.w - startColor.w) * t;

		// Apply fade-in effect if enabled
		if (fadeInDuration > 0.0f && p.lifetime < fadeInDuration) {
			float fadeT = p.lifetime / fadeInDuration;  // 0 to 1 during fade-in
			p.color.w *= fadeT;  // Multiply alpha by fade-in progress
		}

		// Apply fade-out effect if enabled
		if (fadeOutDuration > 0.0f && p.lifetime > (p.maxLifeTime - fadeOutDuration)) {
			float fadeT = (p.maxLifeTime - p.lifetime) / fadeOutDuration;  // 1 to 0 during fade-out
			p.color.w *= fadeT;  // Multiply alpha by fade-out progress
		}

		// Interpolate size if size curve is enabled
		if (useSizeCurve) {
			p.size = startSize + (endSize - startSize) * t;
		}
	}
}

/**
 * @brief Starts particle emission
 *
 * Resets timers and begins emitting particles according to emission rate.
 */
void ParticleEmitterComponent::StartEmission() {
	isEmitting = true;
	durationTimer = 0.0f;  // Reset duration timer
	emissionTimer = 0.0f;  // Reset emission timer
}

/**
 * @brief Stops particle emission
 *
 * Immediately stops emitting new particles. Existing particles continue
 * their lifetime until they expire.
 */
void ParticleEmitterComponent::StopEmission() {
	isEmitting = false;
}

/**
 * @brief Emits a burst of particles
 * @param count Number of particles to emit immediately
 *
 * Spawns the specified number of particles all at once, regardless of
 * emission rate. Useful for explosion effects.
 */
void ParticleEmitterComponent::Burst(int count) {
	auto* transform = owner->GetComponent<TransformComponent>();
	if (!transform) {
		return;  // Can't emit without transform
	}

	float x, y;
	transform->GetPosition(x, y);
	vec2 emitterPos(x, y);

	// Spawn requested number of particles
	for (int i = 0; i < count; i++) {
		SpawnParticle(emitterPos);
	}
}

/**
 * @brief Clears all particles
 *
 * Immediately deactivates all particles in the system.
 */
void ParticleEmitterComponent::Clear() {
	for (auto& p : particles) {
		p.active = false;
	}
}

/**
 * @brief Sets the maximum number of particles
 * @param max New maximum particle count
 *
 * Reallocates the particle pool with the new size.
 * Any existing particles are lost.
 */
void ParticleEmitterComponent::SetMaxParticles(int max) {
	maxParticles = max;
	particles.clear();           // Remove existing particles
	particles.resize(maxParticles);  // Allocate new pool
}

/**
 * @brief Gets the count of currently active particles
 * @return Number of active particles
 */
int ParticleEmitterComponent::GetActiveParticleCount() const {
	int count = 0;
	for (const auto& p : particles) {
		if (p.active) {
			++count;
		}
	}
	return count;
}

/**
 * @brief Adds particle vertices to render batches
 * @param batches Reference to vector of render batches
 * @param viewProj View-projection matrix (unused in current implementation)
 *
 * Collects all active particles and adds them as quads to the appropriate
 * render batch based on texture and layer.
 */
void ParticleEmitterComponent::AddParticlesToBatch(std::vector<RenderBatch>& batches, const mat4&) {
	if (!IsEnabled()) {
		return;  // Skip if component disabled
	}

	// Find existing batch for this texture/layer combination
	RenderBatch* targetBatch = nullptr;
	for (auto& batch : batches) {
		if (batch.textureID == textureID &&
			batch.isCircle == false &&
			batch.layer == layer &&
			batch.screenSpace == screenSpace) {
			targetBatch = &batch;
			break;
		}
	}

	// Create new batch if none exists for this configuration
	if (!targetBatch) {
		batches.emplace_back(textureID, false, layer, false);
		targetBatch = &batches.back();
		targetBatch->vertices.reserve(maxParticles * 4);  // 4 vertices per particle quad
		targetBatch->indices.reserve(maxParticles * 6);   // 6 indices per particle quad (2 triangles)
	}

	// Add each active particle as a quad
	for (const auto& p : particles) {
		if (!p.active) {
			continue;  // Skip inactive particles
		}

		// Define quad vertices in local space (centered at origin)
		vec3 localVerts[4] = {
			{-0.5f, -0.5f, 0.f},  // Bottom-left
			{ 0.5f, -0.5f, 0.f},  // Bottom-right
			{-0.5f,  0.5f, 0.f},  // Top-left
			{ 0.5f,  0.5f, 0.f}   // Top-right
		};

		// Texture coordinates for full texture
		vec2 texCoords[4]{
			{0.f, 0.f},  // Bottom-left UV
			{1.f, 0.f},  // Bottom-right UV
			{0.f, 1.f},  // Top-left UV
			{1.f, 1.f}   // Top-right UV
		};

		// Starting index for this particle's vertices
		GLuint baseIndex = (GLuint)targetBatch->vertices.size();

		// Create 4 vertices for the particle quad
		for (int i = 0; i < 4; i++) {
			BatchVertex vert;
			vert.pos = localVerts[i];              // Local position (will be transformed in shader)
			vert.tex = texCoords[i];               // Texture coordinate
			vert.color = p.color;                  // Particle color with alpha
			vert.uvScale = vec2(1.0f, 1.0f);       // Default UV scaling
			vert.uvOffset = vec2(0.0f, 0.0f);      // Default UV offset
			vert.transform = vec3(p.position.x, p.position.y, p.rotation);  // World transform
			vert.scale = vec2(p.size, p.size);     // Particle size (uniform scaling)

			targetBatch->vertices.push_back(vert);
		}

		// Add indices for two triangles forming the quad
		// Triangle 1: bottom-left, bottom-right, top-left
		targetBatch->indices.push_back(baseIndex + 0);
		targetBatch->indices.push_back(baseIndex + 1);
		targetBatch->indices.push_back(baseIndex + 2);

		// Triangle 2: top-left, bottom-right, top-right
		targetBatch->indices.push_back(baseIndex + 2);
		targetBatch->indices.push_back(baseIndex + 1);
		targetBatch->indices.push_back(baseIndex + 3);
	}
}

/**
 * @brief Spawns a new particle at the specified position
 * @param emitterPosition World position to spawn particle from
 *
 * Finds an inactive particle in the pool and initializes it with
 * randomized properties based on emitter settings.
 */
void ParticleEmitterComponent::SpawnParticle(const vec2& emitterPosition) {
	Particle* p = GetInactiveParticle();  // Get available particle from pool
	if (!p) {
		return;  // No available particles (pool is full)
	}

	p->active = true;
	p->lifetime = 0.0f;
	// Randomize lifetime within variation range
	p->maxLifeTime = particleLifetime + RandomRange(-lifetimeVariation, lifetimeVariation);

	// Calculate spawn position with offsets and spawn area
	vec2 spawnPos = emitterPosition + spawnOffset;  // Apply base offset
	if (useSpawnBox) {
		spawnPos += RandomInBox(spawnBox);          // Random position within box
	}
	else if (spawnRadius > 0.0f) {
		spawnPos += RandomInCircle(spawnRadius);    // Random position within circle
	}
	p->position = spawnPos;

	// Calculate initial velocity with angle spread
	float angle = RandomRange(-angleSpread / 2.0f, angleSpread / 2.0f);  // Random angle within spread
	float radians = angle * pi / 180.0f;  // Convert to radians
	float cosA = cos(radians);
	float sinA = sin(radians);

	// Rotate velocity direction by random angle
	vec2 rotatedDir(velocityDirection.x * cosA - velocityDirection.y * sinA,
		velocityDirection.x * sinA + velocityDirection.y * cosA);

	// Apply randomized speed
	float speed = velocityMagnitude + RandomRange(-velocityVariation, velocityVariation);
	p->velocity = rotatedDir * speed;

	// Set initial visual properties
	p->color = startColor;
	p->size = startSize;

	// Set rotation with variation
	p->rotation = startRotation + RandomRange(-startRotationVariation, startRotationVariation);
	p->angularVelocity = angularVelocity + RandomRange(-angularVelocityVariation, angularVelocityVariation);
}

/**
 * @brief Finds an inactive particle in the pool
 * @return Pointer to inactive particle, or nullptr if all particles are active
 *
 * Searches through the particle pool for the first inactive particle.
 * This implements a simple object pooling system for performance.
 */
Particle* ParticleEmitterComponent::GetInactiveParticle() {
	for (auto& p : particles) {
		if (!p.active) {
			return &p;  // Return first inactive particle
		}
	}
	return nullptr;  // All particles are active (pool full)
}

/**
 * @brief Generates a random float within a range
 * @param min Minimum value (inclusive)
 * @param max Maximum value (inclusive)
 * @return Random float between min and max
 */
float ParticleEmitterComponent::RandomRange(float min, float max) const {
	std::uniform_real_distribution<float> dist(min, max);
	return dist(gen);
}

/**
 * @brief Generates a random point within a circle
 * @param radius Circle radius
 * @return Random vec2 position within the circle
 *
 * Uses square root to ensure uniform distribution within the circle area.
 */
vec2 ParticleEmitterComponent::RandomInCircle(float radius) const {
	std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * pi);  // Random angle
	std::uniform_real_distribution<float> radiusDist(0.0f, radius);     // Random radius

	float angle = angleDist(gen);
	float r = sqrt(radiusDist(gen));  // sqrt for uniform distribution

	return vec2(r * cos(angle), r * sin(angle));
}

/**
 * @brief Generates a random point within a rectangle
 * @param halfExtents Half dimensions of the box (x=half-width, y=half-height)
 * @return Random vec2 position within the box
 */
vec2 ParticleEmitterComponent::RandomInBox(const vec2& halfExtents) const {
	std::uniform_real_distribution<float> xDist(-halfExtents.x, halfExtents.x);  // Random X
	std::uniform_real_distribution<float> yDist(-halfExtents.y, halfExtents.y);  // Random Y

	return vec2(xDist(gen), yDist(gen));
}

/**
 * @brief Creates a deep copy of this component
 * @param newOwner GameObject that will own the cloned component
 * @return Unique pointer to the cloned component
 *
 * Copies all particle system properties to a new instance.
 * Particle pool contents are NOT copied (new instance starts empty).
 */
std::unique_ptr<IComponent> ParticleEmitterComponent::Clone(GameObject* newOwner) const {
	// Create new emitter with same max particles
	auto clone = std::make_unique<ParticleEmitterComponent>(newOwner, maxParticles);

	// Copy all configuration properties
	clone->emissionRate = emissionRate;
	clone->loop = loop;
	clone->duration = duration;
	clone->particleLifetime = particleLifetime;
	clone->lifetimeVariation = lifetimeVariation;
	clone->spawnOffset = spawnOffset;
	clone->spawnRadius = spawnRadius;
	clone->spawnBox = spawnBox;
	clone->useSpawnBox = useSpawnBox;
	clone->velocityDirection = velocityDirection;
	clone->velocityMagnitude = velocityMagnitude;
	clone->velocityVariation = velocityVariation;
	clone->angleSpread = angleSpread;
	clone->textureID = textureID;
	clone->startColor = startColor;
	clone->endColor = endColor;
	clone->startSize = startSize;
	clone->endSize = endSize;
	clone->layer = layer;
	clone->startRotation = startRotation;
	clone->startRotationVariation = startRotationVariation;
	clone->angularVelocity = angularVelocity;
	clone->angularVelocityVariation = angularVelocityVariation;
	clone->gravity = gravity;
	clone->drag = drag;
	clone->useSizeCurve = useSizeCurve;
	clone->fadeInDuration = fadeInDuration;
	clone->fadeOutDuration = fadeOutDuration;

	// Copy enabled state
	clone->SetEnabled(IsEnabled());

	return clone;
}

// =====================================================================
// PRESET CONFIGURATIONS FOR GAME
// =====================================================================

/**
 * @brief Configures the emitter for a dash trail effect
 *
 * Creates a smooth, trailing effect that follows the player during dashes.
 * Particles spawn behind the player and fade out smoothly.
 */
void ParticleEmitterComponent::ConfigureDashTrail() {
    SetEmissionRate(25.0f);      // Lower rate for individual bubbles
    SetMaxParticles(100);        // Moderate pool for visible bubbles
    SetLifetime(1.2f, 0.3f);     // Bubbles rise for a bit, with variation
    SetSpawnRadius(15.0f);       // Wider spawn area for kick disturbance
    SetSpawnOffset(vec2(0.0f, -80.0f));  // Spawn behind/below player
    
    // Bubbles rise and drift slightly
    SetVelocity(vec2(0, 1), 50.0f, 20.0f);  // Upward velocity with variation
    SetAngleSpread(30.0f);       // Some horizontal spread
    
    SetStartColor(vec4(0.8f, 0.9f, 1.0f, 0.7f));  // Light blue-white bubbles
    SetEndColor(vec4(0.9f, 0.95f, 1.0f, 0.0f));   // Fade to transparent at surface
    SetStartSize(25.0f);         // Start smaller for individual bubbles
    SetEndSize(50.0f);           // Grow as they rise (bubbles expand)
    
    SetGravity(vec2(0, -30.0f)); // Slight buoyancy (negative gravity)
    SetDrag(0.5f);               // Water resistance
    SetFadeOut(0.3f);            // Gentle fade as they dissipate
    SetFadeIn(0.1f);             // Quick fade in
    
    // Optional: add rotation for realism
    SetStartRotation(0.0f, 360.0f);  // Random rotation
    SetAngularVelocity(45.0f, 30.0f); // Gentle spin
    
    SetLayer(-2);                // Render behind player
}

/**
 * @brief Configures the emitter for jump dust effect
 *
 * Creates a burst of dust particles when player jumps from ground.
 * Particles shoot upward and fall with gravity.
 */
void ParticleEmitterComponent::ConfigureJumpDust() {
	SetEmissionRate(0.0f);       // Use Burst() instead of continuous emission
	SetMaxParticles(20);         // Small burst
	SetLifetime(0.5f, 0.1f);     // Short lifetime

	SetSpawnBox(vec2(15.0f, 2.0f));  // Wide, flat spawn area (ground spread)
	SetVelocity(vec2(0, 1), 40.0f, 20.0f);  // Upward with variation
	SetAngleSpread(60.0f);       // Wide cone

	SetStartColor(vec4(0.8f, 0.8f, 0.7f, 0.8f));  // Dusty brown/gray
	SetEndColor(vec4(0.6f, 0.6f, 0.5f, 0.0f));

	SetStartSize(8.0f);          // Start small
	SetEndSize(15.0f);           // Expand over time (puff of dust)

	SetGravity(vec2(0, -100.0f)); // Strong downward gravity
	SetDrag(0.5f);                // Moderate air resistance

	SetStartRotation(0.0f, 360.0f);          // Random initial rotation
	SetAngularVelocity(90.0f, 180.0f);       // Slow to moderate spinning

	SetLayer(1);  // Render above ground but below player
}

/**
 * @brief Configures the emitter for wall slide particles
 *
 * Creates sparks that fly out from the wall while player slides down.
 * Particles shoot away from wall and fall with gravity.
 */
void ParticleEmitterComponent::ConfigureWallSlideParticles() {
	SetEmissionRate(50.0f);      // Continuous emission during wall slide
	SetMaxParticles(100);        // Moderate pool size
	SetLifetime(0.4f, 0.1f);     // Short lifetime

	SetSpawnBox(vec2(2.0f, 10.0f));  // Thin vertical spawn area (along wall)
	SetVelocity(vec2(1, 0), 80.0f, 30.0f);  // Shoot away from wall (right direction)
	SetAngleSpread(45.0f);       // Moderate spread

	SetStartColor(vec4(1.0f, 0.8f, 0.3f, 1.0f));  // Orange sparks
	SetEndColor(vec4(1.0f, 0.3f, 0.1f, 0.0f));    // Fade to red

	SetStartSize(6.0f);          // Small spark size
	SetEndSize(2.0f);            // Shrink

	SetGravity(vec2(0, -150.0f)); // Strong downward gravity
	SetDrag(0.3f);                // Light air resistance

	SetStartRotation(0.0f, 360.0f);          // Random initial rotation
	SetAngularVelocity(360.0f, 720.0f);      // Fast spinning (sparks)

	SetLayer(0);  // Default layer
}

/**
 * @brief Configures the emitter for speed lines effect
 *
 * Creates horizontal streaks that give sense of high speed movement.
 * Particles move quickly in opposite direction of movement.
 */
void ParticleEmitterComponent::ConfigureSpeedLines() {
	SetEmissionRate(40.0f);      // Continuous emission
	SetMaxParticles(80);         // Moderate pool
	SetLifetime(0.2f, 0.05f);    // Very short lifetime (quick streaks)

	SetSpawnBox(vec2(100.0f, 50.0f));  // Wide spawn area (covers screen)
	SetVelocity(vec2(-1, 0), 300.0f, 50.0f);  // Fast horizontal (left direction)

	SetStartColor(vec4(1.0f, 1.0f, 1.0f, 0.5f));  // Semi-transparent white
	SetEndColor(vec4(1.0f, 1.0f, 1.0f, 0.0f));    // Fade to transparent

	SetStartSize(30.0f);         // Long lines (horizontal)
	SetEndSize(20.0f);           // Slightly shorter

	SetGravity(vec2(0, 0));      // No gravity (straight lines)
	SetDrag(0.0f);               // No drag (constant speed)

	SetStartRotation(0.0f);      // Horizontal lines (0° rotation)

	SetLayer(0);  // Background layer
}

/**
 * @brief Configures the emitter for death explosion effect
 *
 * Creates a burst of particles in all directions for death/explosion.
 * Particles fly out and fall with gravity.
 */
void ParticleEmitterComponent::ConfigureDeathExplosion() {
	SetEmissionRate(0.0f);       // Use Burst() for one-time explosion
	SetMaxParticles(50);         // Large burst
	SetLifetime(1.0f, 0.3f);     // Medium lifetime

	SetSpawnRadius(5.0f);        // Small central spawn area
	SetVelocity(vec2(0, 1), 150.0f, 80.0f);  // Initial upward velocity
	SetAngleSpread(360.0f);      // Full circle (explode in all directions)

	SetStartColor(vec4(1.0f, 0.3f, 0.2f, 1.0f));  // Bright red
	SetEndColor(vec4(0.8f, 0.1f, 0.0f, 0.0f));    // Dark red fade

	SetStartSize(15.0f);         // Start medium size
	SetEndSize(5.0f);            // Shrink over time

	SetGravity(vec2(0, -200.0f)); // Strong downward gravity
	SetDrag(0.4f);                // Moderate drag

	SetStartRotation(0.0f, 360.0f);          // Random initial rotation
	SetAngularVelocity(180.0f, 360.0f);      // Moderate spinning

	SetFadeOut(0.5f);            // Long fade out for smooth disappearance
	SetLayer(2);                 // Render on top of everything
}