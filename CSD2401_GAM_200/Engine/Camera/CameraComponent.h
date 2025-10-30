#pragma once
#include "Component.h"
#include "MathLib.h"
#include "gameObject.h"

class CameraComponent : public IComponent {
public:
	enum class CameraMode {
		FOLLOW_TARGET,
		FREE_MOVE,
		FIXED
	};

	CameraComponent(GameObject* owner);
	virtual ~CameraComponent() = default;

	virtual void Update(float) override;
	virtual std::string GetName() const override { return "Camera Component"; }
	virtual std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override;

	void SetPosition(float x, float y);
	void SetPosition(const vec2& pos);
	vec2 GetPosition() const { return position; }

	void SetFollowTarget(GameObject* target);
	void SetFollowOffset(const vec2& offset);

	void SetSmoothing(CameraMode m);
	bool IsSmoothingEnabled() const { return enableSmoothing; }

	void SetBounds(float minX, float minY, float maxX, float maxY);
	void EnableBounds(bool enable);
	bool AreBoundsEnabled() const { return useBounds; }

	mat4 GetViewMatrix() const;
	mat4 GetProjectionMatrix(float screenWidth, float screenHeight) const;
	mat4 GetViewProjectionMatrix(float screenWidth, float screenHeight) const;


private:
	vec2 position;
	float zoom;
	CameraMode mode;

	//Follow target settings
	GameObject* followTarget;
	vec2 followOffset;

	//Smoothing
	bool enableSmoothing;
	float smoothSpeed;

	//Bounds
	bool useBounds;
	vec2 boundsMin;
	vec2 boundsMax;

	//Helper
	void ApplyBounds();
};
