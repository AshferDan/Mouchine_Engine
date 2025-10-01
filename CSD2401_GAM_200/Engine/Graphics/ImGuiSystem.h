#pragma once
#include "System.h"
#include "factory.h"
#include "transform.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

class GameObject;
class GameObjectFactory;
class Serialize_Writer;
class ImGuiSystem : public ISystem {
public:
	ImGuiSystem(GLFWwindow* window);
	~ImGuiSystem();

	void Initialize() override;
	void Update(float dt) override;
	//void Destroy();

	std::string GetName() override;

	//void SetWindow(GLFWwindow* window) { m_window = window; }
private:
	void Shutdown();

	GLFWwindow* m_window;
};