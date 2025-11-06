#include "transformSystem.h"

float dt = CoreEngine::GetDeltaTime();
float fps = CoreEngine::GetFPS();
TransformSystem::TransformSystem() {

}

TransformSystem::~TransformSystem() {

}
void TransformSystem::Update(float dt) {
	auto& factory = GameObjectFactory::Instance();
	const auto& gameObjects = factory.GetAllGameObjects();
	
	for (const auto& obj : gameObjects) {
		const auto& physics = obj->GetComponent<PhysicsComponent>();
		const auto& transform = obj->GetComponent<TransformComponent>();
	
		if (physics && transform && !physics->IsStatic()) {
			IntegrateVelocity(physics, transform, dt);
		}
	}
};

/**
* @brief Updates position based on velocity (logic moved from PhysicsSystem)
*/
void TransformSystem::IntegrateVelocity(PhysicsComponent* physics, TransformComponent* transform, float deltaTime) {
	transform->Move(physics->velocityX * deltaTime, physics->velocityY * deltaTime);
}
