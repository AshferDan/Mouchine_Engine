#pragma once
#include "CameraComponent.h"
#include "gameObject.h"

class CameraManager {
public:
	static CameraManager& Instance();

	void SetActiveCamera(GameObject* cameraObject);
	CameraComponent* GetActiveCamera();
	GameObject* GetActiveCameraObject() { return activeCameraObject; }

	mat4 GetViewProjectMatrix(float screenWidth, float screenHeight);
	vec2 GetCameraPosition();

	bool HasActiveCamera() const;

private:
	CameraManager() : activeCameraObject(nullptr) {};
	~CameraManager() = default;

	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

	GameObject* activeCameraObject;
};