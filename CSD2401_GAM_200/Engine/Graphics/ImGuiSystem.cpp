/* Start Header ************************************************************************/
/*!
\file       ImGuiSystem.cpp
\author     Guan Shao Jun [17% - 200 lines]
\co-author	Muhammad Ashfer [76% - 1049 lines]
            Hwee Min [7% - 82 lines]
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
#include <algorithm>
#include <cctype>  
#include "core.h"
#include "RenderComponent.h"
#include <typeinfo>
#include <type_traits>
#include "LevelEditor.h"
#include "ColliderComponent.h"
#include "physics.h"
#include "AudioManager.h"

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

// STATIC BOOL FLAGS
static bool greyOutGOSettings = false;
static bool openLayoutSavePopup = false;
static bool openLayoutLoadPopup = false;
static bool saveNamePopupClear = false;
static bool renderLoadedLayout = false;
static bool showAudioAssetsPanel = false;
static bool g_showAudioErrorPopup = false;
static std::string g_audioErrorMessage;

bool isHoveringOverViewport;

// Vars for asset manager interface
const float paddingSize = 16.f;
const float cellSize = icon_size.x + paddingSize;
DirectoryNavigator AssetBrowserWindow;

// DRAG AND DROP VARS
ImVec2 viewportCursorPos;

// LAYOUT LOADING 
std::string loadLayoutPath{};

// TEXTURE IDS FOR ICONS
GLuint icon_folder_ui;
GLuint icon_file_ui;
GLuint icon_plus_ui;
GLuint icon_prefab_ui;
GLuint icon_audio_ui;
GLuint icon_json_ui;

// selected game object for editing
static EntityID selectedGameObjectID = 0;

/**
 * @brief Ctor - Inits window variable
 */
ImGuiSystem::ImGuiSystem(GLFWwindow *window) : m_window(window), factory(GameObjectFactory::Instance()), selectedGameObjectID(0), showPropertyEditor(true) {}

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
    // TODO make this serialisable, so it remembers which one to load
    std::string layoutPath = LayoutsFilePath + "DefaultLayout.ini";
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = layoutPath.c_str();
    ImGui::LoadIniSettingsFromDisk(layoutPath.c_str());

    // Imgui flags
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    // ASSET MANAGER/ DIRECTORY NAVIGATOR VARIABLES
    // Vars for ImGui::ImageButton
    icon_folder_ui = GraphicsSystem::LoadTexture(icon_folder);
    icon_file_ui = GraphicsSystem::LoadTexture(icon_file);
    icon_plus_ui = GraphicsSystem::LoadTexture(icon_plus);
    icon_prefab_ui = GraphicsSystem::LoadTexture(icon_prefab);
    icon_audio_ui = GraphicsSystem::LoadTexture(icon_audio);
}

/**
 * @brief Update - Runs each frame to update IMGui screens
 * @param dt		  Delta Time
 */
void ImGuiSystem::Update(float dt)
{
    if (renderLoadedLayout)
    {
        // reload layouts next frame, do this so won't crash/ have inconsistent frame end times
        EditorLayout_LoadLayout(loadLayoutPath);
        renderLoadedLayout = false;                 // set flags that will open imgui windows
        openLayoutLoadPopup = true;
        return;
    }
    if (InputManager::KeyStateImgui)
    {
        // KEPT HERE FOR OUR REFERENCE - BUT MOVED THIS TO CORE ALREADY
        // ImGui_ImplOpenGL3_NewFrame(); // Start new OpenGL frame
        // ImGui_ImplGlfw_NewFrame();    // Start new GLFW frame
        // ImGui::NewFrame();        // Start ImGui frame
        
        // Full window docking
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        // Flags & Docking Settings
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

        LevelEditor* levelEditor = CORE->GetLevelEditor();
        bool isPlaying = (levelEditor && levelEditor->GetEditorMode() == EditorMode::Play);

        if (!isPlaying)
        {
            CORE->GetGraphicsSystem()->SetEditorMode(true);
            // GAME VIEWPORT - Always visible (outside conditional so it shows even in play mode)
            {
                ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiCond_FirstUseEver);

                ImGui::Begin("Game Viewport");
                ImVec2 viewportSize = ImGui::GetContentRegionAvail();                                   // Viewport checks how much space there is at sets it as size
                CORE->GetGraphicsSystem()->RenderGameViewportImGui(viewportSize.x, viewportSize.y);

                ImGui::SetItemAllowOverlap();                                                           // to drag and click without moving window
                ImGui::Image((void *)(intptr_t)CORE->GetGraphicsSystem()->GetFrameBufferTexture(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));           // Viewport rendering

                // If hovering over viewport, you can click to select a GO
                // TODO implement outline for selected GO
                if (ImGui::IsItemHovered())
                {
                    Viewport_ClickToSelect();
                    isHoveringOverViewport = true;
                }
                else
                {
                    isHoveringOverViewport = false;
                }

                std::string testdata{};
                // Viewport accepts Prefabs for drag and drop
                ReceiveDragAndDrop(PrefabPayloadID, testdata);
                //ReceiveDragAndDrop(ScenePayloadID, testdata);                 // TODO drag and drop scene to load saved scene
                if (!testdata.empty())
                {
                    // filter for type of data here
                    std::cout << testdata << std::endl;                         // Prints the data sent via drag and drop
                    Serialization::LoadPrefab(testdata);
                }

                // Get cursor position relative to the image
                ImVec2 cursorScreenPos = ImGui::GetMousePos();
                ImVec2 itemScreenPos = ImGui::GetItemRectMin();

                // Takes cursor position - converts it to viewport space that translates to world space too [centre of screen is (0,0)]
                // TODO Currently does not take into account camera zoom
                viewportCursorPos = ImVec2(
                    cursorScreenPos.x - itemScreenPos.x - (viewportSize.x / 2.f),
                    (viewportSize.y / 2.f) - (cursorScreenPos.y - itemScreenPos.y));

                ImGui::Text("Hovering at: (%.1f, %.1f)", viewportCursorPos.x, viewportCursorPos.y);
                ImGui::End();
            }

            // Only show editor UI when not in play mode
            Hierarchy_Render();
            AssetBrowserWindow.RenderImGui();

            // only show audio assets if toggled on
            if (showAudioAssetsPanel)
            {
                AudioAssetsPanel_Render();
            }

            // render property editor for selected game object
            if (showPropertyEditor)
            {
                RenderPropertyEditor();
            }
            // Render profiler! Make it toggleable?
            Profiler_Render();
        }
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
        // Flag with 1 frame delay on whether something is being drag and dropped, used for spawning prefabs via DND
        Serialization::Prefab_DragAndDropped = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
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

/**
 * @brief Manages logic for clicking to select GOs and DNDing to move them around
 */
void ImGuiSystem::Viewport_ClickToSelect()
{
    static bool isBeingDragged = false;

    // Loop for all GOs in scene
    for (auto& go : GameObjectFactory::Instance().GetAllGameObjects())
    {
        if (!go->IsActive()) continue;

        auto* transform = go->GetComponent<TransformComponent>();           // GOs with no transform will not be interactable
        if (!transform) continue;

        vec2 pos{ 0.f, 0.f }, scale{ 0.f, 0.f };                            // Get position and scale
        transform->GetPosition(pos.x, pos.y);
        transform->GetScale(scale.x, scale.y);

        if (go->GetComponent<ColliderComponent>())
        {
            // if GO has a collider
            // Calculates if cursor is within area of GO
            if (viewportCursorPos.x > pos.x - scale.x / 2 && viewportCursorPos.x < pos.x + scale.x / 2 && viewportCursorPos.y > pos.y - scale.y / 2 && viewportCursorPos.y < pos.y + scale.y / 2)
            {
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))              // Set to on release as form of confirmation, so GOs must be selected and THEN dragged. To avoid accidentally moving
                {
                    SetSelectedGameObject(go->GetID());                         // Set GO to be selected
                    
                    std::cout << "[has collider]clicked on " << go->GetName() << std::endl;
                }
                // If GO has been selected and is dragged (with a threshhold of 25 units - added for human misinput and accidental moving)
                if (selectedGameObjectID == go->GetID() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 25.f))
                {
                    isBeingDragged = true;          // Set flag that updates position 
                }
            }
        }
        else
        {
            // if GO does NOT have a collider
            // Find general distance between object and cursor
            float distance = std::sqrt(std::pow(viewportCursorPos.x - pos.x, 2) + std::pow(viewportCursorPos.y - pos.y, 2));

            if (distance < 50.f)                // if object is within 50 units (within general area)
            {
                if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))                      // Click to select, after selecting drag to set flag to true
                {
                    SetSelectedGameObject(go->GetID());
                    std::cout << "[no colider]clicked on " << go->GetName() << std::endl;
                }
                if (selectedGameObjectID == go->GetID() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 10.f))
                {
                    isBeingDragged = true;
                }
            }
        }
    }

    // if flag is true and left click is held, set the new position of the GO based off cursor (which is fitted to world space!)
    if (isBeingDragged && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        GetSelectedGameObject()->GetComponent<TransformComponent>()->SetPosition(viewportCursorPos.x, viewportCursorPos.y);
    }
    // upon mouse release set flag to false
    if (isBeingDragged && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        isBeingDragged = false;
    }
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


/**
 * @brief New cool hierarchy! Now handles DND and other scene related features
 */
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
    // Render top part to show adding of Prefabs
    if (ImGui::ImageButton("##Spawn Prefab", ImTextureID(icon_prefab_ui), {icon_size_small.x, icon_size_small.y}, {0, 1}, {1, 0},
                           (ImVec4(1.f, 1.f, 1.f, .7f)), (ImVec4(1, 1, 1, 1))))
    {
        // this is a file, not sure if we wanna do anything yet
        ReloadPrefabsList();
        ShowLoadPrefabMenu = (!ShowLoadPrefabMenu) ? true : false;
    }
    CreateHintText("Spawn Prefab");

    ImGui::SameLine();
    // Audio Assets toggle button
    if (ImGui::ImageButton("##AudioAssetsToggle", ImTextureID(icon_audio_ui),
        { icon_size_small.x, icon_size_small.y },
        { 0, 1 }, { 1, 0 },
        ImVec4(1.f, 1.f, 1.f, 0.7f), ImVec4(1, 1, 1, 1)))
    {
        showAudioAssetsPanel = !showAudioAssetsPanel;
    }
    CreateHintText("Toggle Audio Assets panel");

    // THIS IS JUST A SAMPLE I THOUGHT IT WAS COOL, SEE IF WE LIKE THEN ILL PORT OVER TO THIS
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

    // dear god help us all
    if (ImGui::Button("Stress Test - 1000 GOs"))
    {
        M2_RenderStressTest();
    }

    // display current scene name
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
    if (auto* levelEditor = CORE->GetLevelEditor())
    {
        ImGui::Text("Scene: %s", CORE->GetLevelEditor()->GetCurrentFilename().c_str());
    }
    else
    {
        ImGui::Text("Scene: Unknown");
    }
    ImGui::PopStyleColor();
    ImGui::Separator();
        
    // display object count
    auto &allObjects = factory.GetAllGameObjects();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
    ImGui::Text("Game Objects: %zu", allObjects.size());
    ImGui::PopStyleColor();
    ImGui::Separator();

    // list all game objects, if they have no parent (root), if not render the tree display!
    for (const auto &objPtr : allObjects)
    {
        if (!objPtr->GetParent())
        {
            Hierarchy_GameObjectTree_Renderer(objPtr.get(), 0, selectedGameObjectID, factory);
        }
    }
    // remove selected GO when clicking on empty space
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (!ImGui::IsAnyItemHovered())
        {
            selectedGameObjectID = 0;
        }
    }
    ImGui::End();
}

void ImGuiSystem::Hierarchy_GameObjectTree_Renderer(GameObject *obj, int depth, EntityID &selectedGameObjectID, GameObjectFactory &factory)
{
    // Just commented this out in case want to revert, but now we can show inactive GOs but greyed out
    //if (!obj->IsActive())
    //    return;

    EntityID id = obj->GetID();

    // Apply indentation based on depth
    ImGui::Indent(depth * 10.0f);

    // Set tree node flags
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    // If has children, make it expandable
    if (obj->GetAllChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (selectedGameObjectID == id)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if (!obj->IsActive())                       // if GO is inactive, make it grey
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }

    // Display the tree node
    bool nodeOpen = ImGui::TreeNodeEx((void *)(intptr_t)id, flags, "%s (ID: %u)", obj->GetName().c_str(), id);

    if (!obj->IsActive())
    {
        ImGui::PopStyleColor();                 // pop the push
    }

    // Drag and drop for Parenting
    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        SendDragAndDrop(HierarchyPayloadID, obj->GetID(), obj->GetName());
    }

    {
        // receiving drag and drop
        std::string testdata{};
        ReceiveDragAndDrop(HierarchyPayloadID, testdata);
        // Do something with DnD data
        if (!testdata.empty())
        {
            std::vector<EntityID> familyTree{};
            GetFamilyTree(obj, familyTree);                         // Get all descendent game objects (child of child wtv) underneath the parrent GO

            std::cout << "GetFamilyTree() has: \n";
            for (auto const &r : familyTree)
            {
                std::cout << r << std::endl;
            }

            EntityID deliveredID = EntityID(std::stoi(testdata));       // Convert payload data to EntityID
            auto check = std::find(familyTree.begin(), familyTree.end(), deliveredID);

            if (check == familyTree.end())
            {
                std::cout << "[Hierarchy] Setting " << testdata << " to be child of " << obj->GetName() << std::endl;
                GameObject *deliveredGO = factory.GetGameObject(deliveredID);
                deliveredGO->SetParent(obj);                                        // Set Dragged GO to be parent of receiver GO

                if (!nodeOpen && !obj->GetAllChildren().empty())
                {
                    // Sets tree node to be expanded then renders it 
                    ImGuiStorage *storage = ImGui::GetStateStorage();
                    storage->SetInt(ImGui::GetID((void *)(intptr_t)id), 1);
                }
            }
            else
            {
                std::cout << "[Hierarchy] Invalid Drag and Drop source!\n";
            }
        }
    }

    // Set selected item
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        selectedGameObjectID = id;
    }

    // Right Click event (for specific GO in hierarchy)
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
            {
                selectedGameObjectID = 0;
            }
        }
        ImGui::EndPopup();
    }

    // Render all children of opened parent GO
    if (nodeOpen)
    {
        // repeats loop again
        for (auto &childPtr : obj->GetAllChildren())
        {
            Hierarchy_GameObjectTree_Renderer(childPtr, depth+1, selectedGameObjectID, factory);
        }
        ImGui::TreePop();
    }
    ImGui::Unindent(depth * 10.0f);
}

/**
 * @brief Within the game object creation menu, handles the spawning of GOs with the specified properties
 */
void GOCreationMenu_Spawn()
{
    // Determine creation type based on shape
    CreateObjectMessage::ObjectType objType;
    RenderComponent::ShapeType shapeType = static_cast<RenderComponent::ShapeType>(current_shape);      // get shape type enum

    // Special case for empty GOs
    if (shapeType == RenderComponent::ShapeType::Empty)
    {
        objType = CreateObjectMessage::ObjectType::Empty;
    }
    else
    {
        objType = CreateObjectMessage::ObjectType::Basic;
    }

    
    // Gather GO data to send
    float pos[2] = { tempGO.pos[0], tempGO.pos[1] };
    float scale[2] = { tempGO.scale[0], tempGO.scale[1] };
    float rot = tempGO.rot;
    vec4 color = vec4(tempGO.color.x, tempGO.color.y, tempGO.color.z, 1.f);
    std::string name = tempGO.name;

    // Construct message and send
    CreateObjectMessage msg(
        name,                   // GameObject name
        objType,                // Shape if is Empty or Basic
        shapeType,              // ShapeType
        pos, scale, rot,        // Transform
        color                   // Colour
    );

    std::cout << "[ImGuiSystem] Sending CreateObjectMessage for " << tempGO.name << std::endl;
    //msg.Dispatch(*CORE);
    CORE->GetMessageBus().Publish(msg);
}

/**
 * @brief Set the selected game object
 */
void ImGuiSystem::SetSelectedGameObject(EntityID id)
{
    selectedGameObjectID = id;

    // TODO WIP FOR HIGHLIGHTING SELECTED GO, I GOT SO LOST IN THE SAUCE LMAO ITS ALMOST DONE THO I JUST DK HOW TO GET VP MATRIX
    //auto const& go = GameObjectFactory::Instance().GetGameObject(id);
    //auto* transform = go->GetComponent<TransformComponent>();

    //if (transform)
    //{
    //    float x, y, sx, sy;
    //    
    //    //CameraManager::Instance().GetViewProjectMatrix(renderWidth, renderHeight);
    //    transform->GetPosition(x, y);
    //    transform->GetScale(sx, sy);
    //    if (CORE->GetGraphicsSystem())
    //    {
    //        std::cout << "azaaaaaaaaaaaa";
    //    }
    //    CORE->GetGraphicsSystem()->DrawRectOutline(x,y,sx,sy,{0,1,0,1}, CORE->GetGraphicsSystem()->getVp());
    //}
}

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
                        current_prefab = i;         // Set selected
                    }
                    else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        current_prefab = i;         // Right click to select and open menu too
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
                    // For renaming of prefab files
                    ImGui::PushID(i);
                    if (prefab_renameFocus)
                    {
                        ImGui::SetKeyboardFocusHere();
                        prefab_renameFocus = false;
                    }
                    if (ImGui::InputText("##", rename_str, IM_ARRAYSIZE(rename_str), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        prefabList[i] = std::string(rename_str);
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

        if (ImGui::BeginPopup("Prefab_RightClickEvent"))            // If item in the prefab lists gets right clicked
        {
            if (ImGui::MenuItem("Rename"))                          // If rename button clicked, get name of input text field and set it to new name
            {
                prefab_renameIndex = current_prefab;
                strncpy(rename_str, prefabList[current_prefab].c_str(), sizeof(rename_str));
                prefab_renameFocus = true;
                prefab_oldname = prefabList[current_prefab];
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete"))
            {
                Serialization::RemovePrefab(prefabList[current_prefab]);        // Remove from file, reload list afterwards
                ReloadPrefabsList();
            }
            ImGui::EndPopup();
        }

        ImGui::EndListBox();
    }
    ImGui::EndGroup();

    ImGui::SameLine(); // This so its on the right side of the selection box

    // draw preview
    // TODO doesnt actually work yet
    ImGui::BeginGroup();
    ImVec2 boxSize = ImVec2(256, 256);
    ImGui::Text("Preview Render:");
    ImGui::Dummy(boxSize); // Reserves space for preview

    ImDrawList *drawList = ImGui::GetWindowDrawList(); // Imgui rendering for box with white outline
    drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 255));

    // Test for if drag and drop works
    std::string testdata{};
    ReceiveDragAndDrop(AssetPayloadID, testdata);
    ReceiveDragAndDrop(HierarchyPayloadID, testdata);
    // Do something with DnD data
    if (!testdata.empty())
    {
        std::cout << testdata << std::endl;
    }
    ImGui::EndGroup();

    if (ImGui::Button("Spawn -> Load Prefab"))
    {
        PrefabMenu_Spawn();                     // Spawn selected prefab as GO
    }

    ImGui::End(); // End of Prefab Search Menu
}

/**
 * @brief Ctor - Spawns the selected prefab as a game object in scene
 */
void PrefabMenu_Spawn()
{
    Serialization::LoadPrefab(prefabList[current_prefab]);          // Deserialises the selected prefab
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
    // Currently project navigator only shows files in assets, not sure if we wanna do anything beyond that

    if (currentDir != fs::path(assetPath))
    {
        if (ImGui::Button("back"))                  // Go back button
        {
            currentDir = currentDir.parent_path();
        }
    }

    // Get amount of columns depending on space available
    int columns = static_cast<int>((ImGui::GetContentRegionAvail().x) / cellSize);
    if (columns <= 0)
        columns = 1;
    ImGui::Columns(columns, 0, false);

    // For each item in the current folder
    for (auto &iter : fs::directory_iterator(currentDir))
    {
        std::string path = iter.path().string();                            // Path we are currently in
        auto relativePath = fs::relative(iter.path(), assetPath);           // Path relative from where we started (assets) to where we are
        std::string fileName = relativePath.filename().string();            // name of the file alone
        std::string extension = iter.path().extension().string();

        // If the current item is a folder
        if (iter.is_directory())
        {
            // TODO add double click? or is single click fine
            if (ImGui::ImageButton(fileName.c_str(), (ImTextureID)(uintptr_t)(icon_folder_ui), {icon_size.x, icon_size.y}, {0, 1}, {1, 0}))     // Draw image button with folder icon
            {
                currentDir /= iter.path().filename();           // Upon clicking, enter the file path
            }
        }
        // If current item is a file 
        else
        {
            // this is a file, not sure if we wanna do anything yet (when we click)
            // add double click to open in windows file explorer?

            // Json files
            if (extension == ".json")
            {
                if (ImGui::ImageButton(fileName.c_str(), ImTextureID(icon_json_ui), icon_size, { 0, 1 }, { 1, 0 }))
                {
                    // do nothin for now
                }
            }
            // Audio Files
            else if (extension == ".wav" || extension == ".mp3" || extension == ".ogg" || extension == ".flac")
            {
                if (ImGui::ImageButton(fileName.c_str(), ImTextureID(icon_audio_ui), icon_size, { 0, 1 }, { 1, 0 }))
                {
                    // do nothin for now
                }
            }

            // Can only drag and drop files, not folders
            if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                // Depending on folder its in, send the respective payloadIDs to receive
                if (iter.path().parent_path().filename() == "Scenes")
                {
                    SendDragAndDrop(ScenePayloadID, iter.path().string(), fileName.c_str());            // For scenes files
                }
                else if (iter.path().parent_path().filename() == "Prefabs")
                {
                    SendDragAndDrop(PrefabPayloadID, iter.path().stem().string(), fileName);            // For prefab files
                }
                else if (iter.path().parent_path().filename() == "Audio" || iter.path().parent_path().filename() == "audio")    //for some reason its lowercase in property editor?
                {
                    SendDragAndDrop(AudioPayloadID, iter.path().string(), fileName.c_str());            // For Audio files
                }
                else
                {
                    SendDragAndDrop(AssetPayloadID, iter.path().string(), fileName.c_str());            // Anything else considered an general asset
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
    static_assert(std::is_arithmetic_v<DataType>, "FUNCTION ONLY TAKES STRINGS OR ARITHMETIC TYPES!!!\n");          // ONLY ACCEPT NUMBERS
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))                                           // For drag and drop SENDER
    {
        std::string data{std::to_string(dataToSend)};
        if (!data.empty())
        {
            ImGui::SetDragDropPayload(PayloadID.c_str(), data.c_str(), data.size() + 1, ImGuiCond_Once);            // Send payload
            ImGui::Text(displayText.c_str());                                                                       // Set display text for visual feedback
            ImGui::EndDragDropSource();
        }
        else
        {
            std::cout << "BAD CONVERSION OF " << typeid(DataType).name() << " to std::string!\n";                   // something went wrong with converting data type to string
            return;
        }
    }
}

// overload for strings as data to send, generally same thing
void SendDragAndDrop(std::string const &PayloadID, std::string const &dataToSend, std::string const &displayText)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(PayloadID.c_str(), dataToSend.c_str(), dataToSend.size() + 1, ImGuiCond_Once);
        ImGui::Text(displayText.c_str());
        ImGui::EndDragDropSource();
    }
}

/**
 * @brief For drag and drop services. This is the 'receiver', takes in a payloadID(specifying what you want to send)
 *        unfortunately cannot be templated bc BeginDragDropTarget only takes in c string...
 */
void ReceiveDragAndDrop(std::string const &PayloadID, std::string &dataToReceive)                                   // For drag and drop RECEIVER
{
    if (ImGui::BeginDragDropTarget())                                                                               // sets to true open hovering
    {
        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(PayloadID.c_str()))                          // sets to true when payload accepted
        {
            const char *droppedPath = (const char *)payload->Data;                                                  // can also change the lhs type and the rhs cast if need
            dataToReceive = droppedPath;
            // use droppedPath (load texture,model,etc)
            // droppedPath in this case is just the file path that we sent
            std::cout << "[DragNDrop] successfully Received\n";
        }
        ImGui::EndDragDropTarget();
    }
}

#pragma endregion


/**
 * @brief Renders the top menu bar that has application settings
 */
void TopMenuBar_Render()
{
    if (ImGui::BeginMainMenuBar()) // Begin the top menu toolbar thingy
    {
        // TODO largely WIP
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
                openLayoutSavePopup = true;                     // if clicked, set flags for saving scenes
            }
            if (ImGui::MenuItem("Load Editor Layout"))
            {
                openLayoutLoadPopup = true;                     // if clicked, set flags for loading scenes
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

    // Have to separate bc refreshes every frame and got some crashes with end popup, this keeps it up
    if (openLayoutSavePopup)
    {
        ImGui::OpenPopup("Save_Layout");                        // Open respective pop up
        EditorLayout_Refresh();
        openLayoutSavePopup = false;
    }
    if (openLayoutLoadPopup)
    {
        ImGui::OpenPopup("Load_Layout");                        // Open respective pop up
        EditorLayout_Refresh();
        openLayoutLoadPopup = false;
    }
    EditorLayout_SaveNamePopUp();                               // Update popup checks
    EditorLayout_LoadPopUp();
}

/**
 * @brief Save imgui layout to file
 */
void EditorLayout_SaveLayout(std::string const &fileName)
{
    // Saves docking layout to file, future implementation will have this be attached to a button (Prefences -> save editor layout etc)
    if (!fileName.empty())
    {
        // Save to a specific file
        std::string layoutPath = LayoutsFilePath + fileName + ".ini";               // Have to specify this if not it saves as generic file
        ImGui::SaveIniSettingsToDisk(layoutPath.c_str());                           // Save settings to ini file
        std::cout << "Saved window layout to: " << layoutPath << std::endl;
    }
}

/**
 * @brief Load imgui layout
 */
void EditorLayout_LoadLayout(std::string const &fileName)
{
    std::string layoutPath = LayoutsFilePath + fileName;                            // get the directory 
    if (fs::exists(layoutPath))
    {
        ImGui::LoadIniSettingsFromDisk(layoutPath.c_str());                         // Load the settings (gets refreshed in the top of update)

        std::cout << "Layout file at path: " << layoutPath << " loaded\n";
    }
    else
    {
        std::cout << "Layout file at path: " << layoutPath << " not found\n";
    }
}

/**
 * @brief Saving of layout POPUP, checks for illegal file names too
 */
void EditorLayout_SaveNamePopUp()
{
    bool PushPopBgColor = false;
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Save_Layout", nullptr, ImGuiWindowFlags_AlwaysAutoResize))              // modal got that cool grey background
    {
        ImGui::Text("Name");
        if (saveNamePopupClear)
        {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(255, 0, 0, 100));                          // red evil scary box for people trying to save weird file names
            PushPopBgColor = true;
        }
        if (ImGui::InputText("##SaveLayoutName", saveLayoutName, sizeof(saveLayoutName)))
        {
            saveNamePopupClear = false;                                                 // after getting illegal warning, type to clear the red box
        }
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere(-1);

        if (PushPopBgColor)
        {
            ImGui::PopStyleColor();                                                     // pop the push
        }
        if (saveNamePopupClear)
        {
            ImGui::Text("Illegal name! Try something else");                            // if user sets an illegal file name, set flag to true which triggers this text
        }

        if (ImGui::Button("Save"))
        {
            std::string fileName = saveLayoutName;
            // LAMBDA LAMBDA LAMBDA LAMBDA LAMBDA I CANNOT ESCAPE THIS HELL
            fileName.erase(std::remove_if(fileName.begin(), fileName.end(), [](char ch)
                                          { return std::isspace(ch); }),fileName.end());

            // list of illegal chars in filename
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

/**
 * @brief Loading of layout POPUP
 */
void EditorLayout_LoadPopUp()
{
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(800, 600));

    // basically the same as project/ asset navigator
    if (ImGui::BeginPopupModal("Load_Layout", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (fs::directory_iterator(LayoutsFilePath) == fs::directory_iterator())                    // if no layouts are found just show empty with text
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

        // Adjusting column position based on size of window
        int columns = static_cast<int>((ImGui::GetContentRegionAvail().x) / cellSize);
        if (columns <= 0)
        {
            columns = 1;
        }
        ImGui::Columns(columns, 0, false);

        // Loops through all items with .ini file type
        for (auto &iter : fs::directory_iterator(LayoutsFilePath))
        {
            if (iter.path().extension() != ".ini")
                continue;

            std::string path = iter.path().string();
            auto relativePath = fs::relative(iter.path(), LayoutsFilePath);
            std::string layoutFileName = relativePath.filename().string();

            // each item will have the file icon, on click set the layoutpath to string. Itll then be loaded by the layout loader function
            if (ImGui::ImageButton(layoutFileName.c_str(), (ImTextureID)(uintptr_t)(icon_file_ui), {icon_size.x, icon_size.y}, {0, 1}, {1, 0}))
            {
                loadLayoutPath = layoutFileName;
                renderLoadedLayout = true;
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

/**
 * @brief Refreshes the vector of layouts, used for loading. OLD CODE!
 */
void EditorLayout_Refresh()
{
    editorLayoutList.clear();

    if (!fs::exists(LayoutsFilePath))
        return;

    // Loops through all .ini files in the folder
    for (const auto &file : fs::directory_iterator(LayoutsFilePath))
    {
        if (file.is_regular_file() && file.path().extension() == ".ini")
        {
            editorLayoutList.push_back(file.path().filename().string());
            std::cout << file.path().string() << std::endl;
        }
    }
}


/**
 * @brief Wrapper to easily create Hints when hovering over Imgui stuff. Call directly under Imgui element! (Text, buttons etc.)
 */
void CreateHintText(std::string const &displayText)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text(displayText.c_str());
        ImGui::EndTooltip();
    }
}

/**
 * @brief Get pointer to selected game object
 */
GameObject* ImGuiSystem::GetSelectedGameObject() const
{
    if (selectedGameObjectID == 0)
        return nullptr;

    return factory.GetGameObject(selectedGameObjectID);
}

/**
 * @brief Render property editor for Selected GOs. Will be empty by default, and then show details if a GO is selected.
 */
void ImGuiSystem::RenderPropertyEditor()
{
    ImGui::SetNextWindowPos(ImVec2(1400, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 900), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Properties##ImGuiSystem", &showPropertyEditor, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    GameObject* obj = GetSelectedGameObject();
    if (!obj)
    {
        // if selected go is nullptr
        ImGui::End();
        return;
    }

    ImGui::Text("GameObject: %s", obj->GetName().c_str());
    ImGui::Text("ID: %u", obj->GetID());
    ImGui::Separator();

    // Check box for is active
    // TODO add eye icon like unity on the side in hierarchy to enable/ disable. or right click option
    bool isActive = obj->IsActive();
    if (ImGui::Checkbox("Active", &isActive))
    {
        obj->SetActive(isActive);
    }

    ImGui::Spacing();

    // GO transform screen
    if (auto* transform = obj->GetComponent<TransformComponent>())
    {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // sliders for position, scale, rotation etc

            float pos[2] = { 0.0f, 0.0f };
            transform->GetPosition(pos[0], pos[1]);
            if (ImGui::DragFloat2("Position", pos, 0.5f))
            {
                transform->SetPosition(pos[0], pos[1]);
            }

            float scale[2] = { 1.0f, 1.0f };
            transform->GetScale(scale[0], scale[1]);
            if (ImGui::DragFloat2("Scale", scale, 0.5f))
            {
                transform->SetScale(scale[0], scale[1]);
            }

            float rotation = transform->GetRotation();
            if (ImGui::DragFloat("Rotation", &rotation, 0.5f))
            {
                transform->SetRotation(rotation);
            }
        }
    }

    // Render component for GO
    if (auto* render = obj->GetComponent<RenderComponent>())
    {
        if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Shape Type: %s", GetShapeTypeName((int)render->GetShapeType()));

            // Show colour picker for GO colour (only if no texture id for custom texture/ sprite)
            if (render->GetTextureID() == 0)
            {
                vec4 color = render->GetColor();
                float colorArray[4] = { color.x, color.y, color.z, color.w };
                if (ImGui::ColorEdit4("Color", colorArray))
                {
                    render->SetColor(vec4(colorArray[0], colorArray[1], colorArray[2], colorArray[3]));
                }
            }
            else
            {
                ImGui::Text("Texture ID: %u", render->GetTextureID());
            }
        }
    }

    // Physics component
    if (auto* physics = obj->GetComponent<PhysicsComponent>())
    {
        if (ImGui::CollapsingHeader("Physics"))
        {
            // Physics sliders with respective functions
            float vel[2] = { 0.0f, 0.0f };
            physics->GetVelocity(vel[0], vel[1]);
            if (ImGui::DragFloat2("Velocity", vel, 0.1f))
            {
                physics->SetVelocity(vel[0], vel[1]);
            }

            bool isStatic = physics->IsStatic();
            if (ImGui::Checkbox("Static", &isStatic))
            {
                physics->SetStatic(isStatic);
            }

            bool hasGravity = physics->useGravity;
            if (ImGui::Checkbox("Gravity", &hasGravity))
            {
                physics->SetGravity(hasGravity);
            }

            ImGui::Text("Mass: %.2f", physics->GetMass());
            ImGui::Text("Grounded: %s", physics->IsGrounded() ? "Yes" : "No");
        }
    }

    // Collider component
    if (auto* collider = obj->GetComponent<ColliderComponent>())
    {
        // For now only display text of collider details
        if (ImGui::CollapsingHeader("Collider"))
        {
            ImGui::Text("Type: %s", collider->GetType() == ColliderComponent::Type::Circle ? "Circle" : "Square");

            if (collider->GetType() == ColliderComponent::Type::Circle)
            {
                Circle circle = collider->GetCircle();
                ImGui::Text("Radius: %.2f", circle.radius);
            }
            else
            {
                AABB aabb = collider->GetAABB();
                float width = aabb.max.x - aabb.min.x;
                float height = aabb.max.y - aabb.min.y;
                ImGui::Text("Width: %.2f", width);
                ImGui::Text("Height: %.2f", height);
            }

            ImGui::Text("Layer: %s", GetCollisionLayerName((int)collider->GetLayer()));
        }
    }

    // TODO WIP drag and drop section for assets from project navigator
    // currently we dont have drag and drop spot for assets for textures etc, so this is just here to show it works 
    // Draws a box and lets it take in drag and drops for assets

    ImGui::BeginGroup();
    ImVec2 boxSize = ImVec2(256, 256);
    ImGui::Text("Preview:");
    ImGui::Dummy(boxSize); // Reserves space for preview

    ImDrawList* drawList = ImGui::GetWindowDrawList(); // Imgui rendering for box with white outline
    drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 255));

    static std::string testdata{};
    ReceiveDragAndDrop(AssetPayloadID, testdata);

    
    ImGui::EndGroup();
    // Do something with DnD asset data - maybe insert into boxes to assign sprites
    ImGui::Text(testdata.c_str());

    ImGui::End();
}

/**
 * @brief HELLO PROFS THIS IS THE STRESS TEST FUNCTION. Spawns 1000 GOs
 */
void M2_RenderStressTest()
{
    // Stress Test:
    auto& factory = GameObjectFactory::Instance();
    auto& resourceMgr = ResourceManager::Instance();

    static std::vector<GameObject*> stressObjects;
    stressObjects.reserve(1000);

    const int totalObjects = 1000;
    const float spawnW = 32.f;
    const float spawnH = 32.f;

    // Load the texture once, reuse for all obj
    const std::string spritePath = "../Assets/protosprite.png";
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
    for (int i = 0; i < totalObjects; ++i)
    {
        std::string objName = "StressObj_" + std::to_string(i);
        GameObject* go = factory.CreateGameObject(objName);
        stressObjects.push_back(go);

        // --- Transform: random position ---
        if (auto* t = go->AddComponent<TransformComponent>())
        {
            float x = static_cast<float>(rand() % 1280) - 640;
            float y = static_cast<float>(rand() % 720) - 360 - (i * 0.1f); // small offset to avoid stacking
            t->SetPosition(x, y);
            t->SetScale(spawnW, spawnH);
        }

        // --- RenderComponent (visiblility) ---
        if (auto* render = go->AddComponent<RenderComponent>())
        {
            render->SetTextureID(sharedTexID);
            render->SetTexturePath(spritePath);
            render->SetShapeType(RenderComponent::ShapeType::Circle);
            render->SetColor({ 1.f, 1.f, 1.f, 1.f });
        }
    }

    std::cout << "[STRESS TEST] Spawned " << totalObjects
        << " objects with Transform + Render.\n";

    // --- Add Collider & Physics gradually to avoid frame freeze ---
    static bool addCollidersLater = true;
    static int colliderIndex = 0;
    const int batchPerFrame = 50; // smaller batch = smoother

    //--- Collider ---
    if (addCollidersLater)
    {
        for (int i = 0; i < batchPerFrame && colliderIndex < totalObjects; ++i, ++colliderIndex)
        {
            GameObject* go = stressObjects[colliderIndex];

            if (!go) continue;

            // Collider
            if (!go->GetComponent<ColliderComponent>())
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
                if (vx == 0 && vy == 0)
                    vx = (rand() % 2 == 0 ? 50.f : -50.f); // ensure nonzero

                p->SetVelocity(vx, vy + 100.f); // small upward bias
                p->SetMass(1.0f);
                p->SetGrounded(false);
                p->SetGravity(true);
            }

        }

        if (colliderIndex >= totalObjects)
        {
            addCollidersLater = false;
            colliderIndex = 0;
            stressObjects.clear(); // clear tracking list
            std::cout << "[STRESS TEST] All stress-test colliders added.\n";
        }
    }
}

#pragma region Audio Assets ImGui
// Renders the Audio Assets panel
void AudioAssetsPanel_Render()
{
    // use the flag to control visibility
    if (!showAudioAssetsPanel)
        return;

    bool open = true;

    ImGui::SetNextWindowSize(ImVec2(350, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Audio Assets", &open))
    {
        // even if collapsed still need to handle closing
        if (!open)
        {
            showAudioAssetsPanel = false;
        }
            
        ImGui::End();
        return;
    }

    if (!open)
    {
        showAudioAssetsPanel = false;
        ImGui::End();
        return;
    }

    AudioManager* audioMgr = CORE->GetAudioManager();
    if (!audioMgr)
    {
        ImGui::TextDisabled("AudioManager not available");
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("Drag .wav / .mp3 / .ogg files from Project Navigator into this window to add them to the AudioManager");
    ImGui::Separator();

    // global controls
    if (ImGui::Button("Stop All Sounds"))
    {
        audioMgr->StopAllSounds();
    }
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::Text("Drop zone:");

    ImVec2 dropSize = ImVec2(ImGui::GetContentRegionAvail().x, 120.0f);
    ImGui::Dummy(dropSize);     // big area as the target

    // draw an outline so it's visible... to change ltr on
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    dl->AddRect(min, max, IM_COL32(200, 200, 200, 255));

    if (ImGui::BeginDragDropTarget())
    {
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(AudioPayloadID.c_str());
        if (!payload)
            payload = ImGui::AcceptDragDropPayload(AssetPayloadID.c_str());
        if (!payload)
            payload = ImGui::AcceptDragDropPayload(ScenePayloadID.c_str());
        if (!payload)
            payload = ImGui::AcceptDragDropPayload(PrefabPayloadID.c_str());
        if (!payload)
            payload = ImGui::AcceptDragDropPayload(HierarchyPayloadID.c_str());

        if (payload)
        {
            const char* droppedPath = (const char*)payload->Data;
            std::string droppedPathStr = droppedPath;

            fs::path p(droppedPathStr);
            std::string ext = p.extension().string();

            // normalise extension to lowercase
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")
            {
                SoundData sound{};
                sound.filepath = droppedPathStr;
                sound.name = p.stem().string();
                sound.loop = false;
                sound.volume = 1.0f;
                sound.category = "sfx";

                audioMgr->AddSound(sound);

                if (audioMgr->LoadSound(sound))
                {
                    std::cout << "[Audio Assets] Added and loaded sound: " << sound.name << std::endl;
                }
                else
                {
                    std::cout << "[Audio Assets] Failed to load sound: " << sound.name << std::endl;
                }
            }
            else
            {
                // trigger imgui error popup
                g_audioErrorMessage = "Unsupported audio file type: " + ext + "\n\nSupported formats: .wav, .mp3, .ogg";
                g_showAudioErrorPopup = true;
                std::cout << "[Audio Assets] Unsupported audio type: " << ext << std::endl;
            }
        }
        ImGui::EndDragDropTarget();
    }

    std::string soundToRemove;
    // show current audio database
    auto& db = audioMgr->GetSoundDatabase();
    if (db.empty())
    {
        ImGui::TextDisabled("No sounds added yet");
    }
    else
    {
        if (ImGui::BeginTable("AudioAssetsTable", 5,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Category");
            ImGui::TableSetupColumn("Loop");
            ImGui::TableSetupColumn("Volume");
            ImGui::TableSetupColumn("Actions");
            ImGui::TableHeadersRow();

            for (auto& pair : db)
            {
                SoundData& sound = pair.second;

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(sound.name.c_str());

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted(sound.category.c_str());

                // loop
                ImGui::TableSetColumnIndex(2);
                {
                    bool loop = sound.loop;
                    if (ImGui::Checkbox(("##loop_" + sound.name).c_str(), &loop))
                    {
                        sound.loop = loop;
                        audioMgr->ReloadSound(sound.name);

                        std::cout << "[Audio Assets] Updated loop for " << sound.name << " = " << (sound.loop ? "true" : "false") << std::endl;
                    }
                }

                // volume
                ImGui::TableSetColumnIndex(3);
                {
                    float volume = sound.volume;
                    if (ImGui::SliderFloat(("##vol_" + sound.name).c_str(),
                        &volume, 0.0f, 1.0f))
                    {
                        sound.volume = volume;
                        audioMgr->SetSoundVolume(sound.name, volume);
                        std::cout << "[Audio Assets] Updated volume for " << sound.name << " = " << sound.volume << std::endl;
                    }
                }

                ImGui::TableSetColumnIndex(4);
                std::string playLabel = "Play##" + sound.name;
                if (ImGui::SmallButton(playLabel.c_str()))
                {
                    audioMgr->PlaySound(sound.name);
                }
                ImGui::SameLine();

                // stop button
                std::string stopLabel = "Stop##" + sound.name;
                if (ImGui::SmallButton(stopLabel.c_str()))
                {
                    audioMgr->StopSound(sound.name + "_channel");
                }

                // remove sound button
                std::string removeLabel = "Remove##" + sound.name;
                if (ImGui::SmallButton(removeLabel.c_str()))
                {
                    soundToRemove = sound.name;
                }
            }

            ImGui::EndTable();
        }
        if (!soundToRemove.empty())
        {
            audioMgr->RemoveSound(soundToRemove);
        }
    }

    // audio file error popup centered and top most
    if (g_showAudioErrorPopup)
    {
        ImGui::OpenPopup("Audio File Error");
        g_showAudioErrorPopup = false; // only open once
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 center = viewport->GetCenter();
    ImGui::SetNextWindowSize(ImVec2(320, 180), ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Audio File Error", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse))
    {
        ImGui::TextWrapped("%s", g_audioErrorMessage.c_str());
        ImGui::Separator();

        ImGui::Dummy(ImVec2(0.0f, 4.0f));
        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX()
            + (ImGui::GetContentRegionAvail().x - 120.0f) * 0.5f
        );
        if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();

    if (!open)
    { 
        showAudioAssetsPanel = false;
    }
}
#pragma endregion

// Helpers
/**
 * @brief Refreshes vector of prefabs
 */
void ReloadPrefabsList()
{
    prefabList = Serialization::GetAllPrefabs();
}

/**
 * @brief Get text of shape type
 */
const char* ImGuiSystem::GetShapeTypeName(int type)
{
    switch (type)
    {
    case 0:
        return "Square";
    case 1:
        return "Circle";
    case 2:
        return "Line";
    case 3:
        return "Point";
    default:
        return "Unknown";
    }
}

/**
 * @brief Get Collision layer string
 */
const char* ImGuiSystem::GetCollisionLayerName(int layer)
{
    switch (layer)
    {
    case 0:
        return "Player";
    case 1:
        return "Ground";
    case 2:
        return "Wall";
    case 3:
        return "Enemy";
    case 4:
        return "Platform";
    case 5:
        return "Default";
    default:
        return "Unknown";
    }
}
