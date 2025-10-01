#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "System.h"
#include "GraphicsSystem.h"

class DebugSystem : public ISystem {

public:
	DebugSystem() {};
	~DebugSystem() {};

	void Initialize() override;
	void Update(float dt) override;
	std::string GetName() override;

	void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color);
	void DrawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);
	void DrawCircle(const glm::vec2& center, float radius, const glm::vec4& color, int segments = 36);
	void DrawPoint(const glm::vec2& ppos, glm::vec4 color);

	void Clear();

	struct DebugShape {
		enum Type { LINE, RECT, CIRCLE, POINT } type;
		glm::vec2 p1, p2;
		float radius;
		glm::vec4 color;
		int segments;
	};
private:
	std::vector<DebugShape> shapes;
public:

	const std::vector<DebugShape>& GetShapes() const { return shapes; }
};