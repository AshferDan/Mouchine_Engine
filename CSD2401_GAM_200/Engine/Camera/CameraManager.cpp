#include "CameraManager.h"
#include <iostream>

CameraManager& CameraManager::Instance() {
	static CameraManager instance;
	return instance;
}

void CameraManager::SetActiveCamera(GameObject* cameraObject) {
	if (!cameraObject) {
		std::cerr << "Warning: Trying to set Null camera object as active" << std::endl;
		activeCameraObject = nullptr;
		return;
	}

	auto* camComponent = cameraObject->GetComponent<CameraComponent>();
	if (!camComponent) {
		std::cerr << "Warning: GameObject does not have a CameraComponent" << std::endl;
		activeCameraObject = nullptr;
		return;
	}

	activeCameraObject = cameraObject;
	std::cout << "Active Camera set to: " << cameraObject->GetName() << std::endl;

}

CameraComponent* CameraManager::GetActiveCamera() {
	if (!activeCameraObject || !activeCameraObject->IsActive()) {
		return nullptr;
	}

	return activeCameraObject->GetComponent<CameraComponent>();
};

mat4 CameraManager::GetViewProjectMatrix(float screenWidth, float screenHeight) {
	auto* camera = GetActiveCamera();

	if (camera && camera->IsEnabled()) {
		return camera->GetViewProjectionMatrix(screenWidth, screenHeight);
	}

	//Fallback
	return mat4::ortho(0.f, screenWidth, 0.f, screenHeight);
}

vec2 CameraManager::GetCameraPosition() {
	auto* camera = GetActiveCamera();
	if (camera) {
		return camera->GetPosition();
	}

	//Fallback
	return vec2(0.f, 0.f);
}

bool CameraManager::HasActiveCamera() const {
	return activeCameraObject != nullptr && activeCameraObject->IsActive();
}

void CameraManager::ClearActiveCamera() {
	activeCameraObject = nullptr;
	std::cout << "[CameraManager] Active camera cleared\n";
}
void CameraManager::ResetState() {
    auto& factory = GameObjectFactory::Instance();
    ClearActiveCamera();

    // 1. Try to find an existing camera from the loaded scene
    GameObject* activeCamObj = nullptr;
    for (auto& obj : factory.GetAllGameObjects()) {
        if (obj->GetComponent<CameraComponent>()) {
            activeCamObj = obj.get();
            if (obj->GetName() == "MainCamera") break;
        }
    }

    // 2. If NO camera found, CREATE one (Fallback)
    if (!activeCamObj) {
        std::cout << "[CameraManager] No camera found. Creating default 'MainCamera'.\n";
        activeCamObj = factory.CreateGameObject("MainCamera");
        activeCamObj->AddComponent<CameraComponent>();
        // Optional: Add Transform if your system requires it for all objects
        activeCamObj->AddComponent<TransformComponent>();
    }

    // 3. Set as active and configure it
    SetActiveCamera(activeCamObj);
    CameraComponent* camComp = GetActiveCamera();

    // 4. Find Hero and set to follow
    GameObject* hero = factory.FindGameObject("Hero");
    if (hero && camComp) {
        std::cout << "[CameraManager] Setting camera to follow 'Hero'.\n";
        camComp->SetMode(CameraComponent::CameraMode::FOLLOW_TARGET);
        camComp->SetFollowTarget(hero);
        camComp->SetFollowOffset({ 0.f, 0.f });
        camComp->SetSmoothing(true, 5.f); 

       
        if (auto* heroTx = hero->GetComponent<TransformComponent>()) {
            camComp->SetPosition({ heroTx->posX, heroTx->posY });
        }
    }
    else if (camComp) {
        
        camComp->SetMode(CameraComponent::CameraMode::FIXED);
        camComp->SetPosition(0.f, 0.f);
    }
}