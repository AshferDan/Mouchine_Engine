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