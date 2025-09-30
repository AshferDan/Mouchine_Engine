#include "DebugSystem.h"

void DebugSystem::Initialize() {
	//empty
}

void DebugSystem::Update(float dt) {
	//DOES NOT WORK
	//if (!graphics || !graphics->isDebugModeOn()) {
	//	return;
	//}
	//for (auto& shape : shapes) {
	//	switch (shape.type) {
	//	case DebugShape::LINE:
	//		graphics->DrawLine(shape.p1, shape.p2, shape.color, graphics->getVp());
	//		//std::cout << "test" << std::endl;
	//		break;
	//	case DebugShape::RECT:
	//		graphics->DrawRectOutline(shape.p1.x, shape.p1.y, shape.p2.x, shape.p2.y, shape.color, graphics->getVp());
	//		break;
	//	case DebugShape::CIRCLE:
	//		graphics->DrawCircleOutline(shape.p1.x, shape.p1.y, shape.radius, shape.segments, shape.color, graphics->getVp());
	//		break;
	//	case DebugShape::POINT:
	//		graphics->DrawPoint(shape.p1, shape.color, graphics->getVp());
	//		break;
	//	}
	//}
}

void DebugSystem::DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color) {
	shapes.push_back({ DebugShape::LINE, start, end, 0.0f, color, 0 });
}

void DebugSystem::DrawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) {
	shapes.push_back({ DebugShape::RECT, pos, size, 0.0f, color, 0 });
}

void DebugSystem::DrawCircle(const glm::vec2& center, float radius, const glm::vec4& color, int segments) {
	shapes.push_back({ DebugShape::CIRCLE, center, {}, radius, color, segments });
}

void DebugSystem::DrawPoint(const glm::vec2& ppos, glm::vec4 color) {
	shapes.push_back({ DebugShape::POINT, ppos, {}, 0.0f, color, 0 });
}

std::string DebugSystem::GetName() {
	return "Debug System";
}