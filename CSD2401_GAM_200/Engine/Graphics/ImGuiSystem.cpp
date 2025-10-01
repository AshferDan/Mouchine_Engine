#include "ImGuiSystem.h"
#include "Serialization.h"
#include <iostream>

bool ShowCreateGOMenu;

struct TempGOData {
	float pos[2]{0.f, 0.f};
	float scale[2]{50.f, 50.f};
	float rot{0.f};// = transform->GetRotation();
	ImVec4 color{0.f,0.f,0.f,1.f};
};
TempGOData tempGO;
static int current_shape = 0;

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

	ShowCreateGOMenu = false;
}

void ImGuiSystem::Update(float dt) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("GameObject Inspector");

	if (ImGui::Button("Create New Game Object")) {
		ShowCreateGOMenu = (!ShowCreateGOMenu) ? true : false;
	}

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

			// Altered for M1 demonstration of 2.5k GOs!
			if (obj->GetName() != "Hero" && obj->GetName() != "Evil Button") {
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

			if (obj->GetName() == "Evil Button") {
				if (ImGui::Button("Spawn 2500 Entities")) {
					DemoM1_SpawnGameObjects();
					std::cout << "Spawned 2500 Circle Entities!\n";
				}
			}

			ImGui::TreePop();
		}
	}

	ImGui::End();


	// Game Object Creator
	if (ShowCreateGOMenu) 
	{
		ImGui::Begin("Game Object Creator", &ShowCreateGOMenu);
		{
			/*auto& factory = GameObjectFactory::Instance();
			GameObject* TempGO = factory.CreateGameObject("Temporary Game Object");
			TempGO->AddComponent<TransformComponent>(0, 0);
			auto* transform = TempGO->GetComponent<TransformComponent>();*/

			// Very ugly sorry, future impl can include a check for empty GOs
			const char* ShapeTypes[4] = { "Square", "Circle", "Line", "Point" };
			ImGui::Text("Shape Type");
			if (ImGui::BeginCombo("##", ShapeTypes[current_shape])) {
				for (int i = 0; i < 4; i++)
				{
					bool is_selected = (current_shape == i);
					if (ImGui::Selectable(ShapeTypes[i], is_selected))
						current_shape = i;

					// Set the initial focus when opening the combo (scroll to current item)
					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::Text("Colour");
			if (ImGui::ColorEdit3(" ", (float*) & tempGO.color)) {
				//transform->SetRotation(rot);
			}
			
			//transform->GetPosition(pos[0], pos[1]);
			ImGui::Text("Position");
			if (ImGui::DragFloat2("##", tempGO.pos, 0.1f)) {
				//transform->SetPosition(pos[0], pos[1]);
			}
			
			//transform->GetScale(scale[0], scale[1]);
			ImGui::Text("Scale");			
			if (ImGui::DragFloat2("###", tempGO.scale, 0.1f)) {			// Need to include ## for empty text fields
				//transform->SetScale(scale[0], scale[1]);
			}

			ImGui::Text("Rotation");
			if (ImGui::DragFloat("####", &tempGO.rot, 0.1f)) {
				//transform->SetRotation(rot);
			}

			ImGui::Text("  ");
			if (ImGui::Button("Generate"))
			{
				//std::cout << tempGO.color.x << " " << tempGO.color.y << " " << tempGO.color.z << " " << tempGO.color.w << "\n";
				auto& factory = GameObjectFactory::Instance();
				std::string selectedShape{ShapeTypes[current_shape]};

				// very ugly colour code bc that uses a different vec 4 AAAA
				GameObject* TempGO = factory.CreateShape("New " + selectedShape, RenderComponent::ShapeType{current_shape}, 20.f, 0.f, 5.f, 5.f, 
					glm::vec4(tempGO.color.x, tempGO.color.y, tempGO.color.z, 1.f));

				TempGO->AddComponent<TransformComponent>(0, 0);
				auto* transform = TempGO->GetComponent<TransformComponent>();

				transform->SetPosition(tempGO.pos[0], tempGO.pos[1]);
				transform->SetScale(tempGO.scale[0], tempGO.scale[1]);
				transform->SetRotation(tempGO.rot);
			}
		}
		ImGui::End();
	}


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
	return "ImGui System";
}