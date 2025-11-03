#pragma once
#include "System.h"
#include "CameraComponent.h"

class CameraSystem : public ISystem {
public:
	CameraSystem() = default;
	virtual ~CameraSystem() = default;

	virtual void Initialize() override {}
	virtual void Update(float dt) override;
	virtual std::string GetName() override { return "Camera System"; }
private:

};