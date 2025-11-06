#pragma once
#include "CameraComponent.h"
#include "gameObject.h"
#include "factory.h"

class CameraManager {
public:
	static CameraManager& Instance();

	void SetActiveCamera(GameObject* cameraObject);
	CameraComponent* GetActiveCamera();
	GameObject* GetActiveCameraObject() { return activeCameraObject; }

	mat4 GetViewProjectMatrix(float screenWidth, float screenHeight);
	vec2 GetCameraPosition();
	void ResetState();

	bool HasActiveCamera() const;
	void ClearActiveCamera();

private:
	CameraManager() : activeCameraObject(nullptr) {};
	~CameraManager() = default;

	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

	GameObject* activeCameraObject;
};