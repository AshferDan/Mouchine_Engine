#include "ImGuiSystem.h"
#include "Serialization.h"
#include <iostream>
#include <chrono>
#include <map>
#include <string>

// Keep externs as ImGuiSystem::Update now correctly relies on these globals 
// being populated by CoreEngine *before* ImGui's display pass.
extern std::map<std::string, double> g_systemTimings;
extern double g_totalLoopTime;


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

	// REMOVED: Internal timing start, as this is now handled by CoreEngine::GameLoop
	// auto start = std::chrono::high_resolution_clock::now();


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

			if (obj->GetName() != "Hero") {
				if (ImGui::Button("Clone")) {
					GameObjectFactory& factory = GameObjectFactory::Instance();
					GameObject* clonedObj = factory.CloneGameObject(obj->GetID());

					// Save cloned object data
					Serialize_Writer writer("../Logs/GameObjectData.txt");
					if (writer.IsStreamGood()) {
						writer.WriteGameObjectData(*clonedObj, 0);
						std::cout << "Cloned object data saved!\n";
					}

					std::cout << "button clicked";
				}

				if (ImGui::Button("Save Data")) {
					Serialize_Writer writer("../Logs/GameObjectData.txt");
					if (writer.IsStreamGood()) {
						writer.WriteGameObjectData(*obj, 0);
						std::cout << "GameObject data saved!\n";
					}
				}
			}

			ImGui::TreePop();
		}
	}
	ImGui::Separator();
	ImGui::Text("=== Performance Profiler ===");

	std::map<std::string, double>::iterator it;
	for (it = g_systemTimings.begin(); it != g_systemTimings.end(); ++it) {
		const std::string& name = it->first;
		double ms = it->second;
		double percent = (g_totalLoopTime > 0.0) ? (ms / g_totalLoopTime) * 100.0 : 0.0;

		// Add a small ImGui::Text displaying the total frame time first for context
		if (it == g_systemTimings.begin()) {
			ImGui::Text("Total System Time: %.3f ms (Excludes GLFW swap/poll)", g_totalLoopTime);
			ImGui::Separator();
		}

		ImGui::Text("%-20s %.4f ms (%.1f%%)", name.c_str(), ms, percent);
		ImGui::ProgressBar((float)(percent / 100.0f), ImVec2(0.0f, 0.0f));
	}

	ImGui::End();

	//Render ImGui
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

void ImGuiSystem::Shutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

std::string ImGuiSystem::GetName() {
	return "ImGui_System";
}
