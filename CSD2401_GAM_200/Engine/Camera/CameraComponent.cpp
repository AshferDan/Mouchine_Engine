#include "CameraComponent.h"
#include "transform.h"
#include <algorithm>

CameraComponent::CameraComponent(GameObject* owner) 
	: IComponent(owner), position(0.f, 0.f), zoom(0.f), mode(CameraMode::FIXED), followTarget(nullptr), followOffset(0.f, 0.f), enableSmoothing(false), smoothSpeed(5.f), useBounds(false), boundsMin(-10000.f, -10000.f), boundsMax(10000.f, 10000.f) {}

void CameraComponent::Update(float dt) {
	if (!IsEnabled()) {
		return;
	}

	if (mode == CameraMode::FOLLOW_TARGET && followTarget && followTarget->IsActive()) {
		auto* transform = followTarget->GetComponent<TransformComponent>();
		if (transform) {
			float targetX, targetY;
			transform->GetPosition(targetX, targetY);

			vec2 targetPos(targetX + followOffset.x, targetY + followOffset.y);

			if (enableSmoothing) {
				float t = 1.f - pow(0.5f, smoothSpeed * dt);
				position.x += (targetPos.x - position.x) * t;
				position.y += (targetPos.y = position.y) * t;
			}
			else {
				position = targetPos;
			}

		}
	}

	if (useBounds) {
		ApplyBounds();
	}
}

void CameraComponent::SetPosition(float x, float y) {
	position.x = x;
	position.y = y;
	
	if (useBounds) {
		ApplyBounds();
	}
}

void CameraComponent::SetPosition(const vec2& pos) {
	position = pos;

	if (useBounds) {
		ApplyBounds();
	}
}

void CameraComponent::Move(float dx, float dy) {
	position.x += dx;
	position.y += dy;

	if (useBounds) {
		ApplyBounds();
	}
}

void CameraComponent::SetZoom(float z) {
	zoom = std::max(0.1f, z);
}

void CameraComponent::SetFollowTarget(GameObject* target) {
	followTarget = target;
}

void CameraComponent::SetFollowOffset(const vec2& offset) {
	followOffset = offset;
}

void CameraComponent::SetMode(CameraMode m) {
	mode = m;
}

void CameraComponent::SetSmoothing(bool enable, float speed) {
	enableSmoothing = enable;
	smoothSpeed = speed;
}

void CameraComponent::SetBounds(float minX, float minY, float maxX, float maxY) {
	boundsMin.x = minX;
	boundsMin.y = minY;
	boundsMax.x = maxX;
	boundsMax.y = maxY;
}

void CameraComponent::EnableBounds(bool enable) {
	useBounds = enable;
}

void CameraComponent::ApplyBounds() {
	position.x = std::clamp(position.x, boundsMin.x, boundsMax.x);
	position.y = std::clamp(position.y, boundsMin.y, boundsMax.y);
}

mat4 CameraComponent::GetViewMatrix() const {
	return mat4::translate(vec3(-position.x, -position.y, 0.f));
}

mat4 CameraComponent::GetProjectionMatrix(float screenWidth, float screenHeight) const {
	float halfWidth = (screenWidth / 2.0f) / zoom;
	float halfHeight = (screenHeight / 2.0f) / zoom;

	return mat4::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.f, 1.f);
}

mat4 CameraComponent::GetViewProjectionMatrix(float screenWidth, float screenHeight) const {
	mat4 projection = GetProjectionMatrix(screenWidth, screenHeight);
	mat4 view = GetViewMatrix();
	return projection * view;
}

std::unique_ptr<IComponent> CameraComponent::Clone(GameObject* newOwner) const {
	auto cloned = std::make_unique<CameraComponent>(newOwner);

	cloned->position = this->position;
	cloned->zoom = this->zoom;
	cloned->mode = this->mode;
	cloned->followTarget = this->followTarget;
	cloned->followOffset = this->followOffset;
	cloned->enableSmoothing = this->enableSmoothing;
	cloned->smoothSpeed = this->smoothSpeed;
	cloned->useBounds = this->useBounds;
	cloned->boundsMin = this->boundsMin;
	cloned->SetEnabled(this->IsEnabled());

	return cloned;
}
