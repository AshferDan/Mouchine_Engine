/* Start Header ************************************************************************/
/*!
\file       ImGuiSystem.cpp
\author     Guan Shao Jun [25%]
\co-author	Muhammad Ashfer [66% - 140 lines]
			Hwee Min [9% - 25 lines]
\par        shaojun.g@digipen.edu,
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ImGuiSystem.h"
#include "Serialization.h"
#include <iostream>
#include <chrono>
#include <map>
#include <string>
#include <filesystem>
#include "core.h"

// Keep externs as ImGuiSystem::Update now correctly relies on these globals
// being populated by CoreEngine *before* ImGui's display pass.
extern std::map<std::string, double> g_systemTimings;
extern double g_totalLoopTime;


/**
 * @brief Struct to hold Data for GO Creation
 * @param name		Name of GO
 * @param shpae		Name of shape of GO
 * @param pos[2]		(x,y) of GOs
 * @param scale[2]	(Width, Height)
 * @param rot		Rotation of GO
 * @param color		Vec4 holding colour of GO
 */
struct TempGOData
{
	char name[128]{""};
	std::string shape;
	float pos[2]{0.f, 0.f};
	float scale[2]{50.f, 50.f};
	float rot{0.f}; // = transform->GetRotation();
	ImVec4 color{0.f, 0.f, 0.f, 1.f};
};

// 2 Temp GO Structs - for Game Object Creator & Prefab Loader
TempGOData tempGO{"Game Object"};
TempGOData tempPrefab;

std::vector<std::string> prefabList;
char rename_str[128]{ "" };

// Flags for IMGUI (current selection in drop down for GO Creator, and showing of some screens)
static int current_shape = 0;
static int current_prefab = 0;
static int prefab_renameIndex = -1;
static bool prefab_renameFocus = false;
std::string prefab_oldname{};
bool ShowCreateGOMenu;
bool ShowLoadPrefabMenu;


/**
 * @brief Ctor - Inits window variable
 */
ImGuiSystem::ImGuiSystem(GLFWwindow *window) : m_window(window) {};

/**
 * @brief Dtor - Calls Funct to terminate properly
 */
ImGuiSystem::~ImGuiSystem()
{
	Shutdown();
}

/**
 * @brief Initialises ImGUI
 */
void ImGuiSystem::Initialize()
{
	IMGUI_CHECKVERSION();	// Verify ImGui version
	ImGui::CreateContext(); // Create ImGui context
	ImGuiIO &io = ImGui::GetIO();
	(void)io;									  // IO object (unused here)
	ImGui::StyleColorsDark();					  // Set dark theme
	ImGui_ImplGlfw_InitForOpenGL(m_window, true); // Initialize GLFW backend
	ImGui_ImplOpenGL3_Init("#version 450");		  // Initialize OpenGL backend
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;	// Enable docking

	ReloadPrefabsList();

	ShowCreateGOMenu = false; // Hide create menu initially
}

/**
 * @brief Update - Runs each frame to update IMGui screens
 * @param dt		  Delta Time
 */
void ImGuiSystem::Update(float dt)
{
	if (InputManager::KeyStateImgui) {
		//ImGui_ImplOpenGL3_NewFrame(); // Start new OpenGL frame
		//ImGui_ImplGlfw_NewFrame();	  // Start new GLFW frame
		//ImGui::NewFrame();			  // Start ImGui frame

		// Full window docking
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Window Dock", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		ImGuiID dockspace_id = ImGui::GetID("DockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

		if (CORE->GetGraphicsSystem()->IsEditorMode()) {
			ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);

			CORE->GetGraphicsSystem()->RenderGameViewportImGui();
		}

		ImGui::End();

		// Begin main GameObject Inspector window
		ImGui::Begin("GameObject Inspector",&InputManager::KeyStateImgui);

		// Toggle GameObject creation menu
		if (ImGui::Button("Create New Game Object"))
		{
			ShowCreateGOMenu = (!ShowCreateGOMenu) ? true : false;
		}

		// Toggle prefab loading menu
		if (ImGui::Button("Load Prefabs"))
		{
			ShowLoadPrefabMenu = (!ShowLoadPrefabMenu) ? true : false;
		}

		// Iterate through all game objects
		for (auto& obj : GameObjectFactory::Instance().GetAllGameObjects())
		{
			if (!obj->IsActive())
			{
				continue; // Skip inactive objects
			}

			// Display tree node for each object
			if (ImGui::TreeNode(obj->GetName().c_str()))
			{
				// Access transform component for editing
				if (auto* transform = obj->GetComponent<TransformComponent>())
				{
					float pos[2];
					transform->GetPosition(pos[0], pos[1]);
					// Drag to edit position
					if (ImGui::DragFloat2("Position", pos, 0.1f))
					{
						transform->SetPosition(pos[0], pos[1]);
					}

					float scale[2];
					transform->GetScale(scale[0], scale[1]);
					// Drag to edit scale
					if (ImGui::DragFloat2("Scale", scale, 0.1f))
					{
						transform->SetScale(scale[0], scale[1]);
					}

					float rot = transform->GetRotation();
					// Drag to edit rotation
					if (ImGui::DragFloat("Rotation", &rot, 0.1f))
					{
						transform->SetRotation(rot);
					}
				}

				// Altered for M1 demonstration of 2.5k GOs!
				if (obj->GetName() != "Hero" && obj->GetName() != "Evil Button")
				{
					if (ImGui::Button("Clone"))
					{
						GameObjectFactory& factory = GameObjectFactory::Instance();
						GameObject* clonedObj = factory.CloneGameObject(obj->GetID());

						// Save cloned object data
						Serialize_Writer writer("../Logs/GameObjectData.txt");
						if (writer.IsStreamGood())
						{
							writer.WriteGameObjectData(*clonedObj, 0);
							std::cout << "Cloned object data saved!\n";
						}

						std::cout << "button clicked";
					}

					// Save GO Data - Serialisation!
					if (ImGui::Button("Save Data"))
					{
						Serialize_Writer writer("../Logs/GameObjectData.txt"); // Create Writer class object with specified path
						if (writer.IsStreamGood())
						{
							writer.WriteGameObjectData(*obj, 0); // If stream is valid, write objects data to file path
							std::cout << "GameObject data saved!\n";
						}
					}

					// Create Prefab with GO Data
					if (ImGui::Button("Create Prefab"))
					{
						PrefabTools::CreatePrefab(*obj);
						ReloadPrefabsList();
					}
				}


				ImGui::TreePop();
			}
		}
				// Evil button that spawns 2.5k Circle GOs
					if (ImGui::Button("Spawn 2500 Entities"))
					{
						DemoM1_SpawnGameObjects();
						std::cout << "Spawned 2500 Circle Entities!\n";
					}

		ImGui::End();

		/**
		 * @brief Game Object Creator Menu IMGui Window
		 */
		if (ShowCreateGOMenu) // if flag to enable is true
		{
			ImGui::Begin("Game Object Creator", &ShowCreateGOMenu); // Mark beginning of window
			{
				// Game Object Text Field
				ImGui::Text("Name");
				if (ImGui::InputText("#", tempGO.name, sizeof(tempGO.name)))
				{
				} // Text input field for GO Name

				// Very ugly sorry, future impl can include a check for empty GOs
				const char* ShapeTypes[4] = { "Square", "Circle", "Line", "Point" }; // Type of shapes
				ImGui::Text("Shape Type");
				if (ImGui::BeginCombo("##", ShapeTypes[current_shape]))
				{ // Drop down menu for each shape
					for (int i = 0; i < 4; i++)
					{
						bool is_selected = (current_shape == i);
						if (ImGui::Selectable(ShapeTypes[i], is_selected))
							current_shape = i;

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo(); // End drop down menu
				}

				// Shape Colour Picker
				ImGui::Text("Colour");
				if (ImGui::ColorEdit3(" ", (float*)&tempGO.color))
				{
				} // Colour picker to later store in Vec4

				// Game Object Position
				ImGui::Text("Position");
				if (ImGui::DragFloat2("##", tempGO.pos, 0.1f))
				{
				} // Slider for GO position

				// Game Object Scale
				ImGui::Text("Scale");
				if (ImGui::DragFloat2("###", tempGO.scale, 0.1f))
				{
				} // Slider for GO scale

				// Game Object Rotation
				ImGui::Text("Rotation");
				if (ImGui::DragFloat("####", &tempGO.rot, 0.1f))
				{
				} // Slider for GO rot

				// Generate Game Object
				ImGui::Text("  ");
				if (ImGui::Button("Generate")) // Button to create new GO
				{
					auto& factory = GameObjectFactory::Instance(); // Factory

					// very ugly colour code bc that uses a different vec 4 AAAA
					GameObject* TempGO = factory.CreateShape(tempGO.name, RenderComponent::ShapeType{ current_shape }, 20.f, 0.f, 5.f, 5.f,
						vec4(tempGO.color.x, tempGO.color.y, tempGO.color.z, 1.f), CollisionLayer::Default);

					// Add transform component to assign transform values
					TempGO->AddComponent<TransformComponent>(0, 0);
					auto* transform = TempGO->GetComponent<TransformComponent>();

					transform->SetPosition(tempGO.pos[0], tempGO.pos[1]);
					transform->SetScale(tempGO.scale[0], tempGO.scale[1]);
					transform->SetRotation(tempGO.rot);
				}
			}
			ImGui::End(); // End of GO Creation
		}

		/**
		 * @brief Prefab Loader Menu IMGui Window
		 */
		if (ShowLoadPrefabMenu) // If flag to show Prefab Search Menu is true
		{
			ImGui::Begin("Prefab Loader", &ShowLoadPrefabMenu);

			ImGui::Text("Prefab Menu");
			ImGui::BeginGroup();
			if (ImGui::BeginListBox("##", ImVec2(200, 150)))					// Draw selectable box for which prefabs are found
			{
				if (prefabList.empty())
				{
					ImGui::Text("No saved Prefabs found.");
				}
				else
				{
					// Draw list of prefabs
					for (int i = 0; i < prefabList.size(); ++i)					// Loop for every prefab in list
					{
						if (prefab_renameIndex != i)
						{
							bool is_selected = (current_prefab == i);
							if (ImGui::Selectable(prefabList[i].c_str(), &is_selected))
							{
								current_prefab = i;									// Selected item in list
							}
							else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
							{
								current_prefab = i; // optional: select on right-click too
								ImGui::OpenPopup("Prefab_RightClickEvent");
							}
							if (i != prefabList.size() - 1) { ImGui::Separator(); }
						}
						else
						{
							ImGui::PushID(i);
							if (prefab_renameFocus)
							{
								ImGui::SetKeyboardFocusHere();
								prefab_renameFocus = false;
							}
							if (ImGui::InputText("##", rename_str, IM_ARRAYSIZE(rename_str), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
							{
								prefabList[i] = std::string(rename_str);
								//ImGui::SetKeyboardFocusHere(-1);
							}
							//if (ImGui::IsKeyPressed(ImGuiKey_Escape))					//supposed to be escape but it closes the whole game LMAO
							if (ImGui::IsKeyPressed(ImGuiKey_Tab) || ImGui::IsKeyPressed(ImGuiKey_Enter))
							{
								prefab_renameIndex = -1;
								PrefabTools::RenamePrefab(prefab_oldname, rename_str);
							}

							ImGui::PopID();
						}
						
					}
					/*char test[128]{ "" };
					if (ImGui::InputText("Entity Name", test, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
					{
						
					}*/
				}

				if (ImGui::BeginPopup("Prefab_RightClickEvent"))					// If an item in the prefab lists gets right clicked
				{
					if (ImGui::MenuItem("Rename")) 
					{
						prefab_renameIndex = current_prefab;
						strncpy(rename_str, prefabList[current_prefab].c_str(), sizeof(rename_str));
						prefab_renameFocus = true;
						prefab_oldname = prefabList[current_prefab];
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Delete")) 
					{
						// TODO: DELETE
						PrefabTools::RemovePrefab(prefabList[current_prefab]);
						ReloadPrefabsList();
					}
					ImGui::EndPopup();
				}

				ImGui::EndListBox();
			}
			ImGui::EndGroup();

			ImGui::SameLine();													// This so its on the right side of the selection box
			// draw preview
			ImGui::BeginGroup();
			ImVec2 boxSize = ImVec2(256, 256);
			ImGui::Text("Preview Render:");
			ImGui::Dummy(boxSize);												// Reserves space for preview

			ImDrawList* drawList = ImGui::GetWindowDrawList();					// Imgui rendering for box with white outline
			drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 255));
			ImGui::EndGroup();

			if (ImGui::Button("Spawn"))
			{
				std::string filePath{ "../Assets/Prefabs/" + prefabList[current_prefab] + ".txt" }; // Checking for specific file path & file type
				Serialize_Reader reader(filePath);
				if (!reader.IsStreamGood())
				{ // Check if is existing file
					std::cout << "file stream bad!\n";
				}
				else // If matching file name found
				{
					std::string PrefabData = PrefabTools::ReadPrefabData(filePath.c_str()); // Get prefab data and store into stringstream to plug into temp prefab struct
					std::stringstream ss(PrefabData);
					std::cout << ss.str();

					ss >> tempPrefab.name >> tempPrefab.shape >> tempPrefab.pos[0] >> tempPrefab.pos[1] >> tempPrefab.scale[0] >> tempPrefab.scale[1] >>
						tempPrefab.color.x >> tempPrefab.color.y >> tempPrefab.color.z >> tempPrefab.color.w;

					RenderComponent::ShapeType shapeEnum = RenderComponentTools::ShapeEnumToString(tempPrefab.shape);

					auto& factory = GameObjectFactory::Instance(); // Factory

					// very ugly colour code bc that uses a different vec 4 AAAA
					GameObject* TempGO = factory.CreateShape(static_cast<std::string>(tempPrefab.name), RenderComponent::ShapeType{ shapeEnum }, tempPrefab.pos[0], tempPrefab.pos[1],
						tempPrefab.scale[0], tempPrefab.scale[1], vec4(tempPrefab.color.x, tempPrefab.color.y, tempPrefab.color.z, tempPrefab.color.w), CollisionLayer::Default);

					// Add transform component to assign transform values
					TempGO->AddComponent<TransformComponent>(0, 0);
					auto* transform = TempGO->GetComponent<TransformComponent>();

					transform->SetPosition(tempPrefab.pos[0], tempPrefab.pos[1]);
					transform->SetScale(tempPrefab.scale[0], tempPrefab.scale[1]);
					transform->SetRotation(tempPrefab.rot);
				}
			}

			ImGui::End(); // End of Prefab Search Menu
		}

		/**
		 * @brief Performance Profiler IMGui Window
		 */
		ImGui::Begin("=== Performance Profiler ===");

		static std::map<std::string, float> smoothedMs;
		static std::map<std::string, float> smoothedPercent;


		const float lerpSpeed = static_cast<float>(0.05); // smaller = smoother

		std::map<std::string, double>::iterator it;
		for (it = g_systemTimings.begin(); it != g_systemTimings.end(); ++it)
		{
			const std::string& name = it->first;
			double ms = it->second;
			double percent = (g_totalLoopTime > 0.0) ? (ms / g_totalLoopTime) * 100.0 : 0.0;

			// Initalize if first time
			if (smoothedMs.find(name) == smoothedMs.end())
			{
				smoothedMs[name] = (float)ms;
				smoothedPercent[name] = (float)percent;
			}

			// Lerp towards raw ms and percent
			smoothedMs[name] += (float)(ms - smoothedMs[name]) * lerpSpeed;
			smoothedPercent[name] += (float)(percent - smoothedPercent[name]) * lerpSpeed;

			// Add a small ImGui::Text displaying the total frame time first for context
			if (it == g_systemTimings.begin())
			{
				ImGui::Text("Total System Time: %.3f ms", g_totalLoopTime);
				ImGui::Text("Total Frame Per Seconds: %f ms", CoreEngine::GetFPS());
				ImGui::Text("Total Delta Time: %f ms", CoreEngine::GetDeltaTime());
				ImGui::Separator();
			}

			ImGui::Text("%-20s %.4f ms (%.1f%%)", name.c_str(), smoothedMs[name], smoothedPercent[name]);
			ImGui::ProgressBar((float)(smoothedPercent[name] / 100.0f), ImVec2(0.0f, 0.0f));
		}
		ImGui::End();

		// Render ImGui frame
		//ImGui::Render();										// Finish ImGui frame
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render draw data via OpenGL3 backend
		//glfwSwapBuffers(m_window);								// Swap GLFW window buffers
		//glfwPollEvents();// Poll input events
	}
}

/**
 * @brief Shutsdown IMGui properly
 */
void ImGuiSystem::Shutdown()
{
	ImGui_ImplOpenGL3_Shutdown(); // Shutdown OpenGL3 backend
	ImGui_ImplGlfw_Shutdown();	  // Shutdown GLFW backend
	ImGui::DestroyContext();	  // Destroy ImGui context
}

/**
 * @brief Get System Name
 */
std::string ImGuiSystem::GetName()
{
	return "ImGui_System"; // Return descriptive system name
}

void ReloadPrefabsList()
{
	prefabList = PrefabTools::LoadPrefabList();
}
