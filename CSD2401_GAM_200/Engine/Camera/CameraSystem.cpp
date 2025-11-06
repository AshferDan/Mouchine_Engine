#include "CameraSystem.h"
#include "factory.h"

void CameraSystem::Update(float dt) {
	auto& gameObjects = GameObjectFactory::Instance().GetAllGameObjects();

	for (auto& obj : gameObjects) {
		if (!obj->IsActive()) {
			continue;
		}

		auto* camera = obj->GetComponent<CameraComponent>();
		if (camera && camera->IsEnabled()) {
			camera->Update(dt);
		}
	}

}