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
#include "PrefabManager.h"
#include <iostream>
#include <chrono>
#include <map>
#include <string>
#include <filesystem>
#include "core.h"
#include "RenderComponent.h"
#include <typeinfo>
#include <type_traits>
#include "LevelEditor.h"

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
    // std::string shape;
    int shapeIndex{0};
    float pos[2]{0.f, 0.f};
    float scale[2]{50.f, 50.f};
    float rot{0.f}; // = transform->GetRotation();
    ImVec4 color{0.f, 0.f, 0.f, 1.f};
};

// GO CREATION MENU VARIABLES
TempGOData tempGO{"Game Object"};
static int current_shape = 0;

// Flags for IMGUI (current selection in drop down for GO Creator, and showing of some screens)
bool ShowCreateGOMenu;
bool ShowLoadPrefabMenu;

std::vector<std::string> editorLayoutList;
char saveLayoutName[128]{""};

// PREFAB VARIABLES
std::vector<std::string> prefabList;
TempGOData tempPrefab;
static int current_prefab = 0;
static int prefab_renameIndex = -1;
static bool prefab_renameFocus = false;
char rename_str[128]{""};
std::string prefab_oldname{};

// DRAG & DROP VARIABLES
static bool greyOutGOSettings = false;
static bool openLayoutSavePopup = false;
static bool openLayoutLoadPopup = false;
static bool saveNamePopupClear = false;

// Vars for asset manager interface
float paddingSize;
float cellSize;
DirectoryNavigator AssetBrowserWindow;

GLuint icon_folder_ui;
GLuint icon_file_ui;
GLuint icon_plus_ui;
GLuint icon_prefab_ui;

// selected game object for editing
static EntityID selectedGameObjectID = 0;

// Helper
void ReloadPrefabsList()
{
    prefabList = Serialization::GetAllPrefabs();
}

/**
 * @brief Ctor - Inits window variable
 */
ImGuiSystem::ImGuiSystem(GLFWwindow *window) : m_window(window), factory(GameObjectFactory::Instance()), selectedGameObjectID(0) {}

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
    IMGUI_CHECKVERSION();   // Verify ImGui version
    ImGui::CreateContext(); // Create ImGui context
    // IO object (moved to EditorLayout_Load)
    ImGui::StyleColorsDark();                                     // Set dark theme
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);                 // Initialize GLFW backend
    ImGui_ImplOpenGL3_Init("#version 450");                       // Initialize OpenGL backend
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking

    // ReloadPrefabsList();

    ShowCreateGOMenu = false; // Hide create menu initially

    // init default docking positions
    // TODO TODO make this serialisable, so opens selected one
    std::string layoutPath = LayoutsFilePath + "DefaultLayout.ini";
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = layoutPath.c_str();
    ImGui::LoadIniSettingsFromDisk(layoutPath.c_str());

    // USE SERIALISATION AAAAAAAAAAA
    // ASSET MANAGER/ DIRECTORY NAVIGATOR VARIABLES
    // Vars for ImGui::ImageButton
    icon_folder_ui = GraphicsSystem::LoadTexture(icon_folder);
    icon_file_ui = GraphicsSystem::LoadTexture(icon_file);
    icon_plus_ui = GraphicsSystem::LoadTexture(icon_plus);
    icon_prefab_ui = GraphicsSystem::LoadTexture(icon_prefab);

    // Vars for asset manager interface
    paddingSize = 16.f;
    cellSize = icon_size.x + paddingSize;

    Serialization::Prefab_DragAndDropped = false;
}

/**
 * @brief Update - Runs each frame to update IMGui screens
 * @param dt		  Delta Time
 */
void ImGuiSystem::Update(float dt)
{
    if (InputManager::KeyStateImgui)
    {
        // ImGui_ImplOpenGL3_NewFrame(); // Start new OpenGL frame
        // ImGui_ImplGlfw_NewFrame();    // Start new GLFW frame
        // ImGui::NewFrame();        // Start ImGui frame

        // Full window docking
        ImGuiViewport *viewport = ImGui::GetMainViewport();
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
        ImGui::End();

        TopMenuBar_Render();
        Hierarchy_Render();

        // commented out bc editor mode isnt rly an editor mode, its more of a game view mode, dragndrop still works tho
        // if (CORE->GetGraphicsSystem()->IsEditorMode())
        {
            ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);

            ImGui::Begin("Game Viewport");
            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
            CORE->GetGraphicsSystem()->RenderGameViewportImGui(viewportSize.x, viewportSize.y);

            ImGui::Image((void *)(intptr_t)CORE->GetGraphicsSystem()->GetFrameBufferTexture(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));

            std::string testdata{};
            // dont rly wanna accept generic assets yet, because should accept those in specific scenarios like properties tab
            ReceiveDragAndDrop(PrefabPayloadID, testdata);
            ReceiveDragAndDrop(ScenePayloadID, testdata);
            ReceiveDragAndDrop(HierarchyPayloadID, testdata);
            if (!testdata.empty())
            {
                std::cout << testdata << std::endl;
                Serialization::LoadPrefab(testdata);
                // CompletedDragAndDrop();
            }

            ImGui::End();
        }

        // Begin main GameObject Inspector window
        ImGui::Begin("OLD HIERARCHY", &InputManager::KeyStateImgui);

        // Toggle GameObject creation menu
        if (ImGui::Button("Create New Game Object"))
        {
            ShowCreateGOMenu = (!ShowCreateGOMenu) ? true : false;
        }

        // Toggle prefab loading menu
        if (ImGui::Button("Load Prefabs"))
        {
            ReloadPrefabsList();
            ShowLoadPrefabMenu = (!ShowLoadPrefabMenu) ? true : false;
        }

        // Stress Test: Spawn 1000 GameObjects
        static bool addCollidersLater = true;
        static std::vector<GameObject*> stressObjects;
        static int colliderIndex = 0;

        const float spawnW = 16.f;
        const float spawnH = 16.f;

        if (ImGui::Button("Run Stress Test (1000 Objects)"))
        {
            auto& factory = GameObjectFactory::Instance();
            auto& resourceMgr = ResourceManager::Instance();

            stressObjects.clear();
            colliderIndex = 0;

            // Load the texture once, reuse for all obj
            const std::string spritePath = "./Assets/protosprite.png";
            const std::string textureKey = "stress_shared";

            TextureResource* texRes = resourceMgr.GetTexture(textureKey);
            if (!texRes)
            {
                if (!resourceMgr.LoadTexture(textureKey, spritePath))
                {
                    std::cerr << "[STRESS TEST] Failed to load shared texture at: " << spritePath << std::endl;
                    return;
                }
                texRes = resourceMgr.GetTexture(textureKey);
            }

            if (!texRes || !texRes->IsLoaded())
            {
                std::cerr << "[STRESS TEST] Texture not loaded. Aborting stress test.\n";
                return;
            }

            GLuint sharedTexID = texRes->GetTextureID();

            // --- Spawn 1000 objects ---
            for (int i = 0; i < 1001; ++i)
            {
                std::string objName = "StressObj_" + std::to_string(i);
                GameObject* go = factory.CreateGameObject(objName);
                stressObjects.push_back(go);

                // --- Transform: random position ---
                if (auto* t = go->AddComponent<TransformComponent>())
                {
                    float x = static_cast<float>(rand() % 1280) - 640;
                    float y = static_cast<float>(rand() % 720) - 360;                 
                    t->SetPosition(x, y);
                    t->SetScale(spawnW, spawnH);
                }

                // --- RenderComponent (visiblility) ---
                if (auto* render = go->AddComponent<RenderComponent>())
                {
                    render->SetTextureID(sharedTexID);
                    render->SetTexturePath(spritePath);
                    render->SetShapeType(RenderComponent::ShapeType::Square);
                    render->SetColor({ 1.f, 1.f, 1.f, 1.f });
                }
            }

            std::cout << "[STRESS TEST] Spawned 1000 game objects with Transform + Physics + Spritesheet.\n";
            addCollidersLater = true;
        }

        // --- Collider ---
        if (addCollidersLater)
        {
            int batchPerFrame = 50; // add 50 colliders per frame

            for (int i = 0; i < batchPerFrame && colliderIndex < static_cast<int>(stressObjects.size()); ++i, ++colliderIndex)
            {
                GameObject* go = stressObjects[colliderIndex];
                if (go && !go->GetComponent<ColliderComponent>())
                {
                    go->AddComponent<ColliderComponent>(
                        ColliderComponent::Type::Square,
                        spawnW, spawnH,
                        CollisionLayer::Enemy
                    );
                }

                // --- Physics: random velocity ---
                if (auto* p = go->AddComponent<PhysicsComponent>())
                {
                    float vx = static_cast<float>((rand() % 200) - 100);
                    float vy = static_cast<float>((rand() % 200) - 100);
                    p->SetVelocity(vx, vy);
                    p->SetMass(1.0f);
                    p->SetGrounded(false);
                    p->SetGravity(true);
                }
            }

            if (colliderIndex >= static_cast<int>(stressObjects.size()))
            {
                addCollidersLater = false;
                colliderIndex = 0;
                stressObjects.clear(); // clear tracking list
                std::cout << "[STRESS TEST] All stress-test colliders added.\n";
            }
        }

        // Render Asset Browser things here!
        AssetBrowserWindow.RenderImGui();

        // Iterate through all game objects
        for (auto &obj : GameObjectFactory::Instance().GetAllGameObjects())
        {
            if (!obj->IsActive())
            {
                continue; // Skip inactive objects
            }

            // Doing this so we can have objects with the same name in the hierachy, but with different hidden ids
            std::string displayName = obj->GetName() + "##" + std::to_string(obj->GetID());

            // Display tree node for each object
            if (ImGui::TreeNode(displayName.c_str()))
            {
                // Access transform component for editing
                if (auto *transform = obj->GetComponent<TransformComponent>())
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

                // ---- Save prefab button ----
                if (ImGui::Button("Save As Prefab"))
                {
                    Serialization::SavePrefab(*obj);
                    ReloadPrefabsList();
                }

                // ---- Clone object ----
                if (obj->GetName() != "Hero" && obj->GetName() != "Evil Button")
                {
                    if (ImGui::Button("Clone Object"))
                    {
                        auto &factory = GameObjectFactory::Instance();
                        GameObject *clone = factory.CloneGameObject(obj->GetID());

                        if (clone)
                        {
                            // Offset position slightly so clones do not overlap
                            if (auto *t = clone->GetComponent<TransformComponent>())
                            {
                                float x, y;
                                t->GetPosition(x, y);
                                t->SetPosition(x + 20.f, y + 20.f);
                            }
                        }
                    }
                }

                // Evil button that spawns 2.5k Circle GOs
            /*    if (obj && obj->GetName() == "Evil Button")
                {
                    if (ImGui::Button("Spawn 2500 Entities"))
                    {
                        DemoM1_SpawnGameObjects();
                        std::cout << "Spawned 2500 Circle Entities!\n";
                    }
                }*/

                ImGui::TreePop();

                // Altered for M1 demonstration of 2.5k GOs!
                // if (obj->GetName() != "Hero" && obj->GetName() != "Evil Button")
                //{
                //    if (ImGui::Button("Clone"))
                //    {
                //        GameObjectFactory& factory = GameObjectFactory::Instance();
                //        GameObject* clonedObj = factory.CloneGameObject(obj->GetID());

                //        // Save cloned object data
                //        Serialize_Writer writer("../Logs/GameObjectData.txt");
                //        if (writer.IsStreamGood())
                //        {
                //            writer.WriteGameObjectData(*clonedObj, 0);
                //            std::cout << "Cloned object data saved!\n";
                //        }

                //        std::cout << "button clicked";
                //    }

                //    // Save GO Data - Serialisation!
                //    if (ImGui::Button("Save Data"))
                //    {
                //        Serialize_Writer writer("../Logs/GameObjectData.txt"); // Create Writer class object with specified path
                //        if (writer.IsStreamGood())
                //        {
                //            writer.WriteGameObjectData(*obj, 0); // If stream is valid, write objects data to file path
                //            std::cout << "GameObject data saved!\n";
                //        }
                //    }

                //    // Create Prefab with GO Data
                //    if (ImGui::Button("Create Prefab"))
                //    {
                //        Serialization::SavePrefab(*obj, "");
                //        Serialization::GetAllPrefabJSONFiles();
                //    }
                //}
            }
        }

        ImGui::End();

        /**
         * @brief Game Object Creator Menu IMGui Window
         */
        if (ShowCreateGOMenu)
        {
            GOCreationMenu_Render();
        }

        /**
         * @brief Prefab Loader Menu IMGui Window
         */
        if (ShowLoadPrefabMenu)
        {
            PrefabMenu_Render();
        }

        // Render profiler! Make it toggleable?
        Profiler_Render();
    }
}

/**
 * @brief Terminates and releases resources allocated for imgui
 */
void ImGuiSystem::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown(); // Shutdown OpenGL3 backend
    ImGui_ImplGlfw_Shutdown();    // Shutdown GLFW backend
    ImGui::DestroyContext();      // Destroy ImGui context
}

/**
 * @brief Get System Name
 */
std::string ImGuiSystem::GetName()
{
    return "ImGui_System"; // Return descriptive system name
}

#pragma region Profiler ImGui
/**
 * @brief Performance Profiler IMGui Window
 */
void Profiler_Render()
{
    ImGui::Begin("=== Performance Profiler ===");

    static std::map<std::string, float> smoothedMs;
    static std::map<std::string, float> smoothedPercent;

    const float lerpSpeed = static_cast<float>(0.05); // smaller = smoother

    std::map<std::string, double>::iterator it;
    for (it = g_systemTimings.begin(); it != g_systemTimings.end(); ++it)
    {
        const std::string &name = it->first;
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
}
#pragma endregion

#pragma region Game Object Creation ImGui
/**
 * @brief Renders the game object creation menu and handles logic as well. I know it doesnt only render...but its the most intuitive naming convention btwn our programmers :)
 */
void GOCreationMenu_Render()
{
    ImGui::Begin("Game Object Creator", &ShowCreateGOMenu); // Mark beginning of window
    {
        // Game Object Text Field
        ImGui::Text("Name");
        if (ImGui::InputText("#", tempGO.name, sizeof(tempGO.name)))
        {
        } // Text input field for GO Name

        // Very ugly sorry, future impl can include a check for empty GOs
        const char *ShapeTypes[5] = {"Square", "Circle", "Line", "Point", "Empty"}; // Type of shapes
        ImGui::Text("Shape Type");
        if (ImGui::BeginCombo("##", ShapeTypes[current_shape]))
        { // Drop down menu for each shape
            for (int i = 0; i < 5; i++)
            {
                bool is_selected = (current_shape == i);
                if (ImGui::Selectable(ShapeTypes[i], is_selected))
                    current_shape = i;

                if (is_selected)
                    ImGui::SetItemDefaultFocus();

                // If selected shape is empty, block out colour selection
                greyOutGOSettings = (!strcmp(ShapeTypes[current_shape], "Empty")) ? true : false;
            }
            ImGui::EndCombo(); // End drop down menu
        }

        if (greyOutGOSettings)
        {
            ImGui::BeginDisabled(true);
        }

        //----- will be disabled if shape selected is empty -----

        // Shape Colour Picker
        ImGui::Text("Colour");
        if (ImGui::ColorEdit3(" ", (float *)&tempGO.color))
        {
        } // Colour picker to later store in Vec4

        //----- will be disabled if shape selected is empty -----

        if (greyOutGOSettings)
        {
            ImGui::EndDisabled();
        }

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
            GOCreationMenu_Spawn();
        }
    }
    ImGui::End(); // End of GO Creation
}

void ImGuiSystem::Hierarchy_Render()
{
    ImGui::SetNextWindowPos(ImVec2(10, 370), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(250, 600), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Hierarchy##LevelEditor", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    // Render top part to show adding of GOs
    if (ImGui::ImageButton("##Create GO Button", ImTextureID(icon_plus_ui), {icon_size_small.x, icon_size_small.y}, {0, 1}, {1, 0},
                           (ImVec4(1.f, 1.f, 1.f, .7f)), (ImVec4(1, 1, 1, 1))))
    {
        // this is a file, not sure if we wanna do anything yet
        ShowCreateGOMenu = (!ShowCreateGOMenu) ? true : false;
    }
    CreateHintText("Create Game Object");

    ImGui::SameLine();
    // Render top part to show adding of GOs
    if (ImGui::ImageButton("##Spawn Prefab", ImTextureID(icon_prefab_ui), {icon_size_small.x, icon_size_small.y}, {0, 1}, {1, 0},
                           (ImVec4(1.f, 1.f, 1.f, .7f)), (ImVec4(1, 1, 1, 1))))
    {
        // this is a file, not sure if we wanna do anything yet
        ShowLoadPrefabMenu = (!ShowLoadPrefabMenu) ? true : false;
    }
    CreateHintText("Spawn Prefab");

    ImGui::SameLine();
    if (ImGui::BeginMenu("[TEST] Create GO"))
    {
        ImGui::Text("or we could do something like this too?");
        if (ImGui::MenuItem("Create Empty", "cool grey key bind"))
        {
        }
        if (ImGui::MenuItem("Create Shape", "Shift+1"))
        {
        }
        ImGui::EndMenu();
    }

    // display current file name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
    ImGui::Text("Scene: %s", "XINYEE IDK HOW TO ADD SCENE NAME HERE");
    ImGui::PopStyleColor();
    ImGui::Separator();

    // display object count
    auto &allObjects = factory.GetAllGameObjects();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::Text("Game Objects: %zu", allObjects.size());
    ImGui::PopStyleColor();
    ImGui::Separator();

    // list all game objects
    for (const auto &objPtr : allObjects)
    {
        if (!objPtr->GetParent() && objPtr->IsActive())
        {
            Hierarchy_GameObjectTree_Renderer(objPtr.get(), 0, selectedGameObjectID, factory);
        }
    }
    ImGui::End();
}

//
// void ImGuiSystem::Hierarchy_Render()
//{
//    ImGui::SetNextWindowPos(ImVec2(10, 370), ImGuiCond_FirstUseEver);
//    ImGui::SetNextWindowSize(ImVec2(250, 600), ImGuiCond_FirstUseEver);
//
//    if (!ImGui::Begin("Hierarchy##LevelEditor", nullptr, ImGuiWindowFlags_NoCollapse))
//    {
//        ImGui::End();
//        return;
//    }
//
//    // Render top part to show adding of GOs
//    if (ImGui::ImageButton("##Create GO Button", ImTextureID(icon_plus_ui), { icon_size_small.x, icon_size_small.y }, { 0, 1 }, { 1, 0 },
//        (ImVec4(1.f, 1.f, 1.f, .7f)), (ImVec4(1, 1, 1, 1))))
//    {
//        // this is a file, not sure if we wanna do anything yet
//        ShowCreateGOMenu = (!ShowCreateGOMenu) ? true : false;
//    }
//    CreateHintText("Create Game Object");
//
//    ImGui::SameLine();
//    // Render top part to show adding of GOs
//    if (ImGui::ImageButton("##Spawn Prefab", ImTextureID(icon_prefab_ui), { icon_size_small.x, icon_size_small.y }, { 0, 1 }, { 1, 0 },
//        (ImVec4(1.f, 1.f, 1.f, .7f)), (ImVec4(1, 1, 1, 1))))
//    {
//        // this is a file, not sure if we wanna do anything yet
//        ShowLoadPrefabMenu = (!ShowLoadPrefabMenu) ? true : false;
//    }
//    CreateHintText("Spawn Prefab");
//
//    ImGui::SameLine();
//    if (ImGui::BeginMenu("[TEST] Create GO"))
//    {
//        ImGui::Text("or we could do something like this too?");
//        if (ImGui::MenuItem("Create Empty", "cool grey key bind"))
//        {
//
//        }
//        if (ImGui::MenuItem("Create Shape", "Shift+1"))
//        {
//
//        }
//        ImGui::EndMenu();
//    }
//
//
//
//
//    // display current file name
//    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
//    ImGui::Text("Scene: %s", "XINYEE IDK HOW TO ADD SCENE NAME HERE");
//    ImGui::PopStyleColor();
//    ImGui::Separator();
//
//    // display object count
//    auto& allObjects = factory.GetAllGameObjects();
//    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
//    ImGui::Text("Game Objects: %zu", allObjects.size());
//    ImGui::PopStyleColor();
//    ImGui::Separator();
//
//    // list all game objects
//    for (const auto& objPtr : allObjects)
//    {
//        // should change to greyed out instead of skipping entirely
//        if (!objPtr->IsActive()) // || objPtr->GetParent() != nullptr)      // skip stuff w parents bc we already rendering it idented
//            continue;
//
//        GameObject* obj = objPtr.get();
//        EntityID id = obj->GetID();
//
//        ImGui::Indent(15.0f);
//
//        // highlight if selected
//        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
//
//        // If has children, make it expandable, otherwise just a leaf
//        if (obj->GetAllChildren().empty()) {
//            flags |= ImGuiTreeNodeFlags_Leaf;
//        }
//
//        if (selectedGameObjectID == id)
//        {
//            flags |= ImGuiTreeNodeFlags_Selected;
//        }
//
//        bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)id, flags, "%s (ID: %u)",
//            obj->GetName().c_str(), id);
//
//        // Drag and drop for Parenting
//        // might wanna send id as data instead
//        if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
//        {
//            SendDragAndDrop(HierarchyPayloadID, obj->GetName(), obj->GetName());
//        }
//
//        {
//            std::string testdata{};
//            ReceiveDragAndDrop(HierarchyPayloadID, testdata);
//            // Do something with DnD data
//            if (!testdata.empty())
//            {
//                if (testdata != obj->GetName())
//                {
//                    std::cout << "[Hierarchy] Setting " << testdata << " to be child of " << obj->GetName() << std::endl;
//                }
//                else
//                {
//                    std::cout << "[Hierarchy] Invalid Drag and Drop source!\n";
//                }
//                CompletedDragAndDrop();
//            }
//        }
//
//        // select on click
//        if (ImGui::IsItemClicked())
//        {
//            SetSelectedGameObject(id);
//        }
//
//        // context menu
//        if (ImGui::BeginPopupContextItem())
//        {
//            if (ImGui::MenuItem("Select"))
//            {
//                SetSelectedGameObject(id);
//            }
//            if (ImGui::MenuItem("Save as Prefab"))
//            {
//                Serialization::SavePrefab(*obj);
//            }
//            if (ImGui::MenuItem("Clone"))
//            {
//                factory.CloneGameObject(id);
//            }
//            if (ImGui::MenuItem("Delete"))
//            {
//                RemoveGameObject(id);
//                if (selectedGameObjectID == id)
//                    selectedGameObjectID = 0;
//            }
//            ImGui::EndPopup();
//        }
//
//    }
//    ImGui::End();
//}
//

void ImGuiSystem::Hierarchy_GameObjectTree_Renderer(GameObject *obj, int depth, EntityID &selectedGameObjectID, GameObjectFactory &factory)
{
    if (!obj->IsActive())
        return;

    EntityID id = obj->GetID();

    // Apply indentation based on depth
    ImGui::Indent(depth * 15.0f);

    // Set tree node flags
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    // If has children, make it expandable, otherwise just a leaf
    if (obj->GetAllChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (selectedGameObjectID == id)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Display the tree node
    bool nodeOpen = ImGui::TreeNodeEx((void *)(intptr_t)id, flags, "%s (ID: %u)",
                                      obj->GetName().c_str(), id);

    // Drag and drop for Parenting
    // might wanna send id as data instead
    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        SendDragAndDrop(HierarchyPayloadID, obj->GetID(), obj->GetName());
    }

    {
        std::string testdata{};
        ReceiveDragAndDrop(HierarchyPayloadID, testdata);
        // Do something with DnD data
        if (!testdata.empty())
        {
            std::vector<EntityID> familyTree{};
            GetFamilyTree(obj, familyTree);

            std::cout << "GetFamilyTree() has: \n";
            for (auto const &r : familyTree)
            {
                std::cout << r << std::endl;
            }

            EntityID deliveredID = EntityID(std::stoi(testdata));
            auto check = std::find(familyTree.begin(), familyTree.end(), deliveredID);

            if (check == familyTree.end())
            {
                std::cout << "[Hierarchy] Setting " << testdata << " to be child of " << obj->GetName() << std::endl;
                GameObject *deliveredGO = factory.GetGameObject(deliveredID);
                deliveredGO->SetParent(obj);

                if (!nodeOpen && !obj->GetAllChildren().empty())
                {
                    ImGuiStorage *storage = ImGui::GetStateStorage();
                    storage->SetInt(ImGui::GetID((void *)(intptr_t)id), 1);
                    // nodeOpen = true;
                }
            }
            else
            {
                std::cout << "[Hierarchy] Invalid Drag and Drop source!\n";
            }
            CompletedDragAndDrop();
        }
    }

    // Select on click
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        selectedGameObjectID = id;
    }

    // Context menu
    if (ImGui::BeginPopupContextItem())
    {
        if (ImGui::MenuItem("Select"))
        {
            selectedGameObjectID = id;
        }
        if (ImGui::MenuItem("Save as Prefab"))
        {
            Serialization::SavePrefab(*obj);
        }
        if (ImGui::MenuItem("Clone"))
        {
            factory.CloneGameObject(id);
        }
        // find way to unparent by drag and drop in the future
        if (obj->GetParent() && ImGui::MenuItem("Make Root"))
        {
            obj->GetParent()->RemoveChild(obj);
        }

        if (ImGui::MenuItem("Delete"))
        {
            factory.DestroyGameObject(id);
            if (selectedGameObjectID == id)
                selectedGameObjectID = 0;
        }
        ImGui::EndPopup();
    }

    // Render children if node is open
    if (nodeOpen)
    {
        for (auto &childPtr : obj->GetAllChildren())
        {
            Hierarchy_GameObjectTree_Renderer(childPtr, depth + 1, selectedGameObjectID, factory);
        }
        ImGui::TreePop();
    }

    // Unindent
    ImGui::Unindent(depth * 15.0f);
}

/**
 * @brief Within the game object creation menu, handles the spawning of GOs with the specified properties
 */
void GOCreationMenu_Spawn()
{
    //auto& factory = GameObjectFactory::Instance(); // Factory
    //GameObject* TempGO = nullptr;

    //if (RenderComponent::ShapeType{ current_shape } == RenderComponent::ShapeType::Empty)
    //{
    //    TempGO = factory.CreateEmptyGO(tempGO.name, 20.f, 0.f, 5.f, 5.f);
    //}
    //else
    //{
    //    // very ugly colour code bc that uses a different vec 4 AAAA
    //    TempGO = factory.CreateShape(tempGO.name, RenderComponent::ShapeType{ current_shape }, 20.f, 0.f, 5.f, 5.f,
    //        vec4(tempGO.color.x, tempGO.color.y, tempGO.color.z, 1.f), CollisionLayer::Default);
    //}

    ////Add transform component to assign transform values
    //TempGO->AddComponent<TransformComponent>(0, 0);
    //auto* transform = TempGO->GetComponent<TransformComponent>();

    //transform->SetPosition(tempGO.pos[0], tempGO.pos[1]);
    //transform->SetScale(tempGO.scale[0], tempGO.scale[1]);
    //transform->SetRotation(tempGO.rot);

    // Determine creation type based on shape
    CreateObjectMessage::ObjectType objType;
    RenderComponent::ShapeType shapeType = static_cast<RenderComponent::ShapeType>(current_shape);

    if (shapeType == RenderComponent::ShapeType::Empty)
    {
        objType = CreateObjectMessage::ObjectType::Empty;
    }
    else
    {
        objType = CreateObjectMessage::ObjectType::Basic;
    }

    // Gather data from your temporary editor object
    float pos[2] = { tempGO.pos[0], tempGO.pos[1] };
    float scale[2] = { tempGO.scale[0], tempGO.scale[1] };
    float rot = tempGO.rot;
    vec4 color = vec4(tempGO.color.x, tempGO.color.y, tempGO.color.z, 1.f); // assuming ImVec4 in editor
    std::string name = tempGO.name;
    // Build and send message
    CreateObjectMessage msg(
        name,          // GameObject name
        objType,              // Empty or Basic
        shapeType,            // ShapeType
        pos, scale, rot,
        color
    );

    std::cout << "[ImGuiSystem] Sending CreateObjectMessage for " << tempGO.name << std::endl;
    //msg.Dispatch(*CORE);
    CORE->GetMessageBus().Publish(msg);

}

/**
 * @brief Get the selected game object
 */
// GameObject* ImGuiSystem::GetSelectedGameObject() const
//{
//     return factory.GetGameObject(selectedGameObjectID);
// }

/**
 * @brief Set the selected game object
 */
void ImGuiSystem::SetSelectedGameObject(EntityID id)
{
    selectedGameObjectID = id;
}

/**
 * @brief Add a game object to the level
 */
// GameObject* ImGuiSystem::AddGameObject(const GameObjectData& data)
//{
//     // commented for now, just use what we have first
//     //return DeserialiseGameObject(data);
// }

/**
 * @brief Remove a game object from the level
 */
void ImGuiSystem::RemoveGameObject(EntityID id)
{
    factory.DestroyGameObject(id);
}

#pragma endregion

#pragma region Prefab ImGui
/**
 * @brief Simply updates the prefabs directory with the newly added/ removed prefab files
 */
// void ReloadPrefabsList()
//{
//     prefabList = PrefabTools::LoadPrefabList();
// }

/**
 * @brief Renders the prefab window on screen
 */
void PrefabMenu_Render()
{
    ImGui::Begin("Prefab Loader", &ShowLoadPrefabMenu);

    ImGui::Text("Prefab Menu");
    ImGui::BeginGroup();
    if (ImGui::BeginListBox("##", ImVec2(200, 150))) // Draw selectable box for which prefabs are found
    {
        if (prefabList.empty())
        {
            ImGui::Text("No saved Prefabs found.");
        }
        else
        {
            // Draw list of prefabs
            for (int i = 0; i < prefabList.size(); ++i) // Loop for every prefab in list
            {
                if (prefab_renameIndex != i)
                {
                    bool is_selected = (current_prefab == i);
                    if (ImGui::Selectable(prefabList[i].c_str(), &is_selected))
                    {
                        current_prefab = i; // Selected item in list
                    }
                    else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        current_prefab = i; // optional: select on right-click too
                        ImGui::OpenPopup("Prefab_RightClickEvent");
                    }

                    // If item is selected and being held, drag and drop event is taking place
                    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        SendDragAndDrop(PrefabPayloadID, prefabList[i], prefabList[i]);
                    }
                    if (i != prefabList.size() - 1)
                    {
                        ImGui::Separator();
                    }
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
                        // ImGui::SetKeyboardFocusHere(-1);
                    }
                    // if (ImGui::IsKeyPressed(ImGuiKey_Escape))          //supposed to be escape but it closes the whole game LMAO
                    if (ImGui::IsKeyPressed(ImGuiKey_Tab) || ImGui::IsKeyPressed(ImGuiKey_Enter))
                    {
                        prefab_renameIndex = -1;
                        Serialization::RenamePrefab(prefab_oldname, rename_str);
                    }

                    ImGui::PopID();
                }
            }
        }

        if (ImGui::BeginPopup("Prefab_RightClickEvent")) // If an item in the prefab lists gets right clicked
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
                Serialization::RemovePrefab(prefabList[current_prefab]);
                ReloadPrefabsList();
            }
            ImGui::EndPopup();
        }

        ImGui::EndListBox();
    }
    ImGui::EndGroup();

    ImGui::SameLine(); // This so its on the right side of the selection box
    // draw preview
    ImGui::BeginGroup();
    ImVec2 boxSize = ImVec2(256, 256);
    ImGui::Text("Preview Render:");
    ImGui::Dummy(boxSize); // Reserves space for preview

    ImDrawList *drawList = ImGui::GetWindowDrawList(); // Imgui rendering for box with white outline
    drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 255));

    // if (ImGui::IsItemHovered() && isDragging)
    //{
    //     std::cout << "testing if ts works\n";           // yea it works LOL
    //     //std::string testdata{};
    //     //ReceiveDragAndDrop(AssetPayloadID, testdata);
    //     //std::cout << testdata << std::endl;
    //     isDragging = false;
    // }

    std::string testdata{};
    ReceiveDragAndDrop(AssetPayloadID, testdata);
    ReceiveDragAndDrop(HierarchyPayloadID, testdata);
    // Do something with DnD data
    if (!testdata.empty())
    {
        std::cout << testdata << std::endl;
        // CompletedDragAndDrop();
        // CompletedDragAndDrop();
    }
    CompletedDragAndDrop();
    ImGui::EndGroup();

    if (ImGui::Button("Spawn -> Load Prefab"))
    {
        PrefabMenu_Spawn();
    }

    ImGui::End(); // End of Prefab Search Menu
}

/**
 * @brief Ctor - Spawns the selected prefab as a game object in scene
 */
void PrefabMenu_Spawn()
{
    Serialization::LoadPrefab(prefabList[current_prefab]);
    std::cout << "prefab menu spawn pressed\n";
}
#pragma endregion

#pragma region Asset Browser/ Directory Navigator ImGui
/**
 * @brief Ctor - Inits Directory Navigator for asset manager
 */
DirectoryNavigator::DirectoryNavigator() : currentDir(assetPath) {}

/**
 * @brief Renders the asset manager section of the drag and drop
 */
void DirectoryNavigator::RenderImGui()
{
    ImGui::Begin("Project Navigator");

    if (currentDir != fs::path(assetPath))
    {
        if (ImGui::Button("back"))
        {
            currentDir = currentDir.parent_path();
        }
    }

    int columns = static_cast<int>((ImGui::GetContentRegionAvail().x) / cellSize);
    if (columns <= 0)
        columns = 1;
    ImGui::Columns(columns, 0, false);

    for (auto &iter : fs::directory_iterator(currentDir))
    {
        std::string path = iter.path().string();
        auto relativePath = fs::relative(iter.path(), assetPath);
        std::string fileName = relativePath.filename().string();

        if (iter.is_directory())
        {
            // add double click?
            if (ImGui::ImageButton(fileName.c_str(), (ImTextureID)(uintptr_t)(icon_folder_ui), {icon_size.x, icon_size.y}, {0, 1}, {1, 0}))
            {
                currentDir /= iter.path().filename();
            }
        }
        else
        {
            // add double click to open in windows file explorer?
            if (ImGui::ImageButton(fileName.c_str(), ImTextureID(icon_file_ui), {icon_size.x, icon_size.y}, {0, 1}, {1, 0}))
            {
                // this is a file, not sure if we wanna do anything yet
            }
            // Can only drag and drop files, not folders
            if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                // Depending on folder its in, send the respective payloadIDs to receive
                if (iter.path().parent_path().filename() == "Scenes")
                {
                    // std::cout << "in /Scenes\n";
                    SendDragAndDrop(ScenePayloadID, iter.path().string(), fileName.c_str());
                }
                else if (iter.path().parent_path().filename() == "Prefabs")
                {
                    // std::cout << "in /Prefabs\n";
                    SendDragAndDrop(PrefabPayloadID, iter.path().stem().string(), fileName);
                }
                else
                {
                    // std::cout << "sending Generic Asset\n";
                    SendDragAndDrop(AssetPayloadID, iter.path().string(), fileName.c_str());
                }
            }
        }

        // Wraps text nicely underneath icon
        ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + icon_size.x);
        ImGui::TextWrapped(fileName.c_str());
        ImGui::PopTextWrapPos();

        ImGui::NextColumn(); // go next
    }
    ImGui::Columns(1);
    ImGui::End();
}
#pragma endregion

#pragma region Drag&Drop Imgui
/**
 * @brief For drag and drop services. This is the 'sender', takes in a payloadID(specifying what you want to send), type of file and a display text for visual feedback
 *        Overloaded for arithmetic types and strings
 */
// FOR ASSET BROWSER PURPOSES IS JUST FOR STRINGS CONTAINING FILE PATH!
template <typename DataType>
void SendDragAndDrop(std::string const &PayloadID, DataType const &dataToSend, std::string const &displayText)
{
    static_assert(std::is_arithmetic_v<DataType>, "FUNCTION ONLY TAKES STRINGS OR ARITHMETIC TYPES!!!\n");
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        std::string data{std::to_string(dataToSend)};
        if (!data.empty())
        {
            ImGui::SetDragDropPayload(PayloadID.c_str(), data.c_str(), data.size() + 1, ImGuiCond_Once);
            // for template ^ might need to take in (filePath.size() + 1) * sizeof(DataType)
            ImGui::Text(displayText.c_str());
            ImGui::EndDragDropSource();
        }
        else
        {
            std::cout << "BAD CONVERSION OF " << typeid(DataType).name() << " to std::string!\n";
            return;
        }

        // can do like a check against which targets match the type, then change outline to show matchable slot
    }
}

void SendDragAndDrop(std::string const &PayloadID, std::string const &dataToSend, std::string const &displayText)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        // if dndcursor.x within viewport x and dndcursor.y within viewport y, then take as valid
        // if not either cancel dnd or spawn prefab with no pos
        ImGui::SetDragDropPayload(PayloadID.c_str(), dataToSend.c_str(), dataToSend.size() + 1, ImGuiCond_Once);
        // for template ^ might need to take in (filePath.size() + 1) * sizeof(DataType)
        ImGui::Text(displayText.c_str());
        // std::cout << "bool is: " << Serialization::Prefab_DragAndDropped << " | DND - successfully Sent\n";
        ImGui::EndDragDropSource();
        // can do like a check against which targets match the type, then change outline to show matchable slot
    }
}

/**
 * @brief For drag and drop services. This is the 'receiver', takes in a payloadID(specifying what you want to send)
 *        unfortunately cannot be templated bc BeginDragDropTarget only takes in c string...
 */
void ReceiveDragAndDrop(std::string const &PayloadID, std::string &dataToReceive)
{
    if (ImGui::BeginDragDropTarget()) // sets to true open hovering
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(PayloadID.c_str())) // sets to true when payload accepted
        {
            Serialization::Prefab_DragAndDropped = true;
            const char *droppedPath = (const char *)payload->Data; // can also change the lhs type and the rhs cast if need
            dataToReceive = droppedPath;
            // use droppedPath (load texture,model,etc)
            // droppedPath in this case is just the file path that we sent
            std::cout << "[DragNDrop] successfully Received\n";
        }
        ImGui::EndDragDropTarget();
    }
}

// future implementation, maybe use function pointers so can put receive + action function + complete, and wrap nicely

void CompletedDragAndDrop()
{
    if (Serialization::Prefab_DragAndDropped == false)
        return;

    Serialization::Prefab_DragAndDropped = false;
    std::cout << "[DragNDrop] successfully Completed\n";
}
#pragma endregion

void TopMenuBar_Render()
{
    if (ImGui::BeginMainMenuBar()) // Begin the top menu bar
    {
        // File menu
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open"))
            {
            }
            if (ImGui::MenuItem("Save"))
            {
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
            {
            }

            ImGui::EndMenu();
        }

        // Edit menu
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Preferences"))
            {
            }

            ImGui::EndMenu();
        }

        // Edit menu
        if (ImGui::BeginMenu("Window"))
        {
            if (ImGui::MenuItem("Save Editor Layout"))
            {
                openLayoutSavePopup = true;
            }
            if (ImGui::MenuItem("Load Editor Layout"))
            {
                openLayoutLoadPopup = true;
            }

            ImGui::EndMenu();
        }

        // Help menu
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Naw"))
            {
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (openLayoutSavePopup)
    {
        ImGui::OpenPopup("Save_Layout");
        EditorLayout_Refresh();
        openLayoutSavePopup = false;
    }
    if (openLayoutLoadPopup)
    {
        ImGui::OpenPopup("Load_Layout");
        // EditorLayout_Refresh();
        openLayoutLoadPopup = false;
    }
    EditorLayout_SaveNamePopUp();
    EditorLayout_LoadPopUp();
}

void EditorLayout_SaveScene(std::string const &fileName)
{
    // Saves docking layout to file, future implementation will have this be attached to a button (Prefences -> save editor layout etc)
    if (!fileName.empty())
    {
        // Save to a specific file
        std::string layoutPath = LayoutsFilePath + fileName + ".ini";
        ImGui::SaveIniSettingsToDisk(layoutPath.c_str());
        std::cout << "Saved window layout to: " << layoutPath << std::endl;
    }
}

void EditorLayout_LoadScene(std::string const &fileName)
{
    std::string layoutPath = LayoutsFilePath + fileName;
    if (fs::exists(layoutPath))
    {
        std::cout << "Loading ImGui layout from: " << layoutPath << std::endl;

        ImGui::LoadIniSettingsFromDisk(layoutPath.c_str());

        std::cout << "Layout file at path: " << layoutPath << " loaded\n";
    }
    else
    {
        std::cout << "Layout file at path: " << layoutPath << " not found\n";
    }
}

void EditorLayout_SaveNamePopUp()
{
    bool PushPopBgColor = false;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Save_Layout", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Name");
        if (saveNamePopupClear)
        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 0, 0, 100));
            PushPopBgColor = true;
        }
        if (ImGui::InputText("##SaveLayoutName", saveLayoutName, sizeof(saveLayoutName)))
        {
            saveNamePopupClear = false;
        }
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(-1);

        if (PushPopBgColor)
        {
            ImGui::PopStyleColor();
        }
        if (saveNamePopupClear)
        {
            ImGui::Text("Illegal name! Try something else");
        }

        if (ImGui::Button("Save"))
        {
            std::string fileName = saveLayoutName;
            // LAMBDA LAMBDA LAMBDA LAMBDA LAMBDA I CANNOT ESCAPE THIS HELL
            fileName.erase(std::remove_if(fileName.begin(), fileName.end(), [](char ch)
                                          { return std::isspace(ch); }),
                           fileName.end());

            if (fileName.empty() || fileName.find_first_of("<>:\"/\\|?*") != std::string::npos || fileName.back() == ' ' || fileName.back() == '.')
            {
                saveNamePopupClear = true;
            }
            else
            {
                // Save to a specific file
                std::string layoutPath = LayoutsFilePath + fileName + ".ini";
                ImGui::SaveIniSettingsToDisk(layoutPath.c_str());
                std::cout << "Saved window layout to: " << layoutPath << std::endl;
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void EditorLayout_LoadPopUp()
{
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(800, 600));

    if (ImGui::BeginPopupModal("Load_Layout", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (fs::directory_iterator(LayoutsFilePath) == fs::directory_iterator())
        {
            ImGui::Text("No layout files found");
            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
            return;
        }
        std::string IntroText{"Layouts Found in: " + LayoutsFilePath};
        ImGui::Text(IntroText.c_str());

        int columns = static_cast<int>((ImGui::GetContentRegionAvail().x) / cellSize);
        if (columns <= 0)
        {
            columns = 1;
        }
        ImGui::Columns(columns, 0, false);

        for (auto &iter : fs::directory_iterator(LayoutsFilePath))
        {
            if (iter.path().extension() != ".ini")
                continue;

            std::string path = iter.path().string();
            auto relativePath = fs::relative(iter.path(), LayoutsFilePath);
            std::string layoutFileName = relativePath.filename().string();

            if (ImGui::ImageButton(layoutFileName.c_str(), (ImTextureID)(uintptr_t)(icon_file_ui), {icon_size.x, icon_size.y}, {0, 1}, {1, 0}))
            {
                EditorLayout_LoadScene(layoutFileName);
            }

            // Wraps text nicely underneath icon
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + icon_size.x);
            ImGui::TextWrapped(layoutFileName.c_str());
            ImGui::PopTextWrapPos();

            ImGui::NextColumn(); // go next
        }
        ImGui::Columns(1);

        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void EditorLayout_Refresh()
{
    editorLayoutList.clear();

    if (!fs::exists(LayoutsFilePath))
        return;

    for (const auto &file : fs::directory_iterator(LayoutsFilePath))
    {
        if (file.is_regular_file() && file.path().extension() == ".ini")
        {
            editorLayoutList.push_back(file.path().filename().string());
            std::cout << file.path().string() << std::endl;
        }
    }
}

// Wrapper to easily create Hints when hovering over Imgui stuff. Call directly under Imgui element! (Text, buttons etc.)
void CreateHintText(std::string const &displayText)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text(displayText.c_str());
        ImGui::EndTooltip();
    }
}