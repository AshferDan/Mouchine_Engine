#include "ImGuiSystem.h"
#include <iostream>

ImGuiSystem::ImGuiSystem(GLFWwindow* window) : m_window(window) {};

ImGuiSystem::~ImGuiSystem() {
	Shutdown();
}

void ImGuiSystem::Initialize() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 450");
}

void ImGuiSystem::Update(float dt) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("GameObject Inspector");

	for (auto& obj : GameObjectFactory::Instance().GetAllGameObjects()) {
		if (!obj->IsActive()) {
			continue;
		}

		if (ImGui::TreeNode(obj->GetName().c_str())) {
			if (auto* transform = obj->GetComponent<TransformComponent>()) {
				float pos[2];
				transform->GetPosition(pos[0], pos[1]);
				if (ImGui::DragFloat2("Position", pos, 0.1f)) {
					transform->SetPosition(pos[0], pos[1]);
				}

				float scale[2];
				transform->GetScale(scale[0], scale[1]);
				if (ImGui::DragFloat2("Scale", scale, 0.1f)) {
					transform->SetScale(scale[0], scale[1]);
				}

				float rot = transform->GetRotation();
				if (ImGui::DragFloat("Rotation", &rot, 0.1f)) {
					transform->SetRotation(rot);
				}
			}
			ImGui::TreePop();
		}
	}

	ImGui::End();

	//Render ImGui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(m_window);
	glfwPollEvents();

}

//void ImGuiSystem::Destroy() {
//	Shutdown();
//}

void ImGuiSystem::Shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

std::string ImGuiSystem::GetName() {
	return "ImGui System";
}