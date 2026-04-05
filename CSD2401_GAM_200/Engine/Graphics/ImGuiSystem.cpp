/* Start Header ************************************************************************/
/*!
\file       ImGuiSystem.cpp
\author     Guan Shao Jun [10% - 200 lines]
\co-author	Muhammad Ashfer [87% - 1941 lines]
            Hwee Min [3% - 697 lines]
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
#include "LayerManager.h"
#include "LayerComponent.h"
#include <typeinfo>
#include <type_traits>
#include "LevelEditor.h"
#include "TilemapEditor.h"
#include "ColliderComponent.h"
#include "physics.h"
#include "AudioManager.h"
#include "logic.h"
#include "ScriptComponent.h"
#include "TextComponent.h"
#include "imgui.h"
#include "imgui_internal.h"

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
 * @param collider
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

    bool addCollider = false;
    int colType = 0; // 0 for Square, 1 for Circle
    float colSize[2]{ 50.f, 50.f};
    int colLayer = 5; // Default index for "Default" layer
};

// GO CREATION MENU VARS
TempGOData tempGO{"Game Object"};
static int current_shape = 0;

// Flags for IMGUI (current selection in drop down for GO Creator, and showing of some screens)
bool ShowCreateGOMenu;
bool ShowLoadPrefabMenu;

// SAVE/ LOAD EDITOR VARS
std::vector<std::string> editorLayoutList;
char saveLayoutName[128]{""};

// PREFAB VARS
std::vector<std::string> prefabList;
TempGOData tempPrefab;
static int current_prefab = 0;
static int prefab_renameIndex = -1;
static bool prefab_renameFocus = false;
char rename_str[128]{""};
std::string prefab_oldname{};

// HIERARCHY VARS
static int hierarchy_renameIndex = -1;
static bool hierarchy_renameFocus = false;
char hierarchy_rename_str[128]{ "" };
std::string hierarchy_oldname{};

// CONTAINERS
std::vector < std::tuple<ComponentType, std::string, ImTextureID, std::function<void(GameObject*)>>>  AllComponents;
std::unordered_map<std::string, std::vector<EntityID>> prefabInstances;

// STATIC BOOL FLAGS
static bool greyOutGOSettings = false;
static bool openLayoutSavePopup = false;
static bool openLayoutLoadPopup = false;
static bool saveNamePopupClear = false;
static bool renderLoadedLayout = false;
static bool showAudioAssetsPanel = false;
static bool g_showAudioErrorPopup = false;
static std::string g_audioErrorMessage;
static bool showTilemapEditor = true;
static std::unique_ptr<TilemapEditor> tilemapEditor = nullptr;
static std::string g_editorErrorMessage;
static bool g_showEditorErrorPopup = false;


// VIEWPORT
bool isHoveringOverViewport;
bool isViewportFocused;
int gizmo_type = -1;
ImVec2 viewportSize;
ImVec2 viewportPos;
std::unordered_set<EntityID> lockedGizmosGOs{};

// ANIMATOR
bool isAnimPreviewFocused;

// PROPERTIES
char draggedSprite_str[128]{"<Drag Image Here>"};

// IMGUI COLOURS
ImVec4 color_icon_highlighted{ 0.5f, 0.5f, 0.5f, .7f };
ImVec4 color_icon_normal{ 1.f, 1.f, 1.f, .7f };
ImVec4 color_icon_bg{ 1.f, 1.f, 1.f, 1.f };

// Vars for asset manager interface
const float paddingSize = 16.f;
const float cellSize = icon_size.x + paddingSize;
DirectoryNavigator AssetBrowserWindow;

// DRAG AND DROP VARS
ImVec2 viewportCursorPos;
vec2 Prefab_Reinstantiate_Pos;

// LAYOUT LOADING 
std::string loadLayoutPath{};

// TEXTURE IDS FOR ICONS
GLuint icon_folder_ui;
GLuint icon_file_ui;
GLuint icon_sprite_ui;
GLuint icon_plus_ui;
GLuint icon_prefab_ui;
GLuint icon_audio_ui;
GLuint icon_json_ui;
GLuint icon_giz_select_ui;
GLuint icon_giz_translate_ui;
GLuint icon_giz_scale_ui;
GLuint icon_giz_rotate_ui;
GLuint icon_giz_resize_ui;
GLuint icon_active_ui;
GLuint icon_inactive_ui;

// selected game object for editing
static EntityID selectedGameObjectID = 0;

// stress test 
static bool stressTestActive = false;

// UNDO/REDO TRACKER
static bool isEditingProperty = false;
static json propertyBeforeJson;

struct UndoTracker 
{
    GameObject* obj;
    LevelEditor* editor;
    bool& isEditing;
    json& beforeJson;

    // Construct handles BOTH Gizmos and Sliders
    UndoTracker(GameObject* o, LevelEditor* ed, bool& editingFlag, json& bJson, bool isCurrentlyUsing = false)
        : obj(o), editor(ed), isEditing(editingFlag), beforeJson(bJson)
    {
        if (beforeJson.is_null()) {
            beforeJson = json::object();
        }

        bool started = isCurrentlyUsing || ImGui::IsItemActivated();

        if (!isEditing && started)
        {
            isEditing = true;
            beforeJson = Serialization::SerializeGameObject(obj);
        }
    }

    // Static helper for instant actions for Undo/Redo
    static void RecordInstantChange(GameObject* obj, LevelEditor* editor, json& beforeJson)
    {
        EditorUndoCommand cmd;
        cmd.action = EditorUndoCommand::ActionType::ObjectModified;
        cmd.entityID = obj->GetID();
        cmd.beforeJson = beforeJson;
        cmd.afterJson = Serialization::SerializeGameObject(obj);

        if (cmd.beforeJson == cmd.afterJson) {
            std::cout << "[Undo System] WARNING: Before and After states are identical. Redo will have no effect." << std::endl;
        }

        editor->RecordUndo(cmd);
    }

    static void RecordDeletion(GameObject* obj, LevelEditor* editor, GameObjectFactory& factory) {
        if (!obj || !editor) return;

        EditorUndoCommand cmd;
        cmd.action = EditorUndoCommand::ActionType::ObjectDeleted;
        cmd.entityID = obj->GetID();

        // Capture everything BEFORE we kill it
        cmd.beforeJson = Serialization::SerializeGameObject(obj);

        // Capture Hierarchy context
        GameObject* parent = obj->GetParent();
        cmd.parentID = parent ? parent->GetID() : -1;
        
        if (parent) {
            for (GameObject* c : parent->GetAllChildren())
                cmd.siblingOrderIDs.push_back(c->GetID());
        }
        else {
            for (auto& u : factory.GetAllGameObjects()) // Ensure factory has this helper
                cmd.rootOrderIDs.push_back(u->GetID());
        }

        for (GameObject* c : obj->GetAllChildren()) {
            cmd.childIDs.push_back(c->GetID());
        }

        if (CORE->GetLevelEditor()) {
            CORE->GetLevelEditor()->RecordUndo(cmd);
        }
    }

    static void RecordCreation(GameObject* obj, LevelEditor* editor, GameObjectFactory& factory)
    {
        if (!obj || !editor) return;

        EditorUndoCommand cmd;
        cmd.action = EditorUndoCommand::ActionType::ObjectCreated;
        cmd.entityID = obj->GetID();

        // Capture everything needed to redo (re-create) the object
        cmd.afterJson = Serialization::SerializeGameObject(obj);

        // Capture hierarchy context so redo can put it back in the same place
        GameObject* parent = obj->GetParent();
        cmd.parentID = parent ? parent->GetID() : -1;

        if (parent)
        {
            // Order among siblings under the same parent (includes the new object)
            for (GameObject* c : parent->GetAllChildren())
                cmd.siblingOrderIDs.push_back(c->GetID());
        }
        else
        {
            // Order among roots (includes the new object)
            for (auto& u : factory.GetAllGameObjects())
                cmd.rootOrderIDs.push_back(u->GetID());
        }

        editor->RecordUndo(cmd);
    }

    ~UndoTracker() 
    {
        if (!obj || beforeJson.is_null() || !GameObjectFactory::Instance().GetGameObject(obj->GetID())) {
            isEditing = false;
            return;
        }

        bool stoppedGizmo = (isEditing && !ImGuizmo::IsUsing() && !ImGui::IsAnyItemActive());
        bool stoppedSlider = ImGui::IsItemDeactivatedAfterEdit();

        if (stoppedGizmo || stoppedSlider) 
        {
            isEditing = false;

            EditorUndoCommand cmd;
            cmd.action = EditorUndoCommand::ActionType::ObjectModified;
            cmd.entityID = obj->GetID();
            cmd.beforeJson = beforeJson;
            cmd.afterJson = Serialization::SerializeGameObject(obj);

            editor->RecordUndo(cmd);
        }
    }
};

static void RenderEditorErrorPopup()
{
    // Only call OpenPopup once per trigger   not every frame
    if (g_showEditorErrorPopup)
    {
        ImGui::OpenPopup("Editor Error##Crash");
        g_showEditorErrorPopup = false;
    }

    if (ImGui::BeginPopupModal("Editor Error##Crash", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse))
    {
        // Pin to center   safe to call here, inside the modal
        ImGui::SetWindowPos(
            ImVec2(
                ImGui::GetMainViewport()->GetCenter().x - 250.0f,
                ImGui::GetMainViewport()->GetCenter().y - 80.0f
            ),
            ImGuiCond_Appearing
        );
        ImGui::SetWindowSize(ImVec2(500.0f, 0.0f), ImGuiCond_Appearing);

        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f),
            "An error occurred in the editor:");
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextWrapped("%s", g_editorErrorMessage.c_str());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextDisabled("The editor is still running. No data was lost.");
        ImGui::Spacing();

        // Centre the OK button
        float regionW = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX((regionW - 120.0f) * 0.5f);
        if (ImGui::Button("OK", ImVec2(120.0f, 0.0f)))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

// Repairs ImGui's internal stack after a mid-frame exception.
// Without this, unclosed Begin/PushStyleColor/PushStyleVar calls cause
// IM_ASSERT - abort() before the catch block can show the popup.
static void RepairImGuiStack(int savedColorDepth,
    int savedStyleVarDepth,
    int savedWindowDepth)
{
    ImGuiContext* ctx = ImGui::GetCurrentContext();
    if (!ctx) return;

    // Close any unclosed ImGui::Begin() windows
    while (ctx->CurrentWindowStack.Size > savedWindowDepth)
    {
        std::cerr << "[ImGuiRepair] Closing unclosed window: "
            << (ctx->CurrentWindow ? ctx->CurrentWindow->Name : "?") << "\n";
        ImGui::End();
    }

    // Pop any unclosed PushStyleColor()
    int colorExcess = ctx->ColorStack.Size - savedColorDepth;
    if (colorExcess > 0)
    {
        std::cerr << "[ImGuiRepair] Popping " << colorExcess
            << " unclosed PushStyleColor.\n";
        ImGui::PopStyleColor(colorExcess);
    }

    // Pop any unclosed PushStyleVar()
    int styleVarExcess = ctx->StyleVarStack.Size - savedStyleVarDepth;
    if (styleVarExcess > 0)
    {
        std::cerr << "[ImGuiRepair] Popping " << styleVarExcess
            << " unclosed PushStyleVar.\n";
        ImGui::PopStyleVar(styleVarExcess);
    }
}

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
    //ImGui::GetIO().FontGlobalScale = 1.5f;                        // Jo's setting
    ImGui::StyleColorsDark();                                     // Set dark theme
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);                 // Initialize GLFW backend
    ImGui_ImplOpenGL3_Init("#version 450");                       // Initialize OpenGL backend
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable docking
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    // ReloadPrefabsList();

    ShowCreateGOMenu = false; // Hide create menu initially

    // init default docking positions
    // TODO make this serialisable, so it remembers which one to load
    std::string layoutPath = LayoutsFilePath + "DefaultLayout.ini";
    ImGuiIO &io = ImGui::GetIO();
    //io.IniFilename = layoutPath.c_str();
    io.IniFilename = nullptr;
    ImGui::LoadIniSettingsFromDisk(layoutPath.c_str());

    // Imgui flags
    io.ConfigWindowsMoveFromTitleBarOnly = true;

    tilemapEditor = std::make_unique<TilemapEditor>();

    // ASSET MANAGER/ DIRECTORY NAVIGATOR VARIABLES
    // Vars for ImGui::ImageButton
    icon_folder_ui = GraphicsSystem::LoadTexture(icon_folder);
    icon_file_ui = GraphicsSystem::LoadTexture(icon_file);
    icon_sprite_ui = GraphicsSystem::LoadTexture(icon_sprite);
    icon_plus_ui = GraphicsSystem::LoadTexture(icon_plus);
    icon_prefab_ui = GraphicsSystem::LoadTexture(icon_prefab);
    icon_audio_ui = GraphicsSystem::LoadTexture(icon_audio);
    icon_json_ui = GraphicsSystem::LoadTexture(icon_json);
    icon_giz_select_ui = GraphicsSystem::LoadTexture(icon_giz_select);
    icon_giz_translate_ui = GraphicsSystem::LoadTexture(icon_giz_translate);
    icon_giz_scale_ui = GraphicsSystem::LoadTexture(icon_giz_scale);
    icon_giz_rotate_ui = GraphicsSystem::LoadTexture(icon_giz_rotate);
    icon_giz_resize_ui = GraphicsSystem::LoadTexture(icon_giz_resize);
    icon_active_ui = GraphicsSystem::LoadTexture(icon_active);
    icon_inactive_ui = GraphicsSystem::LoadTexture(icon_inactive);

    PopulateComponentList();
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

    if (!InputManager::KeyStateImgui)
        return;

    int savedColorDepth = 0;
    int savedStyleVarDepth = 0;
    int savedWindowDepth = 0;

    if (ImGuiContext* ctx = ImGui::GetCurrentContext())
    {
        savedColorDepth = ctx->ColorStack.Size;
        savedStyleVarDepth = ctx->StyleVarStack.Size;
        savedWindowDepth = ctx->CurrentWindowStack.Size;
    }

    try
    {
        if (InputManager::KeyStateImgui)
        {
            // KEPT HERE FOR OUR REFERENCE - BUT MOVED THIS TO CORE ALREADY
            // ImGui_ImplOpenGL3_NewFrame(); // Start new OpenGL frame
            // ImGui_ImplGlfw_NewFrame();    // Start new GLFW frame
            // ImGui::NewFrame();        // Start ImGui frame
            if (ImGui::IsKeyPressed(ImGuiKey_T) && ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
            {
                showTilemapEditor = !showTilemapEditor;
            }
            // Full window docking
            ImGuiViewport* viewport = ImGui::GetMainViewport();
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
    #ifndef _DEBUG
    #else
            TopMenuBar_Render();
    #endif

            LevelEditor* levelEditor = CORE->GetLevelEditor();
            bool isPlaying = (levelEditor && levelEditor->GetEditorMode() == EditorMode::Play);

            if (!isPlaying)
            {
                CORE->GetGraphicsSystem()->SetEditorMode(true);
                // GAME VIEWPORT - Always visible (outside conditional so it shows even in play mode)
                {
                    TileMap_Render(dt);                       // Rendering first so viewport shows first by default
                    RenderAnimationPreviewer();

                    ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f), ImGuiCond_FirstUseEver);
                    ImGui::SetNextWindowPos(ImVec2(50.0f, 50.0f), ImGuiCond_FirstUseEver);

                    ImGui::Begin("Game Viewport", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus);
                    viewportSize = ImGui::GetContentRegionAvail();                                   // Viewport checks how much space there is at sets it as size
                    CORE->GetGraphicsSystem()->RenderGameViewportImGui(viewportSize.x, viewportSize.y);

                    //ImGui::SetItemAllowOverlap();                                                           // to drag and click without moving window
                    ImGui::Image((void*)(intptr_t)CORE->GetGraphicsSystem()->GetFrameBufferTexture(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));           // Viewport 

                    // Render gizmos on viewport
                    if (!isTileMapFocused && !isAnimPreviewFocused)
                    {
                        // RENDER SECTION
                        Viewport_RenderGizmoToolbar();
                    }
                    // LOGIC SECTION
                    Viewport_ManageGizmos();

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

                    std::string draggedPrefab{};
                    // Viewport accepts Prefabs for drag and drop
                    ReceiveDragAndDrop(PrefabPayloadID, draggedPrefab);

                    if (!draggedPrefab.empty())
                    {
                        // filter for type of data here
                        //std::cout << draggedPrefab << std::endl;                         // Prints the data sent via drag and drop
                        //Serialization::LoadPrefab(draggedPrefab);

                       // Load prefab AND retrieve the root object created
                        GameObject* root = Serialization::LoadPrefabWithReturn(draggedPrefab);

                        if (root)
                        {
                            LevelEditor* editor = CORE->GetLevelEditor();
                            if (editor)
                                UndoTracker::RecordCreation(root, editor, factory);
                        }

                    }

                    // Viewport accepts Sprites for drag and drop
                    std::string draggedSprite{};
                    ReceiveDragAndDrop(SpritePayloadID, draggedSprite);

                    if (!draggedSprite.empty())
                    {
                        std::cout << "[Viewport] Dropped sprite: " << draggedSprite << std::endl;

                        // Extract sprite name from path
                        fs::path spritePath(draggedSprite);
                        std::string spriteName = spritePath.stem().string();

                        // Create GameObject
                        GameObject* newGO = factory.CreateGameObject(spriteName);

                        if (newGO)
                        {
                            float defaultVal{ 64.f };
                            vec2 size{ defaultVal, defaultVal };           // default
                            // Add Transform at cursor position
                            auto* transform = newGO->AddComponent<TransformComponent>();
                            // checks and adjustments below to let everything load before changing pos and scale

                            // Add Render with texture
                            auto* render = newGO->AddComponent<RenderComponent>();
                            if (render)
                            {
                                auto& resourceMgr = ResourceManager::Instance();
                                std::string textureKey = spriteName + "_texture";

                                if (resourceMgr.LoadTexture(textureKey, draggedSprite))
                                {
                                    TextureResource* texRes = resourceMgr.GetTexture(textureKey);
                                    if (texRes && texRes->IsLoaded())
                                    {
                                        render->SetTextureID(texRes->GetTextureID());
                                        render->SetTexturePath(draggedSprite);
                                        render->SetShapeType(RenderComponent::ShapeType::Square);
                                        render->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
                                    }
                                    size = render->GetTextureResolution(3);
                                    if (!size.x && !size.y)                                         // If invalid value found, set back to default value of 64,64. Else keep the aspect ratio of texture
                                        size = { defaultVal , defaultVal };
                                    //std::cout << "\nMIP LEVEL 3 res: " << size.x << ' ' << size.y << "\n";
                                }
                            }

                            if (transform)
                            {
                                transform->SetPosition(viewportCursorPos.x, viewportCursorPos.y);
                                transform->SetScale(size.x, size.y); // Default size
                            }

                            // Add Square Collider - CORRECTED CONSTRUCTOR CALL!
                            // Constructor: ColliderComponent(owner, type, width, height, layer)
                            newGO->AddComponent<ColliderComponent>(
                                ColliderComponent::Type::Square,
                                size.x,
                                size.y,
                                CollisionLayer::Default
                            );

                            // Select the new GameObject
                            SetSelectedGameObject(newGO->GetID());

                            LevelEditor* editor = CORE->GetLevelEditor();
                            if (editor)
                                UndoTracker::RecordCreation(newGO, editor, factory);

                            std::cout << "[Viewport] Created sprite GameObject: " << spriteName << std::endl;
                        }
                    }
                    // gets cursor pos WRT world pos
                    viewportPos = ImGui::GetItemRectMin();
                    viewportCursorPos = CursorToWorldPos(viewportPos, viewportSize);

                    ImGui::Text("(World Space) Hovering at: (%.1f, %.1f)", viewportCursorPos.x, viewportCursorPos.y);
                    ImGui::End();
                }

                // Only show editor UI when not in play mode
                Hierarchy_Render();
                AssetBrowserWindow.RenderImGui();

                // drawing highlight when a GO is selected
                if (selectedGameObjectID != 0)
                {
                    Viewport_HighlightGO();
                }

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

                // Render layer visibility panel
                LayerPanel_Render();

                Profiler_Render();
            }
            // if IS in play mode
            else
            {
                // toggle off other debug/ editor stuff.
                CORE->GetGraphicsSystem()->ClearEditorSelection();
            }

            if (CORE->GetLevelEditor()->GetEditorMode() != EditorMode::Play)
            {
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
            }

            // Flag with 1 frame delay on whether something is being drag and dropped, used for spawning prefabs via DND
            Serialization::Prefab_DragAndDropped = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
        }
    }
    catch (const std::exception& e)
    {
        RepairImGuiStack(savedColorDepth, savedStyleVarDepth, savedWindowDepth);
        g_editorErrorMessage = std::string("[Exception] ") + e.what();
        g_showEditorErrorPopup = true;
        Serialization::Prefab_DragAndDropped = false;

        std::cerr << "[ImGuiSystem] Caught std::exception: " << e.what() << "\n";
    }
    catch (...)
    {
        RepairImGuiStack(savedColorDepth, savedStyleVarDepth, savedWindowDepth);
        g_editorErrorMessage = "An unknown error occurred inside the editor update.";
        g_showEditorErrorPopup = true;
        Serialization::Prefab_DragAndDropped = false;

        std::cerr << "[ImGuiSystem] Caught unknown exception.\n";
    }

    // Must be called every frame after try/catch so the modal can open and stay visible
    RenderEditorErrorPopup();
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

ImVec2 CursorToWorldPos(ImVec2 vpPos, ImVec2 vpSize)
{
    ImVec2 cursorScreenPos = ImGui::GetMousePos();
    const float camZoom = CORE->GetLevelEditor()->GetCameraZoom();
    const vec2 camPos = { CORE->GetLevelEditor()->GetCameraX(), CORE->GetLevelEditor()->GetCameraY() };

    // --- THE FIX: Factor in the GraphicsSystem 1080p resolution scale ---
    // This calculates the hidden "zoom" that forces the 1920x1080 aspect ratio
    float resScale = std::min(vpSize.x / 1920.0f, vpSize.y / 1080.0f);
    if (resScale <= 0.0f) resScale = 1.0f; // Safety check

    // [from top left] local cursor pos - viewports position - viewportsize/2 (so we can have 0,0 be centered)
    float relX = cursorScreenPos.x - vpPos.x - (vpSize.x / 2.f);
    float relY = (vpSize.y / 2.f) - (cursorScreenPos.y - vpPos.y);

    // Evil function that took me like an hour to figure out and prolly took even more years off my lifespan
    // but basically the gist is
    // [from top left] local cursor pos - viewports position - viewportsize/2 (so we can have 0,0 be centered) / camerazoom + camera position
    // camera zoom represents how zoomed in we are, so cursor needs to scale off that. Camera pos is alr in world space so just add it onto!

    // Apply BOTH the resolution scale and the camera zoom to the relative mouse position!
    float worldx = (relX / resScale) / camZoom + camPos.x;
    float worldy = (relY / resScale) / camZoom + camPos.y;

    return ImVec2(worldx, worldy);
}

/**
 * @brief Manages logic for clicking to select GOs and DNDing to move them around
 */
void ImGuiSystem::Viewport_ClickToSelect()
{
    // Loop for all GOs in scene
    int highestLayer = std::numeric_limits<int>::min();
    GameObject* mostFrontGO = nullptr;
    for (auto& go : GameObjectFactory::Instance().GetAllGameObjects())
    {
        if (!go->IsActive()) continue;

        auto* transform = go->GetComponent<TransformComponent>();           // GOs with no transform will not be interactable
        if (!transform) continue;

        vec2 pos{ 0.f, 0.f }, scale{ 0.f, 0.f };                            // Get position and scale
        transform->GetPosition(pos.x, pos.y);
        transform->GetScale(scale.x, scale.y);

        // if GO has a collider
        // Calculates if cursor is within area of GO
        // Updated post M3 to fix issues with layering
        if (viewportCursorPos.x > pos.x - scale.x / 2 && viewportCursorPos.x < pos.x + scale.x / 2 && viewportCursorPos.y > pos.y - scale.y / 2 && viewportCursorPos.y < pos.y + scale.y / 2
            && !ImGuizmo::IsUsing())
        {
            int currLayer{};
            auto* render = go->GetComponent<RenderComponent>();
            if (render)
            {
                currLayer = render->GetLayer();
            }
            if (currLayer > highestLayer)
            {
                highestLayer = currLayer;
                mostFrontGO = go.get();
            }
        }
    }

    if (mostFrontGO && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))            // changed back to on mouse click, since we have gizmos, wont have accidental moving anymore
    {
        SetSelectedGameObject(mostFrontGO->GetID());                         // Set GO to be selected

        std::cout << "[has collider]clicked on " << mostFrontGO->GetName() << std::endl;
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

        ImGui::Separator();

       // Game Object Collider
        ImGui::Checkbox("Add Collider", &tempGO.addCollider);

        if (tempGO.addCollider)
        {
            ImGui::Indent();

            const char* colTypes[] = { "Square", "Circle" };
            ImGui::Combo("Type", &tempGO.colType, colTypes, IM_ARRAYSIZE(colTypes));

            ImGui::DragFloat2("Size", tempGO.colSize, 1.0f);

            const char* layers[] = { "Player", "Ground", "Enemy", "Platform", "Pickup", "Default"};
            ImGui::Combo("Layer", &tempGO.colLayer, layers, IM_ARRAYSIZE(layers));

            ImGui::Unindent();
        }

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
    if (ImGui::ImageButton("##Create GO Button", ImTextureID(icon_plus_ui), icon_size_small, {0, 1}, {1, 0},
                           (ImVec4(1.f, 1.f, 1.f, .7f)), (ImVec4(1, 1, 1, 1))))
    {
        // this is a file, not sure if we wanna do anything yet
        ShowCreateGOMenu = (!ShowCreateGOMenu) ? true : false;
    }
    CreateHintText("Create Game Object");

    ImGui::SameLine();
    // Render top part to show adding of Prefabs
    if (ImGui::ImageButton("##Spawn Prefab", ImTextureID(icon_prefab_ui), icon_size_small, {0, 1}, {1, 0},
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

    // dear god help us all
    if (!stressTestActive)
    {
        if (ImGui::Button("Run Stress Test"))
        {
            stressTestActive = true;
            SpawnStressObjects();   // your spawn function
        }
    }
    else
    {
        if (ImGui::Button("Delete Stress Test"))
        {
            stressTestActive = false;
            DestroyStressTestGameObjects();
        }
    }

    // Undo & Redo Feature
    LevelEditor* editor = CORE->GetLevelEditor();
    int undoCount = editor ? (int)editor->undoStack.size() : 0;
    int redoCount = editor ? (int)editor->redoStack.size() : 0;
    int cap = LevelEditor::GetUndoCapacity();

    bool canUndo = undoCount > 0;
    bool canRedo = redoCount > 0;

    std::string undoLabel = "Undo (" + std::to_string(undoCount) + "/" + std::to_string(cap) + ")";
    std::string redoLabel = "Redo (" + std::to_string(redoCount) + ")";

    if (!canUndo) ImGui::BeginDisabled();
    if (ImGui::Button(undoLabel.c_str()))
    {
        if (editor) editor->PerformUndo();
    }
    if (!canUndo) ImGui::EndDisabled();

    ImGui::SameLine();

    if (!canRedo) ImGui::BeginDisabled();
    if (ImGui::Button(redoLabel.c_str()))
    {
        if (editor) editor->PerformRedo();
    }
    if (!canRedo) ImGui::EndDisabled();

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

    // Expand parents of selected object so it is visible in the hierarchy
    if (hierarchyScrollToSelected && selectedGameObjectID != 0)
    {
        GameObject* sel = factory.GetGameObject(selectedGameObjectID);
        if (sel)
        {
            GameObject* parent = sel->GetParent();
            while (parent)
            {
                ImGuiStorage* storage = ImGui::GetStateStorage();
                storage->SetInt(ImGui::GetID((void*)(intptr_t)parent->GetID()), 1);
                parent = parent->GetParent();
            }
        }
    }

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
            ResetSelectedGameObject();
        }
    }

    ImVec2 spaceAvail = ImGui::GetContentRegionAvail();
    ImGui::Dummy(spaceAvail);
    if (spaceAvail.y > 0.f)
    {
        std::string draggedID{};
        ReceiveDragAndDrop(HierarchyPayloadID, draggedID);
        if (!draggedID.empty())
        {
            EntityID deliveredID = EntityID(std::stoi(draggedID));       // Convert payload data to EntityID
            GameObject* go = factory.GetGameObject(deliveredID);

            if (!go) return;
            GameObject* goParent = go->GetParent();
            if (goParent)
            {
                json beforeUnparentJson = Serialization::SerializeGameObject(go);
                goParent->RemoveChild(go);
                //LevelEditor* editor = CORE->GetLevelEditor();
                if (editor)
                    UndoTracker::RecordInstantChange(go, editor, beforeUnparentJson);
            }
        }
    }
    ImGui::End();

    // Press F to focus onto GO
    // TODO can make this like a nice panning if we want, but naw
    if (ImGui::IsKeyPressed(ImGuiKey_F) && selectedGameObjectID != 0)
    {
        // TODO set camera focus to that game object
        auto* transform = GetSelectedGameObject()->GetComponent<TransformComponent>();
        if (!transform) return;
        vec2 newPos{};
        float newZoom{ (viewportSize.x - transform->scaleX)/viewportSize.x };
        transform->GetPosition(newPos.x, newPos.y);
        CORE->GetLevelEditor()->SetCameraPos(newPos);
        CORE->GetLevelEditor()->SetCameraZoom(newZoom);
    }
}

void ImGuiSystem::Hierarchy_GameObjectTree_Renderer(GameObject *obj, int depth, EntityID &selectedGOID, GameObjectFactory &p_factory)
{
    // Just commented this out in case want to revert, but now we can show inactive GOs but greyed out
    //if (!obj->IsActive())
    //    return;
    if (!obj) return;
    EntityID id = obj->GetID();
    LevelEditor* editor = CORE->GetLevelEditor();
    //json propertyBeforeJson;

    // Set tree node flags
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (selectedGOID == id)
        flags |= ImGuiTreeNodeFlags_Selected;

    // Apply indentation based on depth
    ImGui::Indent(depth * 10.0f);
    bool nodeOpen{};

    // gotta do this for proper ids, was causing crashes
    std::string buttonName = "##Toggle Active" + std::to_string(int(id));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));             // remove bg for image [PUSH]
    if (obj->IsActive())
    {
        if (ImGui::ImageButton(buttonName.c_str(), ImTextureID(icon_active_ui), icon_size_smallest, {0, 1}, {1, 0},
            ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            obj->SetActive(false);
        }
    }
    else
    {
        if (ImGui::ImageButton(buttonName.c_str(), ImTextureID(icon_inactive_ui), icon_size_smallest, { 0, 1 }, { 1, 0 },
            ImVec4(0, 0, 0, 0), ImVec4(1, 1, 1, 1)))
        {
            obj->SetActive(true);
        }
    }
    ImGui::PopStyleColor();                                                 // remove bg for image [POP]
    ImGui::SameLine();

    // Grey out text if inactive [PUSH]
    if (!obj->IsActive())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
    }

    ImGui::AlignTextToFramePadding();
    // if renaming, set the name of the tree node as an input text field
    if (hierarchy_renameIndex == (int)id)
    {
        ImGui::PushID(id);
        ImGuiTreeNodeFlags nameFlags = flags | ImGuiTreeNodeFlags_Leaf;

        nodeOpen = ImGui::TreeNodeEx("##RenamingNode", nameFlags);
        ImGui::SameLine();
        if (hierarchy_renameFocus)
        {
            ImGui::SetKeyboardFocusHere();
            hierarchy_renameFocus = false;
        }
        if (ImGui::InputText("##Renaming", rename_str, IM_ARRAYSIZE(rename_str), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            propertyBeforeJson = Serialization::SerializeGameObject(obj);
            GetSelectedGameObject()->SetName(std::string(rename_str));
            UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
            hierarchy_renameIndex = -1;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter))
        {
            GetSelectedGameObject()->SetName(std::string(rename_str));
            hierarchy_renameIndex = -1;
        }

        ImGui::PopID();
    }
    else
    {
        // Display the tree node
        nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)id, flags, "%s (ID: %u)", obj->GetName().c_str(), id);
    }

    // Grey out text if inactive [POP]
    if (!obj->IsActive())
    {
        ImGui::PopStyleColor();
    }

    // Scroll hierarchy to show this item when it was just selected
    if (selectedGOID == id && hierarchyScrollToSelected)
    {
        ImGui::SetScrollHereY(0.5f);
        hierarchyScrollToSelected = false;
    }

    if (!ImGui::IsItemFocused() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        hierarchy_renameIndex = -1;
    }

    // If has children, make it expandable
    if (obj->GetAllChildren().empty())
    {
        flags |= ImGuiTreeNodeFlags_Leaf;
    }

    if (selectedGOID == id)
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // Drag and drop for Parenting
    if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        SendDragAndDrop(HierarchyPayloadID, obj->GetID(), obj->GetName());
    }

    {
        // receiving drag and drop
        std::string draggedID{};
        ReceiveDragAndDrop(HierarchyPayloadID, draggedID);
        // Do something with DnD data
        if (!draggedID.empty())
        {
            std::vector<EntityID> familyTree{};
            GetFamilyTree(obj, familyTree);                         // Get all descendent game objects (child of child wtv) underneath the parrent GO

            /*std::cout <<  obj->GetName() <<"'s GetFamilyTree() has: \n";
            for (auto const &r : familyTree)
            {
                std::cout << p_factory.GetGameObject(r)->GetName() << std::endl;
            }*/

            EntityID deliveredID = EntityID(std::stoi(draggedID));       // Convert payload data to EntityID
            auto check = std::find(familyTree.begin(), familyTree.end(), deliveredID);

            std::vector<EntityID> selfParentingTree{};
            GetFamilyTree(p_factory.GetGameObject(deliveredID), selfParentingTree);
            auto checkSelfParenting = std::find(selfParentingTree.begin(), selfParentingTree.end(), obj->GetID());

            if (check == familyTree.end() && checkSelfParenting == selfParentingTree.end())
            {
                GameObject *deliveredGO = p_factory.GetGameObject(deliveredID);

                propertyBeforeJson = Serialization::SerializeGameObject(deliveredGO);

                std::cout << "[Hierarchy] Setting " << deliveredGO->GetName() << " to be child of " << obj->GetName() << std::endl;
                deliveredGO->SetParent(obj);                                        // Set Dragged GO to be parent of receiver GO

                UndoTracker::RecordInstantChange(deliveredGO, editor, propertyBeforeJson);

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
        SetSelectedGameObject(id);
    }

    EntityID deleteID{0};                                                        // fast check if game object should be deleted
    if (isHoveringOverViewport && selectedGameObjectID != 0 && ImGui::IsKeyPressed(ImGuiKey_Delete))
        deleteID = selectedGameObjectID;

    // Right Click event (for specific GO in hierarchy)
    if (ImGui::BeginPopupContextItem())
    {
        SetSelectedGameObject(id);                                          // right click auto selects them, but sure let them click select also 
        if (ImGui::MenuItem("Select"))
        {
            SetSelectedGameObject(id);
        }
        if (ImGui::MenuItem("Rename"))
        {
            // rename shiz here
            hierarchy_renameIndex = id;
            strncpy(rename_str, GetSelectedGameObject()->GetName().c_str(), sizeof(hierarchy_rename_str));
            prefab_oldname = GetSelectedGameObject()->GetName();
            hierarchy_renameFocus = true;
        }
        if (ImGui::MenuItem("Save as Prefab"))
        {
            Serialization::SavePrefab(*obj);
        }
        if (ImGui::MenuItem("Clone"))
        {
            GameObject* clone = p_factory.CloneGameObject(id);

            if (clone)
            {               
                if (clone)
                    UndoTracker::RecordCreation(clone, editor, p_factory);
            }
        }
        // find way to unparent by drag and drop in the future
        if (obj->GetParent() && ImGui::MenuItem("Make Root"))
        {
            propertyBeforeJson = Serialization::SerializeGameObject(obj);

            obj->GetParent()->RemoveChild(obj);

            UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
        }

        if (ImGui::MenuItem("Delete"))
            deleteID = id;

        ImGui::EndPopup();
    }  

    if (deleteID)
    {
        GameObject* gameObj = factory.GetGameObject(deleteID);

        // TODO - block out the deleting of main camera - crashes the game
        if (!gameObj) return;
        if (factory.GetGameObject(deleteID)->GetName() == "MainCamera") return;
        else 
        {            
            if (gameObj) {
                UndoTracker::RecordDeletion(gameObj, CORE->GetLevelEditor(), factory);

                factory.DestroyAllRelatedGameObjects(deleteID);

                if (selectedGameObjectID == deleteID)  ResetSelectedGameObject();
            }                 
        }
    }

    // Render all children of opened parent GO
    if (nodeOpen)
    {
        // repeats loop again
        for (auto &childPtr : obj->GetAllChildren())
        {
            Hierarchy_GameObjectTree_Renderer(childPtr, depth+1, selectedGOID, p_factory);
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
    float colliderSize[2] = { tempGO.scale[0], tempGO.scale[1] };
    ColliderComponent::Type cType = (tempGO.colType == 0) ? ColliderComponent::Type::Square : ColliderComponent::Type::Circle;
    CollisionLayer cLayer = static_cast<CollisionLayer>(tempGO.colLayer);

    // Construct message and send
    CreateObjectMessage msg(
        name,                   // GameObject name
        objType,                // Shape if is Empty or Basic
        shapeType,              // ShapeType
        pos, scale, rot,        // Transform
        color,               // Colour
        colliderSize,
        tempGO.addCollider,
        cType,
        cLayer
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
    if (selectedGameObjectID != id)
    {
        // Cancel any active ImGui widget (e.g. a DragFloat that is mid-drag).
        // Without this, the accumulated drag delta carries over and gets applied
        // to the newly selected object's transform on the next frame.
        ImGui::ClearActiveID();
    }
    selectedGameObjectID = id;
    hierarchyScrollToSelected = true;
    CORE->GetGraphicsSystem()->ClearEditorSelection();              // need to clear after resassigning so newly selected GO can have the correct outlines drawn
}

void ImGuiSystem::ResetSelectedGameObject()
{
    selectedGameObjectID = 0;
    CORE->GetGraphicsSystem()->ClearEditorSelection();
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
void ImGuiSystem::PrefabMenu_Render()
{
    ImGui::Begin("Prefab Menu", &ShowLoadPrefabMenu);

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

                    if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyPressed(ImGuiKey_Enter))
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
    ImGui::Text("[WIP] Preview Render:");
    ImGui::Dummy(boxSize); // Reserves space for preview

    ImDrawList* drawList = ImGui::GetWindowDrawList(); // Imgui rendering for box with white outline
    drawList->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 255));
    ImGui::EndGroup();


    if (ImGui::Button("Spawn -> Load Prefab"))
    {
        PrefabMenu_Spawn();                     // Spawn selected prefab as GO
    }
    CreateHintText("Spawn prefab as Game Object in scene.");

    ImGui::Dummy({30, 30}); // Reserves space for preview
    std::string dispText;
    bool bothSelected{ selectedGameObjectID && current_prefab };
    if (!selectedGameObjectID && !current_prefab)
    {
        dispText = "Select GO and prefab to override";
    }
    else
    {
        std::string goName{}, prefabName{};
        //if (selectedGameObjectID) { goName = factory.GetGameObject(selectedGameObjectID)->GetName(); }
        //else goName = "_____";

        if (selectedGameObjectID)
        {
            GameObject* selectedGO = factory.GetGameObject(selectedGameObjectID);
            goName = selectedGO ? selectedGO->GetName() : "_____";  // safe even if object was deleted
        }
        else goName = "_____";

        
        if (current_prefab) { prefabName = prefabList[current_prefab]; }
            else prefabName = "_____";

        dispText = "Override " + prefabName + " with " + goName;
    }
    // looks kinda ugly but it makes it more obvious
    if (!bothSelected)
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), dispText.c_str());
    else
        ImGui::Text(dispText.c_str());

    if (!bothSelected)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 0, 0, 1)),
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 0.3f, 0.3f, 1)),
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0, 0, 1));
    }
    if (ImGui::Button("Override"))
    {
        // takes the selected GO and replace the selected prefab with all the GO's data
        if (bothSelected)
        {
            PrefabMenu_Override();
        }
        else
        {
            std::cout << "Select GO in Scene + prefab in Prefab Menu!\n";
        }
    }
    CreateHintText("Override an existing prefab with a selected Game Object.");
    ImGui::SameLine();
    if (ImGui::Button("Update all instances"))
    {
        if (bothSelected)
        {
            // tweaked to work with spawned prefab objects or any game object
            // Gets selected prefab/ game object, replaces ALL newly spawned prefabs to be of that newly edited game prefab/ game object
            PrefabMenu_Reinstantiate();
        }
        else
        {
            std::cout << "Select GO in Scene + prefab in Prefab Menu!\n";
        }
    }
    CreateHintText("Update all newly created prefabs of selected type.");
    if (!bothSelected)
    {
        ImGui::PopStyleColor(3);
    }

    ImGui::End(); // End of Prefab Search Menu
}

/**
 * @brief Override existing prefab with edited prefab. Tweaked to also work for any GO in scene.
 */
void ImGuiSystem::PrefabMenu_Override()
{
    auto* selectedGO = factory.GetGameObject(selectedGameObjectID);
    if (!selectedGO) return;
    std::string selectedPrefabName = prefabList[current_prefab];

    // if same name, simply override 
    Serialization::SavePrefab(*selectedGO);
    if (selectedGO->GetName() != selectedPrefabName)                     // only delete prefab if names are different, if names are the same it gets overriden automatically
    {
        Serialization::RemovePrefab(selectedPrefabName);
        Serialization::RenamePrefab(selectedGO->GetName(), selectedPrefabName);
    }
}


/**
 * @brief Reinstantiated all newly spawned prefabs to be as the selected GO in scene.
 */
void ImGuiSystem::PrefabMenu_Reinstantiate()
{
    // If want to tweak for it to replace with only newly added prefabs, just do a check if the selectedGO is also in prefabInstances.
    // Tried to only replace components so we can keep the same entity IDs, but parenting is not a component and it causes issues. So we just delete and spawn again at specified location.

    Serialization::Prefab_Reinstantiate = true;                             // flag for specifying location of where to spawn prefab (Prefab_Reinstantiate_Pos)
    auto* selectedGO = factory.GetGameObject(selectedGameObjectID);
    std::string selectedPrefabName = prefabList[current_prefab];
    selectedGO->SetName(selectedPrefabName);                                // force set the name to be of selected prefab

    Serialization::SavePrefab(*selectedGO);                                 // Properly overrides the old prefab

    std::vector<EntityID> prefabReiterationList = prefabInstances[selectedPrefabName];          // make a copy because we will be deleting things from the active list

    for (EntityID p : prefabReiterationList)
    {
        GameObject* currPrefab = factory.GetGameObject(p);                                      // get GO
        if (!currPrefab || !currPrefab->GetComponent<TransformComponent>())
        {
            std::cout << "Could not find GO id: " << p << " or its transform. Skipping...\n";           // print if unable to modify GO, skip as well
            continue;
        }

        currPrefab->GetComponent<TransformComponent>()->GetPosition(Prefab_Reinstantiate_Pos.x, Prefab_Reinstantiate_Pos.y);            // get position for spawning of new prefab (handled in loadprefab)
        factory.DestroyGameObject(p);                                                               // destroy old GO and spawn new GO
        Serialization::LoadPrefab(selectedPrefabName);
    }
    Serialization::Prefab_Reinstantiate = false;                                // reset flag to false
}


/**
 * @brief Ctor - Spawns the selected prefab as a game object in scene
 */
void ImGuiSystem::PrefabMenu_Spawn()
{
    Serialization::LoadPrefab(prefabList[current_prefab]);          // Deserialises the selected prefab
}
#pragma endregion

#pragma region Viewport/ Gizmos
void ImGuiSystem::Viewport_ManageGizmos()
{
    if (selectedGameObjectID != 0 && gizmo_type != -1)
    {
        auto* go = GetSelectedGameObject();

        if (lockedGizmosGOs.contains(go->GetID()))
        {
            // draw locked gizmo sign
            //ImGuizmo::Enable(false);
        }

        auto* transform = go->GetComponent<TransformComponent>();
        auto* camComp = CameraManager::Instance().GetActiveCamera();
        if (!camComp)
        {
            std::cout << "[ManageGizmos]NO CAMERA FOUND!\n";
            return;
        }
        if (!transform)
        {
            std::cout << "[ManageGizmos]NO TRANSFORM FOUND!\n";
            return;
        }

        // Setup Viewport Data
        ImVec2 vpSize{ ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y };
        ImVec2 vpPos{ ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMin().x, ImGui::GetWindowPos().y + ImGui::GetWindowContentRegionMin().y };

        ImGuizmo::SetOrthographic(true);        // or false idk
        ImGuizmo::SetDrawlist();                                                                                                            // draws current window
        ImGuizmo::SetRect(vpPos.x, vpPos.y, vpSize.x, vpSize.y);             // set rect to viewport

        //mat4 view = camComp->GetViewMatrixUnshaken();
        mat4 view = mat4::identity();

        //mat4 projection = camComp->GetProjectionMatrix(ImGui::GetWindowWidth(), ImGui::GetWindowHeight());                                // replaced with getVp() bc this doesnt scale with cam zoom etc
        mat4 mtx = transform->GetMatrix();

        // Record Undo/Redo Entries
        UndoTracker gizmoTracker(go, CORE->GetLevelEditor(), isEditingProperty, propertyBeforeJson, ImGuizmo::IsUsing());

        // Manipulate
        ImGuizmo::Manipulate(value_ptr(view), value_ptr(CORE->GetGraphicsSystem()->getVp()), ImGuizmo::OPERATION(gizmo_type), ImGuizmo::WORLD, value_ptr(mtx));

        if (gizmo_type == (int)(ImGuizmo::OPERATION::BOUNDS))
        {
            float* deltaMat{};                                                  // we not gonna use this for anything, just a filler ptr so we can access bounds
            float bounds[6] = { -0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f };         // plots the cross section so we can get the middle edges
            float windowWidth = static_cast<float>(CORE->GetGraphicsSystem()->GetWindowWidth());
            float windowHeight = static_cast<float>(CORE->GetGraphicsSystem()->GetWindowHeight());
            ImGuizmo::Manipulate(
                value_ptr(view),
                value_ptr(CameraManager::Instance().GetViewProjectMatrix(windowWidth, windowHeight)),
                ImGuizmo::OPERATION::BOUNDS,
                ImGuizmo::WORLD,
                value_ptr(mtx),
                deltaMat,
                deltaMat,
                bounds
            ); // manipulate with bounds in mind
        }

        if (ImGuizmo::IsUsing())
        {
            vec3 trans{}, scale{}, rot{};
            DecomposeMatrix(mtx, trans.x, trans.y, scale.x, scale.y, rot.x);

            //std::cout << "Decomposed rot: " << rot.x << " " << rot.y << " " << rot.z << "\n";
            // debugging the c# anim image flipping for some reason AAAAAAAAAAAAAAAAAAAAAAA

            // clamps so stuff doesnt get out of hand - only on gizmos bc its likely to be an accident here, will not check for clamps if edited via editor slider
            scale.x = (scale.x < 1.f) ? 1.f : scale.x;
            scale.y = (scale.y < 1.f) ? 1.f : scale.y;

            transform->SetPosition(trans.x, trans.y);
            transform->SetScale(scale.x, scale.y);
            transform->SetRotation(rot.x);
        }
    }
}

void ImGuiSystem::Viewport_RenderGizmoToolbar()
{
    // On screen GUI settings
    viewportPos = ImGui::GetWindowPos();
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowBgAlpha(0.75f);
    ImGui::SetNextWindowPos(ImVec2(viewportPos.x + 15, viewportPos.y + 35), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(58.0f, 260.0f), ImGuiCond_FirstUseEver);

    ImGui::Begin("##Toolbar", nullptr, flags);

    if (ImGui::ImageButton("##Gizmo_Select", ImTextureID(icon_giz_select_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == -1) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_Q) && isHoveringOverViewport)
    {
        gizmo_type = -1;
    }
    CreateHintText("(Q) Select");
    ImGui::Separator();
    if (ImGui::ImageButton("##Gizmo_Translate", ImTextureID(icon_giz_translate_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == int(ImGuizmo::OPERATION::TRANSLATE)) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_W) && isHoveringOverViewport)
    {
        gizmo_type = int(ImGuizmo::OPERATION::TRANSLATE);
    }
    CreateHintText("(W) Move");
    ImGui::Separator();
    if (ImGui::ImageButton("##Gizmo_Scale", ImTextureID(icon_giz_scale_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == int(ImGuizmo::OPERATION::SCALE)) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_E) && isHoveringOverViewport)
    {
        gizmo_type = int(ImGuizmo::OPERATION::SCALE);
    }
    CreateHintText("(E) Scale");
    ImGui::Separator();
    if (ImGui::ImageButton("##Gizmo_Rotate", ImTextureID(icon_giz_rotate_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == int(ImGuizmo::OPERATION::ROTATE)) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_R) && isHoveringOverViewport)
    {
        gizmo_type = int(ImGuizmo::OPERATION::ROTATE);
    }
    CreateHintText("(R) Rotate");
    ImGui::Separator();
    if (ImGui::ImageButton("##Gizmo_Resize", ImTextureID(icon_giz_resize_ui), icon_size_medium, { 0, 1 }, { 1, 0 },
        (gizmo_type == int(ImGuizmo::OPERATION::BOUNDS)) ? color_icon_highlighted : color_icon_normal, color_icon_bg) || ImGui::IsKeyPressed(ImGuiKey_T) && isHoveringOverViewport)
    {
        gizmo_type = int(ImGuizmo::OPERATION::BOUNDS);
    }
    CreateHintText("(T) Resize");

    ImGui::End();
}

void ImGuiSystem::Viewport_HighlightGO()
{
    // TODO WIP FOR HIGHLIGHTING SELECTED GO, I GOT SO LOST IN THE SAUCE LMAO ITS ALMOST DONE THO I JUST DK HOW TO GET VP MATRIX
    //if (!selectedGameObjectID) return;
    auto* go = GameObjectFactory::Instance().GetGameObject(selectedGameObjectID);
    if (!go) return;
    auto* transform = go->GetComponent<TransformComponent>();
    auto* collider = go->GetComponent<ColliderComponent>();
    //auto* transform = GameObjectFactory::Instance().GetGameObject(selectedGameObjectID)->GetComponent<TransformComponent>();
    //auto* collider = GameObjectFactory::Instance().GetGameObject(selectedGameObjectID)->GetComponent<ColliderComponent>();

    if (transform)
    {
        float x, y, sx, sy;

        transform->GetPosition(x, y);
        transform->GetScale(sx, sy);
        if (CORE->GetGraphicsSystem())
        {
            CORE->GetGraphicsSystem()->SetEditorSelection(x, y, sx, sy);
        }
        if (collider)
        {
            vec2 colliderScale{ collider->GetSize() };

            if (CORE->GetGraphicsSystem())
            {
                CORE->GetGraphicsSystem()->SetEditorSelection_ColliderOnly(x, y, colliderScale.x, colliderScale.y);
            }
        }
    }

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

    //sorts alphabetically
    std::vector<fs::directory_entry> entries;
    for (auto& iter : fs::directory_iterator(currentDir))
        entries.push_back(iter);

    std::sort(entries.begin(), entries.end(), [](fs::directory_entry const& lhs, fs::directory_entry const& rhs)
    {
        bool aIsDir{ lhs.is_directory() }, bIsDir{ rhs.is_directory() };
        if (aIsDir != bIsDir)
            return aIsDir > bIsDir;                                             // prioritise folders, compare bools
        return lhs.path().filename().string() < rhs.path().filename().string(); // alphabet, compare strings
    }); 

    // For each item in the current folder
    for (auto &iter : entries)
    {
        std::string path = iter.path().string();                            // Path we are currently in
        auto relativePath = fs::relative(iter.path(), assetPath);           // Path relative from where we started (assets) to where we are
        std::string fileName = relativePath.filename().string();            // name of the file alone
        std::string extension = iter.path().extension().string();

        // If the current item is a folder
        if (iter.is_directory())
        {
            // TODO add double click? or is single click fine
            if (ImGui::ImageButton(fileName.c_str(), (ImTextureID)(uintptr_t)(icon_folder_ui), icon_size, {0, 1}, {1, 0}))     // Draw image button with folder icon
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
            // Sprite Files
            else if (extension == ".png" || extension == ".jpg")
            {
                if (ImGui::ImageButton(fileName.c_str(), ImTextureID(icon_sprite_ui), icon_size, { 0, 1 }, { 1, 0 }))
                {
                    // do nothin for now
                }
            }
            // Generic file
            else
            {
                if (ImGui::ImageButton(fileName.c_str(), ImTextureID(icon_file_ui), icon_size, { 0, 1 }, { 1, 0 }))
                {
                    // do nothin for now
                }
            }

            // Can only drag and drop files, not folders
            if (ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            {
                // Depending on folder its in, send the respective payloadIDs to receive
                // changed from string() to generic_string() because of weird issues with front n back slashes
                if (iter.path().parent_path().filename() == "Scenes")
                {
                    SendDragAndDrop(ScenePayloadID, iter.path().generic_string(), fileName.c_str());            // For scenes files
                }
                else if (iter.path().parent_path().filename() == "Prefabs")
                {
                    SendDragAndDrop(PrefabPayloadID, iter.path().stem().generic_string(), fileName);            // For prefab files
                }
                else if (iter.path().parent_path().filename() == "Audio" || iter.path().parent_path().filename() == "audio"
                    || extension == ".wav" || extension == ".mp3" || extension == ".ogg" || extension == ".flac")
                {
                    SendDragAndDrop(AudioPayloadID, iter.path().generic_string(), fileName.c_str());            // For Audio files
                }
                else if (iter.path().parent_path().filename() == "Sprites" || iter.path().parent_path().filename() == "Icons" || iter.path().parent_path().filename() == "Tiles"
                    || extension == ".png" || extension == ".jpg")
                {
                    SendDragAndDrop(SpritePayloadID, iter.path().generic_string(), fileName.c_str());            // For Sprites/ Spritesheets files
                }
                else if (extension == ".cs")
                {
                    SendDragAndDrop(ScriptPayloadID, iter.path().generic_string(), fileName.c_str());
                }
                else if (extension == ".ttf")
                {
                    SendDragAndDrop(FontPayloadID, iter.path().generic_string(), fileName.c_str());
                }
                else
                {
                    SendDragAndDrop(AssetPayloadID, iter.path().generic_string(), fileName.c_str());            // Anything else considered an general asset
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
            if (ImGui::MenuItem("[WIP]"))
            {
            }
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
            if (ImGui::MenuItem("[WIP]"))
            {
            }
            if (ImGui::MenuItem("Preferences"))
            {
                // key binds? Rebinding keys for shortcuts?
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

            ImGui::Separator();  // Add a separator line

            ImGui::MenuItem("Tilemap Editor", "Ctrl+T", &showTilemapEditor);

            ImGui::EndMenu();
        }

        // Help menu
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("[WIP]"))
            {
            }
            if (ImGui::MenuItem("Naw"))
            {
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    // POP UP CALLS HERE
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
            if (ImGui::ImageButton(layoutFileName.c_str(), (ImTextureID)(uintptr_t)(icon_file_ui), icon_size, {0, 1}, {1, 0}))
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

    if (!ImGui::Begin("Properties##ImGuiSystem", nullptr, ImGuiWindowFlags_NoCollapse))     // changed to remove the close window (to add back, change nullptr to priv mem showpropertyeditor)
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

    //static bool isEditingProperty = false;
    //static json propertyBeforeJson;
    LevelEditor* editor = CORE->GetLevelEditor();

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

    UndoTracker activeTracker(obj, editor, isEditingProperty, propertyBeforeJson);

    ImGui::Spacing();
    // this is done, just need to attach to a component? so it'll belong to a specific component where users can drag and drop stuff
    //ps.RenderImGui();

    // GO transform screen
    if (auto* transform = obj->GetComponent<TransformComponent>())
    {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool isTransformEnabled = transform->IsEnabled();
                if (ImGui::Checkbox("Component Enabled ##Transform", &isTransformEnabled))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj); // BEFORE change
                    transform->SetEnabled(isTransformEnabled);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }
  
                // sliders for position, scale, rotation etc
                bool isGizmoLocked = lockedGizmosGOs.contains(obj->GetID());
                if (ImGui::Checkbox("Lock Gizmos ##Transform", &isGizmoLocked))
                {
                    if (isGizmoLocked)
                        lockedGizmosGOs.insert(obj->GetID());
                    else
                        lockedGizmosGOs.erase(obj->GetID());
                }
                ImGuizmo::Enable(!isGizmoLocked);

                float pos[2] = { 0.0f, 0.0f };
                transform->GetPosition(pos[0], pos[1]);
                if (ImGui::DragFloat2("Position", pos, 0.5f))
                {
                    transform->SetPosition(pos[0], pos[1]);

                }               
                UndoTracker posTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                float scale[2] = { 1.0f, 1.0f };
                transform->GetScale(scale[0], scale[1]);
                if (ImGui::DragFloat2("Scale", scale, 0.5f))
                {
                    transform->SetScale(scale[0], scale[1]);

                    // Sync collider
                    auto* collider = obj->GetComponent<ColliderComponent>();
                    if (collider) {
                        collider->SetSize(scale[0], scale[1]);
                    }
                }
                UndoTracker scaleTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                float rotationDeg = transform->GetRotation() * (180.0f / pi);
                if (ImGui::DragFloat("Rotation (deg)", &rotationDeg, 0.5f, -360.0f, 360.0f))
                {
                    transform->SetRotation(rotationDeg * (pi / 180.0f));
                }
                UndoTracker rotTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                if (transform->GetPropertySlots().size())
                {
                    for (auto& slot : transform->GetPropertySlots())
                    {
                        slot.RenderImGui();
                        //UndoTracker slotTracker(obj, editor, isEditingProperty, propertyBeforeJson);
                    }
                }
        }
    }

    // Render component for GO
    if (auto* render = obj->GetComponent<RenderComponent>())
    {
        if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Remove button aligned right
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            float buttonWidth = 100.0f;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - 10);
            bool renderRemove = ImGui::Button("Remove##Render", ImVec2(buttonWidth, 22));
            ImGui::PopStyleColor(3);
            if (renderRemove)
            {
                propertyBeforeJson = Serialization::SerializeGameObject(obj);
                obj->RemoveComponent("RenderComponent");
                UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
            }
            else
            {
                bool isRenderEnabled = render->IsEnabled();
                if (ImGui::Checkbox("Component Enabled ##Render", &isRenderEnabled))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    render->SetEnabled(isRenderEnabled);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }

                ImGui::Text("Shape Type: %s", GetShapeTypeName((int)render->GetShapeType()));
                static bool showTextureDND = false;

                // Show colour picker for GO colour (only if no texture id for custom texture/ sprite)
                if (render->GetTextureID() == 0)
                {
                    vec4 color = render->GetColor();
                    float colorArray[4] = { color.x, color.y, color.z, color.w };
                    if (ImGui::ColorEdit4("Color", colorArray))
                    {
                        render->SetColor(vec4(colorArray[0], colorArray[1], colorArray[2], colorArray[3]));
                    }
                    UndoTracker colorTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                    if (showTextureDND)
                    {
                        std::string draggedSprite{};
                        ImGui::InputText("##DragImageHere", draggedSprite_str, sizeof(draggedSprite_str));
                        if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
                        {
                            draggedSprite = draggedSprite_str;
                        }

                        ReceiveDragAndDrop(SpritePayloadID, draggedSprite);
                        if (!draggedSprite.empty())
                        {
                            propertyBeforeJson = Serialization::SerializeGameObject(obj);

                            std::cout << draggedSprite << std::endl;
                            render->SetTexturePath(draggedSprite);

                            GLuint oldTexID = render->GetTextureID();               // get old tex before overwriting
                            GLuint newTexID = CORE->GetGraphicsSystem()->LoadTexture(draggedSprite.c_str());

                            if (newTexID)
                            {
                                if (oldTexID) glDeleteTextures(1, &oldTexID);           // free the old one first
                                render->SetColor(1.0f);                                 // removes colour and makes way for loading image
                                render->SetTextureID(newTexID);
                                strcpy(draggedSprite_str, "<Drag Image Here>");                   // reset
                                std::cout << "[Properties] New Texture loaded successfully!" << std::endl;

                                UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                            }
                            else
                            {
                                std::cout << "[Properties] Failed to load texture!" << std::endl;
                            }
                        }
                    }

                    // placed below here for cleaner transition when pressing the button
                    if (!showTextureDND && ImGui::Button("Add Texture"))
                    {
                        render->SetTextureID(GLuint{});
                        showTextureDND = true;
                    }
                }
                else
                {
                    ImGui::Text("Texture ID: %u", render->GetTextureID());
                    showTextureDND = false;
                    if (ImGui::Button("Remove Existing Texture"))
                    {
                        propertyBeforeJson = Serialization::SerializeGameObject(obj);
                        render->SetTextureID(GLuint{});
                        render->SetColor({ 1,0,1,1 });                    // set to pink to make it obvious if texture missing
                        UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                    }
                }
                int goRenderLayer{ render->GetLayer() };
                ImGui::Text("Rendering Layer");
                ImGui::SetNextItemWidth(100.0f);
                if (ImGui::InputInt("##Rendering Layer", &goRenderLayer))
                {
                    render->SetLayer(goRenderLayer);
                }
                UndoTracker layerTracker(obj, editor, isEditingProperty, propertyBeforeJson);
                CreateHintText("Ascending order for topmost rendering position");

                // --- Screen Space ---
                bool rcSS = render->isScreenSpace();
                if (ImGui::Checkbox("Screen Space##Render", &rcSS))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    render->SetScreenSpace(rcSS);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }
                CreateHintText("Screen Space: object stays fixed on screen. World Space: object follows camera.");

            }
        }
    }

    // Text component
    if (auto* tc = obj->GetComponent<TextComponent>())
    {
        if (ImGui::CollapsingHeader("Text", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Remove button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            float textBtnW = 100.0f;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - textBtnW - 10);
            bool textRemove = ImGui::Button("Remove##Text", ImVec2(textBtnW, 22));
            ImGui::PopStyleColor(3);

            if (textRemove)
            {
                propertyBeforeJson = Serialization::SerializeGameObject(obj);
                obj->RemoveComponent("TextComponent");
                UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
            }
            else
            {
                // --- Enabled ---
                bool isTextEnabled = tc->IsEnabled();
                if (ImGui::Checkbox("Component Enabled ##Text", &isTextEnabled))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    tc->SetEnabled(isTextEnabled);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }

                // --- Text content ---
                static char textContentBuf[512]{};
                strncpy_s(textContentBuf, tc->GetText().c_str(), sizeof(textContentBuf) - 1);
                if (ImGui::InputText("Text Content", textContentBuf, sizeof(textContentBuf)))
                {
                    tc->SetText(textContentBuf);
                }
                UndoTracker textContentTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                // --- Font path (drag .ttf from Project Navigator or type manually) ---
                ImGui::Separator();
                ImGui::Text("Font (.ttf only)");
                static char fontPathBuf[512]{};
                strncpy_s(fontPathBuf, tc->GetFontPath().c_str(), sizeof(fontPathBuf) - 1);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 10.0f);
                if (ImGui::InputText("##FontPath", fontPathBuf, sizeof(fontPathBuf), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    std::string p = fontPathBuf;
                    if (p.size() >= 4 && p.substr(p.size() - 4) == ".ttf")
                    {
                        propertyBeforeJson = Serialization::SerializeGameObject(obj);
                        tc->SetFont(fontPathBuf, tc->GetFontSize());
                        UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                    }
                    else
                    {
                        std::cout << "[TextComponent] Only .ttf files are accepted.\n";
                    }
                }
                // Drag-and-drop target — only accepts FontPayloadID (.ttf)
                {
                    std::string droppedFont{};
                    ReceiveDragAndDrop(FontPayloadID, droppedFont);
                    if (!droppedFont.empty())
                    {
                        propertyBeforeJson = Serialization::SerializeGameObject(obj);
                        tc->SetFont(droppedFont.c_str(), tc->GetFontSize());
                        UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                    }
                }
                CreateHintText("Type a path and press Enter, or drag a .ttf from the Project Navigator");

                // --- Font size ---
                int fontSizeVal = static_cast<int>(tc->GetFontSize());
                ImGui::SetNextItemWidth(120.0f);
                if (ImGui::InputInt("Font Size", &fontSizeVal))
                {
                    if (fontSizeVal < 1) fontSizeVal = 1;
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    tc->SetFont(tc->GetFontPath().c_str(), static_cast<GLuint>(fontSizeVal));
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }

                ImGui::Separator();

                // --- Color ---
                vec4 tcCol = tc->GetColor();
                float tcColorArr[4] = { tcCol.x, tcCol.y, tcCol.z, tcCol.w };
                if (ImGui::ColorEdit4("Text Color", tcColorArr))
                {
                    tc->SetColor(vec4(tcColorArr[0], tcColorArr[1], tcColorArr[2], tcColorArr[3]));
                }
                UndoTracker tcColorTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                // --- Scale ---
                float tcScale = tc->GetScale();
                if (ImGui::DragFloat("Scale##Text", &tcScale, 0.01f, 0.0f, 100.0f))
                {
                    tc->SetScale(tcScale);
                }
                UndoTracker tcScaleTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                // --- Screen Space ---
                bool tcSS = tc->isScreenSpace();
                if (ImGui::Checkbox("Screen Space##Text", &tcSS))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    tc->SetScreenSpace(tcSS);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }
                CreateHintText("Screen Space: text stays fixed on screen. World Space: text follows camera.");
            }
        }
    }

    // Layer component
    if (auto* lc = obj->GetComponent<LayerComponent>())
    {
        (void)lc;
        if (ImGui::CollapsingHeader("Layer", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Remove button
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            float buttonWidth = 100.0f;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - 10);
            bool layerRemove = ImGui::Button("Remove##Layer", ImVec2(buttonWidth, 22));
            ImGui::PopStyleColor(3);

            if (layerRemove)
            {
                propertyBeforeJson = Serialization::SerializeGameObject(obj);
                obj->RemoveComponent("LayerComponent");
                obj->SetGameLayer(0); // reset to Default
                UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
            }
            else
            {
                const char* layerNames[] = { "Default", "Background", "Player", "Enemies", "UI" };
                constexpr int NUM_LAYERS = 5;
                int currentLayer = obj->GetGameLayer();
                if (currentLayer < 0 || currentLayer >= NUM_LAYERS) currentLayer = 0;
                ImGui::Text("Game Layer");
                ImGui::SetNextItemWidth(180.0f);
                if (ImGui::Combo("##GameLayerComp", &currentLayer, layerNames, NUM_LAYERS))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    obj->SetGameLayer(currentLayer);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }
                CreateHintText("Disabling a layer freezes rendering, physics and scripting for all objects on it.");
            }
        }
    }

    // Physics component
    if (auto* physics = obj->GetComponent<PhysicsComponent>())
    {
        if (ImGui::CollapsingHeader("Physics"))
        {
            // Remove button aligned right
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            float buttonWidth = 100.0f;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - 10);
            bool physicsRemove = ImGui::Button("Remove##Physics", ImVec2(buttonWidth, 22));
            ImGui::PopStyleColor(3);

            if (physicsRemove)
            {
                propertyBeforeJson = Serialization::SerializeGameObject(obj);
                obj->RemoveComponent("PhysicsComponent");
                UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
            }
            else
            {
                // Physics sliders with respective functions
                bool isPhysEnabled = physics->IsEnabled();
                if (ImGui::Checkbox("Phsyics Enabled##Physics", &isPhysEnabled))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    physics->SetEnabled(isPhysEnabled);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }
 
                float vel[2] = { 0.0f, 0.0f };
                physics->GetVelocity(vel[0], vel[1]);
                if (ImGui::DragFloat2("Velocity", vel, 0.1f))
                {
                    physics->SetVelocity(vel[0], vel[1]);
                }
                UndoTracker velTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                bool isStatic = physics->IsStatic();
                if (ImGui::Checkbox("Static", &isStatic))
                {
                    physics->SetStatic(isStatic);
                }
                UndoTracker staticTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                bool hasGravity = physics->useGravity;
                if (ImGui::Checkbox("Gravity", &hasGravity))
                {
                    physics->SetGravity(hasGravity);
                }
                UndoTracker gravTracker(obj, editor, isEditingProperty, propertyBeforeJson);

                ImGui::Text("Mass: %.2f", physics->GetMass());
                ImGui::Text("Grounded: %s", physics->IsGrounded() ? "Yes" : "No");
            }
        }
    }

    // Collider component
    if (auto* collider = obj->GetComponent<ColliderComponent>())
    {
        if (ImGui::CollapsingHeader("Collider"))
        {
            // Remove button aligned right
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            float buttonWidth = 100.0f;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - 10);
            bool colliderRemove = ImGui::Button("Remove##Collider", ImVec2(buttonWidth, 22));
            ImGui::PopStyleColor(3);

            if (colliderRemove)
            {
                propertyBeforeJson = Serialization::SerializeGameObject(obj);
                obj->RemoveComponent("ColliderComponent");
                UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
            }
            else
            {
                bool isColEnabled = collider->IsEnabled();
                if (ImGui::Checkbox("Component Enabled##Collider", &isColEnabled))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    collider->SetEnabled(isColEnabled);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }

                const char* colliderTypes[] = { "Square", "Circle" };
                int currentType = (collider->GetType() == ColliderComponent::Type::Circle) ? 1 : 0;

                if (ImGui::Combo("Type", &currentType, colliderTypes, 2))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);

                    ColliderComponent::Type newType = (currentType == 1)
                        ? ColliderComponent::Type::Circle
                        : ColliderComponent::Type::Square;
                    collider->setColliShape(newType);
                    std::cout << "[Property Editor] Changed collider type to "
                        << colliderTypes[currentType] << std::endl;  

                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }

                // Display dimensions
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

                // Editable Size (In case we need it)
                if (collider->GetType() == ColliderComponent::Type::Square)
                {
                    AABB aabb = collider->GetAABB();
                    float width = aabb.max.x - aabb.min.x;
                    float height = aabb.max.y - aabb.min.y;

                    float newSize[2] = { width, height };
                    if (ImGui::DragFloat2("Size", newSize, 1.0f, 1.0f, 5000.0f))
                    {
                        collider->SetSize(newSize[0], newSize[1]);
                        std::cout << "[Property Editor] Changed collider size to "
                            << newSize[0] << "x" << newSize[1] << std::endl;
                    }
                    UndoTracker sizeTracker(obj, editor, isEditingProperty, propertyBeforeJson);
                }
                else if (collider->GetType() == ColliderComponent::Type::Circle)
                {
                    Circle circle = collider->GetCircle();
                    float radius = circle.radius;

                    if (ImGui::DragFloat("Radius", &radius, 1.0f, 1.0f, 5000.0f))
                    {
                        collider->SetSize(radius * 2.0f, radius * 2.0f);
                        std::cout << "[Property Editor] Changed circle radius to "
                            << radius << std::endl;
                    }
                   UndoTracker radiusTracker(obj, editor, isEditingProperty, propertyBeforeJson);
                }


                // Editable Collision Layer Dropdown
                const char* layerNames[] = { "Player", "Ground", "Enemy", "Platform", "Pickup", "Default" };
                int currentLayer = static_cast<int>(collider->GetLayer());

                if (ImGui::Combo("Layer", &currentLayer, layerNames, 6))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);

                    collider->SetLayer(static_cast<CollisionLayer>(currentLayer));
                    std::cout << "[Property Editor] Changed collision layer to "
                        << layerNames[currentLayer] << std::endl;
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }
            }
        }
    }

    if (auto* events = obj->GetComponent<EventComponent>())
    {
        // Add event into GameObjects
        RenderEventComponent(events, obj);
    }

    // Script component
    if (auto* script = obj->GetComponent<ScriptComponent>())
    {
        if (ImGui::CollapsingHeader("Script", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Remove button aligned right
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
            float buttonWidth = 100.0f;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - 10);
            bool scriptRemove = ImGui::Button("Remove##Script", ImVec2(buttonWidth, 22));
            ImGui::PopStyleColor(3);
            if (scriptRemove)
            {
                propertyBeforeJson = Serialization::SerializeGameObject(obj);

                // Remove all attached scripts from ScriptSystem before removing the component
                auto* scriptSystem = CORE->GetScriptSystem();
                if (scriptSystem)
                {
                    const auto& activeScripts = script->GetScripts();
                    for (const auto& [id, scriptName] : activeScripts)
                    {
                        scriptSystem->RemoveScript(obj, scriptName);
                    }
                }
                obj->RemoveComponent("ScriptComponent");

                UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
            }
            else
            {
                bool isScriptEnabled = script->IsEnabled();
                if (ImGui::Checkbox("Component Enabled ##Script", &isScriptEnabled))
                {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    script->SetEnabled(isScriptEnabled);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }

                if (ImGui::Button("Add Script Slot"))
                {
                    script->AddDragDropSlot("Drag Script Here", ScriptPayloadID);
                }

                if (script->GetPropertySlots().size())
                {
                    for (auto& slot : script->GetPropertySlots())
                    {
                        slot.RenderImGui();
                    }
                }

                ImGui::Text("Attached Scripts:");

                auto* scriptSystem = CORE->GetScriptSystem();
                if (scriptSystem)
                {
                    auto* instances = scriptSystem->GetScriptInstances(obj);
                    if (instances)
                    {
                        std::string scriptToRemove;
                        for (auto& instance : *instances)
                        {
                            const char* className = mono_class_get_name(instance.scriptClass->monoClass);
                            ImGui::PushID(className);

                            // Red X button to detach this script
                            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
                            if (ImGui::Button("X", ImVec2(20, 0)))
                            {
                                propertyBeforeJson = Serialization::SerializeGameObject(obj);
                                scriptToRemove = className;
                            }
                            ImGui::PopStyleColor(3);

                            ImGui::SameLine();

                            if (ImGui::TreeNode(className))
                            {
                                auto fields = scriptSystem->GetPublicFields(instance);
                                for (auto& field : fields)
                                {
                                    if (field.type == ScriptFieldType::Float)
                                    {
                                        float val = std::get<float>(field.value);
                                        if (ImGui::DragFloat(field.name.c_str(), &val, 0.1f))
                                        {
                                            scriptSystem->SetPublicField(instance, field.name, val);
                                        }
                                        UndoTracker fieldTracker(obj, editor, isEditingProperty, propertyBeforeJson);
                                    }
                                    else if (field.type == ScriptFieldType::Int)
                                    {
                                        int val = std::get<int>(field.value);
                                        if (ImGui::DragInt(field.name.c_str(), &val))
                                        {
                                            scriptSystem->SetPublicField(instance, field.name, val);
                                        }
                                        UndoTracker fieldTracker(obj, editor, isEditingProperty, propertyBeforeJson);
                                    }
                                    else if (field.type == ScriptFieldType::Bool)
                                    {
                                        bool val = std::get<bool>(field.value);
                                        if (ImGui::Checkbox(field.name.c_str(), &val))
                                        {
                                            scriptSystem->SetPublicField(instance, field.name, val);
                                        }
                                        UndoTracker fieldTracker(obj, editor, isEditingProperty, propertyBeforeJson);
                                    }
                                    else if (field.type == ScriptFieldType::String)
                                    {
                                        std::string val = std::get<std::string>(field.value);
                                        char buffer[256];
                                        strncpy(buffer, val.c_str(), sizeof(buffer));
                                        buffer[sizeof(buffer) - 1] = 0;
                                        if (ImGui::InputText(field.name.c_str(), buffer, sizeof(buffer)))
                                        {
                                            scriptSystem->SetPublicField(instance, field.name, std::string(buffer));
                                        }
                                        UndoTracker fieldTracker(obj, editor, isEditingProperty, propertyBeforeJson);
                                    }
                                }
                                ImGui::TreePop();
                            }

                            ImGui::PopID();
                        }

                        // Remove after iteration to avoid invalidating the instances vector
                        if (!scriptToRemove.empty())
                        {
                            propertyBeforeJson = Serialization::SerializeGameObject(obj);
                            scriptSystem->RemoveScript(obj, scriptToRemove);
                            script->RemoveScript(scriptToRemove);
                            UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                        }
                    }
                }
            }
        }
    }

    // Add Component Button
    ImVec2 space = ImGui::GetContentRegionAvail();
    ImVec2 buttonSize{ 100,25 };
    ImGui::SetCursorPosX((space.x - buttonSize.x)/2);
    if (ImGui::Button("Add Component", buttonSize))
    {
        ImGui::OpenPopup("Add_Component");
    }
    if (ImGui::BeginPopup("Add_Component"))
    {
        {
            for (auto& comp : AllComponents)
            {
                // can use to sort by component type in the future when we got more components, and can add explanation text
                //ComponentType type      { std::get<0>(comp) };
                std::string& name       { std::get<1>(comp) };
                ImTextureID& textureID  { std::get<2>(comp) };
                auto& onclick           { std::get<3>(comp) };

                std::string idname = "##iconItem" + name;
                bool onClick = ImGui::Selectable(idname.c_str(), false, ImGuiSelectableFlags_SpanAllColumns);
                ImGui::SameLine();

                // Draw the icon
                ImGui::Image(textureID, icon_size_smallest);

                ImGui::SameLine(40); // spacing between icon and label

                ImGui::Text(name.c_str());
                CreateHintText("Explanation text here.");

                if (onClick)
                {
                    // clicked 
                    onclick(GetSelectedGameObject());
                }
            }
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}

#pragma region Event Component ImGui
void ImGuiSystem::RenderEventComponent(EventComponent* ec, GameObject* obj) {
    if (ImGui::CollapsingHeader("Event", ImGuiTreeNodeFlags_DefaultOpen)) {

        LevelEditor* editor = CORE->GetLevelEditor();
        //static bool isEditingProperty = false;
        //static json propertyBeforeJson;

        bool isEventEnabled = ec->IsEnabled();
        if (ImGui::Checkbox("Component Enabled ##Event", &isEventEnabled))
        {
            propertyBeforeJson = Serialization::SerializeGameObject(obj);
            ec->SetEnabled(isEventEnabled);
            UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
        }

        // Data for Selection
        static const char* eventNames[] = { "None", "OnEnable", "OnDisable", "OnDestroy", "OnCollisionEnter", "OnCollisionStay", "OnCollisionExit" };
        static const char* addableActions[] = { "Play Sound", "Destroy", "Enable", "Disable", "Respawn" };

        // Get all gameObjects for the "Trigger Filter" dropdown
        std::vector<std::string> objectNames = { "Any Object" };
        std::vector<int> objectIds = { -1 };
        for (const auto& goPtr : GameObjectFactory::Instance().GetAllGameObjects()) {
            if (goPtr) {
                objectNames.push_back(goPtr->GetName());
                objectIds.push_back((int)goPtr->GetID());
            }
        }

        // Convert strings to const char* for ImGui Combo
        std::vector<const char*> cStrNames;
        for (const auto& name : objectNames) cStrNames.push_back(name.c_str());


        // Render Logic Chains
        int chainToDelete = -1;
        for (size_t i = 0; i < ec->chains.size(); ++i) {
            auto& chain = ec->chains[i];
            ImGui::PushID((int)i);

            if (ImGui::TreeNodeEx("Event Chain", ImGuiTreeNodeFlags_DefaultOpen)) {

                // ===== Trigger Selection ("When") =====
                int evIndex = (int)chain.triggerType;
                if (ImGui::Combo("Event Type", &evIndex, eventNames, IM_ARRAYSIZE(eventNames))) {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    chain.triggerType = (EventType)evIndex;
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }

                // ===== Target Filter (Trigger By) =====
                int currentIndex = 0; // Default to "Any Object"
                for (size_t k = 0; k < objectIds.size(); k++) {
                    if (objectIds[k] == chain.targetFilterID) {
                        currentIndex = (int)k;
                        break;
                    }
                }

                // Convert vector of strings to vector of char* for ImGui
               // std::vector<const char*> cStrNames;
                for (const auto& name : objectNames) cStrNames.push_back(name.c_str());

                if (ImGui::Combo("Filter: Triggered By", &currentIndex, cStrNames.data(), (int)cStrNames.size())) {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    chain.targetFilterID = objectIds[currentIndex];
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }

                // ===== Action List =====
                ImGui::NewLine();
                ImGui::Separator();
                ImGui::Text("Actions:");

                int actionToDelete = -1;
                for (size_t j = 0; j < chain.actions.size(); ++j) {
                    ImGui::PushID((int)j);
                    IAction* action = chain.actions[j].get();

                    ImGui::Bullet();
                    switch (action->GetType()) {
                    case ActionType::PlaySound: {
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Play Sound");
                        ActionPlaySound* soundAction = static_cast<ActionPlaySound*>(action);

                        // Combo Box Selection from Database
                        auto* audioMgr = CORE->GetAudioManager();
                        std::vector<std::string> soundList;

                        // Fetch sound names
                        if (audioMgr && !audioMgr->GetSoundDatabase().empty()) {
                            for (auto const& [name, sound] : audioMgr->GetSoundDatabase()) {
                                soundList.push_back(name);
                            }
                        }
                        else {
                            // Read from file
                            static std::vector<std::string> cachedFallback;
                            if (cachedFallback.empty()) {
                                std::ifstream f("../Assets/Audio/audio.json");
                                if (f.is_open()) {
                                    json audioData;
                                    f >> audioData;
                                    for (auto& element : audioData["AudioManager"]["Sounds"]) {
                                        cachedFallback.push_back(element["Name"]);
                                    }
                                }
                            }
                            soundList = cachedFallback;
                        }

                        // Render the Dropdown for selection of sounds
                        if (!soundList.empty()) {
                            if (ImGui::BeginCombo("Select Sound", soundAction->soundName.c_str())) {
                                for (const auto& soundName : soundList) {
                                    bool isSelected = (soundAction->soundName == soundName);
                                    if (ImGui::Selectable(soundName.c_str(), isSelected)) {
                                        propertyBeforeJson = Serialization::SerializeGameObject(obj);
                                        soundAction->soundName = soundName;
                                        UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                                    }
                                    if (isSelected) ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                        }

                        break;
                    }
                    case ActionType::DestroyObject:
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Destroy Self");
                        break;
                    case ActionType::EnableObject:
                        ImGui::Text("Enable Object");
                        break;
                    case ActionType::DisableObject:
                        ImGui::Text("Disable Object");
                        break;
                    case ActionType::Respawn:
                        ImGui::Text("Respawn Object");
                        break;
                    default:
                        ImGui::Text("Unknown Action");
                        break;
                    }

                    ImGui::SameLine();
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red
                    if (ImGui::SmallButton("Remove")) 
                    { 
                        propertyBeforeJson = Serialization::SerializeGameObject(obj);
                        actionToDelete = (int)j; 
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                }

                if (actionToDelete != -1) {
                    chain.actions.erase(chain.actions.begin() + actionToDelete);
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }

                // ===== Add Action =====
                static int newActionIdx = 0;
                ImGui::Combo("Add Action", &newActionIdx, addableActions, IM_ARRAYSIZE(addableActions));
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f)); // Green
                if (ImGui::Button("Add")) {
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    if (newActionIdx == 0) chain.actions.push_back(std::make_unique<ActionPlaySound>("Empty"));
                    if (newActionIdx == 1) chain.actions.push_back(std::make_unique<ActionDestroy>());
                    if (newActionIdx == 2) chain.actions.push_back(std::make_unique<ActionToggleActive>(true));
                    if (newActionIdx == 3) chain.actions.push_back(std::make_unique<ActionToggleActive>(false));
                    if (newActionIdx == 4) chain.actions.push_back(std::make_unique<ActionRespawn>());
                    UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red
                if (ImGui::Button("Delete Entire Chain")) 
                { 
                    propertyBeforeJson = Serialization::SerializeGameObject(obj);
                    chainToDelete = (int)i; 
                }
                ImGui::PopStyleColor();

                ImGui::NewLine();
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        if (chainToDelete != -1) {
            ec->chains.erase(ec->chains.begin() + chainToDelete);
            UndoTracker::RecordInstantChange(obj, editor, propertyBeforeJson);
        }

        //ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f)); //Green
        if (ImGui::Button("Add New Event")) {
            ec->chains.push_back(LogicChain());
        }
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
    }
}
#pragma endregion

//  generate unique sound name if one already exists
static std::string MakeUniqueSoundName(const std::string& baseName,
    const std::map<std::string, SoundData>& db)
{
    // if base not used, just use it
    if (db.find(baseName) == db.end())
        return baseName;

    int counter = 1;
    std::string candidate;
    do
    {
        candidate = baseName + "_" + std::to_string(counter++);
    } while (db.find(candidate) != db.end());

    return candidate;
}

#pragma region Audio Assets ImGui
// Renders the Audio Assets panel
void AudioAssetsPanel_Render()
{
    // use the flag to control visibility
    if (!showAudioAssetsPanel)
        return;

    bool open = true;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Audio Manager", &open))
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

    ImGui::TextWrapped("Drag .wav / .mp3 / .ogg files from Project Navigator into this window to add them to the game");
    ImGui::Separator();

    // global controls
    if (ImGui::Button("Stop All Sounds"))
    {
        audioMgr->StopAllSounds();
    }
    ImGui::SameLine();

    // Fade controls
    static float fadeDuration = 1.0f;
    static int selectedCategory = 0;
    static const char* categories[] = { "music", "sfx", "voice", "ambient" };
    static float fadeTargetVolume = 1.0f;

    ImGui::Separator();
    ImGui::Text("Fade Controls:");

    ImGui::PushItemWidth(100.0f);
    ImGui::InputFloat("Fade Duration (s)", &fadeDuration, 0.1f, 1.0f, "%.1f");
    if (fadeDuration < 0.1f) fadeDuration = 0.1f;
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(100.0f);
    ImGui::Combo("Category", &selectedCategory, categories, 4);
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(100.0f);
    ImGui::SliderFloat("Target Volume", &fadeTargetVolume, 0.0f, 1.0f, "%.1f");
    ImGui::PopItemWidth();

    if (ImGui::Button("Fade In"))
    {
        audioMgr->FadeInCategory(categories[selectedCategory], fadeDuration, fadeTargetVolume);
        std::cout << "[Audio] Fading in " << categories[selectedCategory]
                  << " over " << fadeDuration << "s to volume " << fadeTargetVolume << std::endl;
    }
    ImGui::SameLine();

    if (ImGui::Button("Fade Out"))
    {
        audioMgr->FadeOutCategory(categories[selectedCategory], fadeDuration);
        std::cout << "[Audio] Fading out " << categories[selectedCategory]
                  << " over " << fadeDuration << "s" << std::endl;
    }
    ImGui::SameLine();

    if (ImGui::Button("Fade Out All"))
    {
        audioMgr->FadeOutAll(fadeDuration);
        std::cout << "[Audio] Fading out all categories over " << fadeDuration << "s" << std::endl;
    }

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
                // get current database so we can avoid name clashes
                auto& db = audioMgr->GetSoundDatabase();

                std::string baseName = p.stem().string();                   // "bgm"
                std::string uniqueName = MakeUniqueSoundName(baseName, db); // "bgm1"

                SoundData sound{};
                sound.filepath = droppedPathStr;
                sound.name = p.stem().string();
                sound.loop = false;
                sound.volume = 1.0f;
                sound.pitch = 1.0f;
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
        if (ImGui::BeginTable("AudioAssetsTable", 6,
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
        {
            ImGui::TableSetupColumn("Name");
            ImGui::TableSetupColumn("Category");
            ImGui::TableSetupColumn("Loop");
            ImGui::TableSetupColumn("Volume");
            ImGui::TableSetupColumn("Pitch");
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
                    ImGui::SetNextItemWidth(-60.0f);
                    if (ImGui::SliderFloat(("##vol_" + sound.name).c_str(),
                        &volume, 0.0f, 1.0f))
                    {
                        volume = std::round(volume * 10.0f) / 10.0f;
                        sound.volume = volume;
                        audioMgr->SetSoundVolume(sound.name, volume);
                        std::cout << "[Audio Assets] Updated volume for " << sound.name << " = " << sound.volume << std::endl;
                    }
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(50.0f);
                    float volDisplay = sound.volume;
                    ImGui::InputFloat(("##vol_val_" + sound.name).c_str(), &volDisplay, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_ReadOnly);
                }

                //pitch
                ImGui::TableSetColumnIndex(4);
                {
                    float pitch = (sound.pitch <= 0.0f ? 1.0f : sound.pitch);
                    ImGui::SetNextItemWidth(-60.0f);
                    if (ImGui::SliderFloat(("##pitch_" + sound.name).c_str(),
                        &pitch, 0.5f, 2.0f, "%.2f"))
                    {
                        pitch = std::round(pitch * 10.0f) / 10.0f;
                        sound.pitch = pitch;
                        audioMgr->SetSoundPitch(sound.name, pitch);
                        std::cout << "[Audio Assets] Updated pitch for " << sound.name << " = " << sound.pitch << std::endl;
                    }
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(50.0f);
                    float pitchDisplay = sound.pitch;
                    ImGui::InputFloat(("#pitch_val_" + sound.name).c_str(), &pitchDisplay, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_ReadOnly);
                }
                /* actions */
                // play button
                ImGui::TableSetColumnIndex(5);
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

#pragma region Layer Panel ImGui

void LayerPanel_Render()
{
    if (!ImGui::Begin("Render Layers", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        ImGui::End();
        return;
    }

    ImGui::Text("Toggle layer visibility:");
    ImGui::Separator();

    // Named game layers (must match LayerManager initialization order)
    const char* layerNames[] = { "Default", "Background", "Player", "Enemies", "UI" };
    constexpr int NUM_NAMED_LAYERS = 5;

    for (int i = 0; i < NUM_NAMED_LAYERS; i++)
    {
        bool enabled = LayerManager::IsLayerEnabled(i);
        if (ImGui::Checkbox(layerNames[i], &enabled))
        {
            LayerManager::SetLayerEnabled(i, enabled);
        }
    }

    ImGui::Separator();

    // Quick buttons
    if (ImGui::Button("Show All"))
    {
        for (int i = 0; i < NUM_NAMED_LAYERS; i++)
            LayerManager::SetLayerEnabled(i, true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Hide All"))
    {
        for (int i = 0; i < NUM_NAMED_LAYERS; i++)
            LayerManager::SetLayerEnabled(i, false);
    }

    ImGui::End();
}

#pragma endregion

#pragma region Animation Previewer ImGui

void ImGuiSystem::RenderAnimationPreviewer()
{
    // DraftClip
    struct DraftClip
    {
        std::string name;
        std::string filepath;
        int  rows = 1, cols = 1;
        int  startR = 0, startC = 0;
        int  endR = 0, endC = 0;
        float fps = 12.0f;
        bool  loop = true;
        bool pingPong = false;
        bool flipRows = false;
    };

    // All statics
    static std::vector<DraftClip> draftClips;
    static int    selectedDraftClip = -1;

    // Draft (right column) preview state
    static std::string draftPath;
    static GLuint      draftTexID = 0;
    static int         draftRows = 1, draftCols = 1;
    static int         draftFrameLinear = 0;
    static float       draftTimer = 0.0f;
    static bool        isPlayingDraft = false;
    static int         draftPreviewDir = 1;   // 1 forward, -1 backward

    // Left-column GO preview state � kept as statics so playback advances
    static int         goFrameRow = 0;
    static int         goFrameCol = 0;
    static float       goFrameTimer = 0.0f;
    static bool        isPlayingCurrent = false;
    static EntityID    lastPreviewedGOID = 0;        // detect GO switch
    static std::string lastPreviewedAnim;            // detect animation switch
    static int         goPreviewDir = 1;      // 1 forward, -1 backward

    // Clip authoring inputs
    static char  actionName[64] = "Idle";
    static int   startR = 0, startC = 0;
    static int   endR = 0, endC = 0;
    static float actionFPS = 12.0f;
    static bool  actionLoop = true;
    static bool  actionPingPong = false;
    static bool  actionFlipRows = false;
    static int   frameCount = 1;
    static int   rowPick = 0;

    auto StepPreviewFrame = [](int& linear,
        int startIdx,
        int endIdx,
        bool loop,
        bool pingPong,
        int& direction,
        bool& isPlaying)
        {
            if (endIdx < startIdx)
                std::swap(startIdx, endIdx);

            if (linear < startIdx || linear > endIdx)
                linear = startIdx;

            // Single-frame animation
            if (startIdx == endIdx)
            {
                linear = startIdx;
                if (!loop)
                    isPlaying = false;
                return;
            }

            if (!pingPong)
            {
                ++linear;

                if (linear > endIdx)
                {
                    if (loop)
                    {
                        linear = startIdx;
                    }
                    else
                    {
                        linear = endIdx;
                        isPlaying = false;
                    }
                }
                return;
            }

            // Ping-pong mode
            linear += direction;

            if (direction > 0 && linear > endIdx)
            {
                direction = -1;
                linear = endIdx - 1;
            }
            else if (direction < 0 && linear < startIdx)
            {
                if (loop)
                {
                    direction = 1;
                    linear = startIdx + 1;
                }
                else
                {
                    linear = startIdx;
                    isPlaying = false;
                }
            }
        };

    // Draft-clip JSON
    static const std::string DRAFT_CLIPS_FILE = "../Assets/AnimPreview/draft_clips.json";

    // Load saved draft clips
    static bool draftClipsLoaded = false;
    if (!draftClipsLoaded)
    {
        draftClipsLoaded = true;
        try
        {
            std::ifstream f(DRAFT_CLIPS_FILE);
            if (f.is_open())
            {
                json j; f >> j;
                if (j.is_array())
                {
                    for (const auto& cj : j)
                    {
                        DraftClip c;
                        c.name = cj.value("name", "");
                        c.filepath = cj.value("filepath", "");
                        c.rows = cj.value("rows", 1);
                        c.cols = cj.value("cols", 1);
                        c.startR = cj.value("startR", 0);
                        c.startC = cj.value("startC", 0);
                        c.endR = cj.value("endR", 0);
                        c.endC = cj.value("endC", 0);
                        c.fps = cj.value("fps", 12.0f);
                        c.loop = cj.value("loop", true);
                        c.pingPong = cj.value("pingPong", false);
                        c.flipRows = cj.value("flipRows", false);
                        if (!c.name.empty()) draftClips.push_back(c);
                    }
                    std::cout << "[AnimPreview] Loaded "
                        << draftClips.size() << " draft clips.\n";
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "[AnimPreview] Load failed: " << e.what() << "\n";
        }
    }

    // Save all draft clips back to disk.
    auto SaveDraftClips = [&]()
        {
            try
            {
                std::filesystem::create_directories("../Assets/AnimPreview");
                json j = json::array();
                for (const auto& c : draftClips)
                {
                    j.push_back({
                        {"name",     c.name},    {"filepath", c.filepath},
                        {"rows",     c.rows},    {"cols",     c.cols},
                        {"startR",   c.startR},  {"startC",   c.startC},
                        {"endR",     c.endR},    {"endC",     c.endC},
                        {"fps",      c.fps},     {"loop",     c.loop},
                        {"pingPong", c.pingPong},{"flipRows", c.flipRows}
                        });
                }
                std::ofstream f(DRAFT_CLIPS_FILE);
                if (f.is_open()) f << j.dump(4);
            }
            catch (const std::exception& e)
            {
                std::cerr << "[AnimPreview] Save failed: " << e.what() << "\n";
            }
        };

    // Reset right-column preview playback to the first valid frame.
    auto ResetDraftPlayback = [&]()
        {
            isPlayingDraft = false;
            draftTimer = 0.0f;
            draftFrameLinear = 0;
            draftPreviewDir = 1;
        };

    // Reset left-column preview playback to the first valid frame.
    auto ResetGoPlayback = [&](Spritesheet* ss)
        {
            isPlayingCurrent = false;
            goFrameTimer = 0.0f;
            goPreviewDir = 1;
            goFrameRow = ss ? ss->GetCurrentRow() : 0;
            goFrameCol = ss ? ss->GetCurrentCol() : 0;
        };

    // Find a saved draft clip by name.
    auto FindClipByName = [&](const std::string& n) -> int
        {
            for (int i = 0; i < (int)draftClips.size(); ++i)
                if (draftClips[i].name == n) return i;
            return -1;
        };

    // clamp selectedDraftClip every frame
    if (selectedDraftClip >= (int)draftClips.size())
        selectedDraftClip = -1;

    // Context
    GameObject* obj = factory.GetGameObject(selectedGameObjectID);
    RenderComponent* rc = obj ? obj->GetComponent<RenderComponent>() : nullptr;
    Spritesheet* ss = obj ? obj->GetComponent<Spritesheet>() : nullptr;

    // Detect GO switch - reset left-column playback state
    EntityID currentGOID = obj ? obj->GetID() : 0;
    if (currentGOID != lastPreviewedGOID)
    {
        lastPreviewedGOID = currentGOID;
        lastPreviewedAnim = "";
        ResetGoPlayback(ss);
    }

    bool hasStaticTexture = (rc && rc->GetTextureID() != 0 && !rc->GetTexturePath().empty());
    bool hasExistingSS = (ss != nullptr);

    // Preview Window
    ImGui::Begin("Animation Preview");
    isAnimPreviewFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    static bool fromFile = true;
    static bool fromGO = false;
    if (ImGui::Checkbox("From File", &fromFile)) 
    {
        if (fromFile) fromGO = false;
    }
    ImGui::SameLine();
    if (ImGui::Checkbox("From Game Object", &fromGO))
    {
        if (fromGO) fromFile = false;
    }

    ImGui::Separator();

    int columnCount = (fromGO && obj) ? 2 : 1;
    ImGui::Columns(columnCount, "PreviewSplit", true);

    // LEFT COLUMN � current GameObject preview
    if (fromGO)
    {
        if (!obj)
        {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "PLEASE SELECT A GAMEOBJECT");
        }
        else
        {
            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "CURRENT GAMEOBJECT");

            ss = obj->GetComponent<Spritesheet>();
            hasExistingSS = (ss != nullptr);

            // If undo just removed the Spritesheet, stop playback
            if (!hasExistingSS)
            {
                isPlayingCurrent = false;
                goFrameTimer = 0.0f;
            }

            if (hasExistingSS && ss)
            {
                const int goRows = std::max(1, ss->GetRows());
                const int goCols = std::max(1, ss->GetCols());

                std::string curAnimName = ss->GetCurrentAnimationName();

                // Detect animation change - re-sync
                if (curAnimName != lastPreviewedAnim)
                {
                    lastPreviewedAnim = curAnimName;
                    goFrameRow = ss->GetCurrentRow();
                    goFrameCol = ss->GetCurrentCol();
                    goFrameTimer = 0.0f;
                    goPreviewDir = 1;
                }

                // playback for the preview image.
                if (isPlayingCurrent)
                {
                    const auto& reg = ss->GetAnimationRegistry();
                    auto it = reg.find(curAnimName);
                    if (it != reg.end())
                    {
                        const auto& animData = it->second;
                        float fps = (animData.fps > 0.001f) ? animData.fps : 6.0f;
                        float frameDur = 1.0f / fps;

                        goFrameTimer += ImGui::GetIO().DeltaTime;
                        while (goFrameTimer >= frameDur)
                        {
                            goFrameTimer -= frameDur;

                            int linear = goFrameRow * goCols + goFrameCol;

                            StepPreviewFrame(
                                linear,
                                animData.startframe,
                                animData.endframe,
                                true,                    // or animData.loop if store loop there too
                                animData.pingPong,
                                goPreviewDir,
                                isPlayingCurrent
                            );

                            goFrameRow = linear / std::max(1, goCols);
                            goFrameCol = linear % std::max(1, goCols);
                        }
                    }
                    else
                    {
                        // Fallback: no registry data
                        // When not playing
                        goFrameTimer += ImGui::GetIO().DeltaTime;
                        if (goFrameTimer >= 0.1f)
                        {
                            goFrameTimer = 0.0f;
                            goFrameCol = (goFrameCol + 1) % goCols;
                        }
                    }
                }
                else
                {
                    // Not playing - mirror spritesheet's real current frame
                    goFrameRow = ss->GetCurrentRow();
                    goFrameCol = ss->GetCurrentCol();
                }

                // Clamp to grid to avoid UB on UV calculation
                goFrameRow = std::clamp(goFrameRow, 0, goRows - 1);
                goFrameCol = std::clamp(goFrameCol, 0, goCols - 1);

                float uW = 1.0f / (float)goCols;
                float vH = 1.0f / (float)goRows;

                // Mirror the row when flipRows is active, matching GetFlipUVOffset() at runtime.
                    int goUVRow = ss->GetFlipRows() ? (goRows - 1) - goFrameRow : goFrameRow;

                ImVec2 uv0 = ImVec2(goFrameCol * uW, (goUVRow + 1) * vH);
                ImVec2 uv1 = ImVec2((goFrameCol + 1) * uW, goUVRow * vH);

                ImGui::Image(
                    (void*)(intptr_t)ss->GetTextureID(),
                    ImVec2(180, 180), uv0, uv1,
                    ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.2f)
                );

                // Animation dropdown � lists registered clips on the spritesheet.
                {
                    const auto& reg = ss->GetAnimationRegistry();
                    std::vector<const char*> names;
                    names.reserve(reg.size());

                    for (const auto& kv : reg) 
                    {
                        names.push_back(kv.first.c_str());
                    }

                    if (!names.empty())
                    {
                        int curIdx = 0;
                        for (int i = 0; i < (int)names.size(); ++i) 
                        {
                            if (curAnimName == names[i]) { curIdx = i; break; }
                        }

                        if (ImGui::Combo("Action##GO", &curIdx, names.data(), (int)names.size()))
                        {
                            ss->PlayAnimation(names[curIdx]);
                            lastPreviewedAnim = names[curIdx];
                            goFrameRow = ss->GetCurrentRow();
                            goFrameCol = ss->GetCurrentCol();
                            goFrameTimer = 0.0f;
                            goPreviewDir = 1;
                        }
                    }
                    else
                    {
                        ImGui::TextDisabled("No animations registered.");
                    }
                }

                if (ImGui::Button(isPlayingCurrent ? "STOP##Cur" : "PLAY##Cur", ImVec2(180, 0)))
                {
                    if (!isPlayingCurrent)
                    {
                        isPlayingCurrent = true;
                        goFrameTimer = 0.0f;
                        goPreviewDir = 1;
                        goFrameRow = ss ? ss->GetCurrentRow() : 0;
                        goFrameCol = ss ? ss->GetCurrentCol() : 0;
                    }
                    else
                    {
                        isPlayingCurrent = false;
                        goFrameRow = ss ? ss->GetCurrentRow() : 0;
                        goFrameCol = ss ? ss->GetCurrentCol() : 0;
                        goFrameTimer = 0.0f;
                        goPreviewDir = 1;
                    }
                }
            }
            else if (hasStaticTexture)
            {
                rc = obj->GetComponent<RenderComponent>(); // re-fetch
                if (rc)
                {
                    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "STATIC TEXTURE");
                    ImGui::Image(
                        (void*)(intptr_t)rc->GetTextureID(),
                        ImVec2(180, 180),
                        ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f),
                        ImVec4(1, 1, 1, 1), ImVec4(1, 1, 1, 0.2f)
                    );
                    ImGui::TextDisabled("Texture: %s", rc->GetTexturePath().substr(rc->GetTexturePath().find_last_of("\\/") + 1).c_str());
                }
            }
            else
            {
                ImGui::BeginChild("NoExisting", ImVec2(180, 180), true);
                ImGui::TextWrapped("\n\n NO TEXTURE\n / SPRITESHEET");
                ImGui::EndChild();
            }

            ImGui::Text("GAMEOBJECT: %s", obj->GetName().c_str());
            ImGui::Spacing();
        }
        ImGui::NextColumn();
    }

    // RIGHT COLUMN � draft preview + authoring
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), draftTexID ? "EDIT / PREVIEW" : "DRAG SPRITE HERE");

    std::string dropped;

    if (draftTexID != 0)
    {
        int activeStartR = std::clamp(startR, 0, std::max(0, draftRows - 1));
        int activeStartC = std::clamp(startC, 0, std::max(0, draftCols - 1));
        int activeEndR   = std::clamp(endR  , 0, std::max(0, draftRows - 1));
        int activeEndC   = std::clamp(endC  , 0, std::max(0, draftCols - 1));

        int startIdx = (activeStartR * draftCols) + activeStartC;
        int endIdx   = (activeEndR * draftCols) + activeEndC;
        if (endIdx < startIdx) std::swap(startIdx, endIdx);

        if (draftFrameLinear < startIdx || draftFrameLinear > endIdx) 
        {
            draftFrameLinear = startIdx;
        }         

        if (isPlayingDraft)
        {
            float fps = std::max(0.001f, actionFPS);
            draftTimer += ImGui::GetIO().DeltaTime;

            while (draftTimer >= 1.0f / fps)
            {
                draftTimer -= 1.0f / fps;

                StepPreviewFrame(
                    draftFrameLinear,
                    startIdx,
                    endIdx,
                    actionLoop,
                    actionPingPong,
                    draftPreviewDir,
                    isPlayingDraft
                );
            }
        }

        // The draft preview uses a linear frame index for playback, then converts
        // that index back into row/column for UV calculation
        int showRow = draftFrameLinear / std::max(1, draftCols);
        int showCol = draftFrameLinear % std::max(1, draftCols);
        float duW = 1.0f / (float)std::max(1, draftCols);
        float dvH = 1.0f / (float)std::max(1, draftRows);

        // When flipRows is on, mirror the row so row 0 = top of the image,
        // matching what GetFlipUVOffset() does at runtime.
        int uvRow = actionFlipRows ? (std::max(1, draftRows) - 1) - showRow : showRow;

        ImGui::BeginGroup();
        ImGui::Image(
            (void*)(intptr_t)draftTexID, ImVec2(180, 180),
            ImVec2(showCol * duW, (uvRow + 1) * dvH),
            ImVec2((showCol + 1) * duW, uvRow* dvH),
            ImVec4(1, 1, 1, 1), ImVec4(1, 1, 0, 1)
        );
        ImGui::EndGroup();

        ReceiveDragAndDrop(SpritePayloadID, dropped);

        if (ImGui::Button( isPlayingDraft ? "STOP PREVIEW##Drf" : "PLAY PREVIEW##Drf", ImVec2(180, 0)))
        {
            if(!isPlayingDraft)
            {
                isPlayingDraft = true;
                draftTimer = 0.0f;
                draftPreviewDir = 1;

                int startIdxPlay = (startR * draftCols) + startC;
                draftFrameLinear = startIdxPlay;
            }
            else
            {
                isPlayingDraft = false;
            }
        }

        ImGui::NewLine();
        ImGui::Text("FILE: %s", draftPath.substr(draftPath.find_last_of("\\/") + 1).c_str());
    }
    else
    {
        ImGui::BeginChild("DropZone", ImVec2(180, 180), true);
        ImGui::TextDisabled("\n\n   DRAG SPRITE HERE\n   TO START EDITING");
        ImGui::EndChild();
        ReceiveDragAndDrop(SpritePayloadID, dropped);
    }

    // Drag-drop load
    if (!dropped.empty() && SupportedImageFile(dropped))
    {
        draftPath = dropped;

        if (draftTexID) glDeleteTextures(1, &draftTexID);
        draftTexID = GraphicsSystem::LoadTexture(dropped.c_str());

        draftRows = 1;  draftCols = 1;
        startR = startC = endR = endC = 0;
        rowPick = 0;  frameCount = 1;
        actionFPS = 12.0f;
        actionLoop = true;
        std::strncpy(actionName, "Idle", sizeof(actionName) - 1);
        actionName[sizeof(actionName) - 1] = '\0';
        selectedDraftClip = -1;
        ResetDraftPlayback();
    }

    // Controls
    if (draftTexID != 0)
    {
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("GRID CONFIGURATION:");
        ImGui::PushItemWidth(80);
        bool gridChanged = false;

        if (ImGui::Button("13 x 10"))
        {
            draftRows = 13; draftCols = 10;
            ResetDraftPlayback();
        }
        ImGui::Spacing();

        if (ImGui::InputInt("ROWS", &draftRows))
        {
            draftRows = std::max(1, draftRows); gridChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::InputInt("COLS", &draftCols))
        {
            draftCols = std::max(1, draftCols); gridChanged = true;
        }
        ImGui::PopItemWidth();
        if (gridChanged) ResetDraftPlayback();

        // Draft clip list
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("DRAFT CLIPS:");

        std::vector<std::string>  clipLabels;
        clipLabels.reserve(draftClips.size() + 1);
        clipLabels.push_back("<New / Unsaved>");
        for (const auto& c : draftClips) 
        {
            clipLabels.push_back(c.name.empty() ? "<unnamed>" : c.name);       
        }

        std::vector<const char*> clipPtrs;
        clipPtrs.reserve(clipLabels.size());
        for (const auto& s : clipLabels) 
        {
            clipPtrs.push_back(s.c_str());
        }        

        int comboIndex = std::clamp(selectedDraftClip + 1, 0, (int)clipPtrs.size() - 1);

        if (ImGui::Combo("Draft Clip", &comboIndex, clipPtrs.data(), (int)clipPtrs.size()))
        {
            selectedDraftClip = comboIndex - 1;

            if (selectedDraftClip >= 0 && selectedDraftClip < (int)draftClips.size())
            {
                const auto& c = draftClips[selectedDraftClip];
                draftRows = std::max(1, c.rows);
                draftCols = std::max(1, c.cols);

                std::strncpy(actionName, c.name.c_str(), sizeof(actionName) - 1);
                actionName[sizeof(actionName) - 1] = '\0';

                if (!c.filepath.empty() && c.filepath != draftPath)
                {
                    draftPath = c.filepath;
                    draftTexID = GraphicsSystem::LoadTexture(draftPath.c_str());
                }

                startR = std::clamp(c.startR, 0, draftRows - 1);
                startC = std::clamp(c.startC, 0, draftCols - 1);
                endR = std::clamp(c.endR, 0, draftRows - 1);
                endC = std::clamp(c.endC, 0, draftCols - 1);

                actionFPS = c.fps;
                actionLoop = c.loop;
                actionPingPong = c.pingPong;
                actionFlipRows = c.flipRows;

                rowPick = startR;
                frameCount = std::max(1, (endC - startC) + 1);

                draftFrameLinear = (startR * draftCols) + startC;
                isPlayingDraft = false;
                draftTimer = 0.0f;
                draftPreviewDir = 1;
            }
            else
            {
                std::strncpy(actionName, "", sizeof(actionName) - 1);
                actionName[sizeof(actionName) - 1] = '\0';
                draftFrameLinear = 0;
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("New##DraftClip"))
        {
            selectedDraftClip = -1;
            ResetDraftPlayback();
            std::strncpy(actionName, "", sizeof(actionName) - 1);
            actionName[sizeof(actionName) - 1] = '\0';
        }

        // Define action range 
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::Text("DEFINE ACTION RANGE:");

        ImGui::InputText("Name", actionName, IM_ARRAYSIZE(actionName));
        ImGui::NewLine();
        ImGui::Checkbox("Loop", &actionLoop);
        ImGui::SameLine();
        ImGui::Checkbox("Ping-Pong", &actionPingPong);        
        ImGui::SameLine();
        ImGui::Checkbox("Flip Rows", &actionFlipRows);
        ImGui::PushItemWidth(120);
        ImGui::InputFloat("FPS", &actionFPS, 0.5f, 1.0f, "%.2f");
        actionFPS = std::max(0.001f, actionFPS);
        ImGui::PopItemWidth();
        ImGui::NewLine();

        rowPick = std::clamp(rowPick, 0, draftRows - 1);
        endR = std::clamp(endR, 0, draftRows - 1);
        startC = std::clamp(startC, 0, draftCols - 1);
        frameCount = std::max(1, frameCount);

        if (ImGui::SliderInt("Start Row", &rowPick, 0, draftRows - 1))
            ResetDraftPlayback();
        startR = rowPick;

        // Allow end row to span multiple rows for multi-row animations
        endR = std::clamp(endR, startR, draftRows - 1);
        if (ImGui::SliderInt("End Row", &endR, startR, draftRows - 1))
            ResetDraftPlayback();

        if (ImGui::SliderInt("Start Col", &startC, 0, draftCols - 1))
            ResetDraftPlayback();

        // "frameCount" is edited in UI as clip length, then converted into endC.
        int maxCount = std::max(1, draftCols - startC);
        frameCount = std::clamp(frameCount, 1, maxCount);
        if (ImGui::SliderInt("End Col", &frameCount, 1, maxCount))
            ResetDraftPlayback();

        //endR = startR;
        endC = std::clamp(startC + frameCount - 1, 0, draftCols - 1);

        ImGui::TextDisabled("Range: row %d  cols %d->%d  (%d frames)", startR, startC, endC, frameCount);
        ImGui::NewLine();

        std::string nameStr(actionName);
        bool hasName = !nameStr.empty();

        if (!hasName) ImGui::BeginDisabled();

        // Save the currently authored clip into the editor draft list.
        // If another clip with the same name exists, overwrite it.
        // Otherwise, create a new entry.
        if (ImGui::Button("Add / Update Clip"))
        {
            int idx = FindClipByName(nameStr);
            DraftClip c;
            c.name = nameStr;   c.filepath = draftPath;
            c.rows = draftRows; c.cols = draftCols;
            c.startR = startR;    c.startC = startC;
            c.endR = endR;      c.endC = endC;
            c.fps = actionFPS; c.loop = actionLoop;
            c.pingPong = actionPingPong;
            c.flipRows = actionFlipRows;

            if (idx >= 0)
            {
                draftClips[idx] = c; selectedDraftClip = idx;
            }
            else
            {
                draftClips.push_back(c); selectedDraftClip = (int)draftClips.size() - 1;
            }

            SaveDraftClips();
        }
        if (!hasName) ImGui::EndDisabled();
        CreateHintText(hasName ? "Save clip to draft list." : "Enter a name first.");

        ImGui::SameLine();
        bool canRemove = (selectedDraftClip >= 0 &&
            selectedDraftClip < (int)draftClips.size());
        if (!canRemove) ImGui::BeginDisabled();
        if (ImGui::Button("Remove Clip")) // Remove the currently selected saved draft clip from the draft list.
        {
            draftClips.erase(draftClips.begin() + selectedDraftClip);
            selectedDraftClip = -1;
            ResetDraftPlayback();
            std::strncpy(actionName, "", sizeof(actionName) - 1);
            actionName[sizeof(actionName) - 1] = '\0';
            SaveDraftClips();
        }
        if (!canRemove) ImGui::EndDisabled();
    }

    // APPLY TO SELECTED OBJECT
    if (obj && draftTexID != 0)
    {
        ImGui::NewLine();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));

        if (ImGui::Button("APPLY TO SELECTED OBJECT", ImVec2(-1, 35)))
        {
            LevelEditor* editor = CORE->GetLevelEditor();

            rc = obj->GetComponent<RenderComponent>();
            ss = obj->GetComponent<Spritesheet>();

            // Record gameObject state
            json beforeJson;
            if (editor)
                beforeJson = Serialization::SerializeGameObject(obj);

            // Capture whether a Spritesheet existed before apply.
            //const bool hadSpritesheetBefore = (ss != nullptr);

            try
            {
                if (!draftPath.empty())
                {
                    // Treat the texture as animated if it is a grid larger than 1x1
                    const bool wantsAnimated =
                        (draftRows > 1 || draftCols > 1 || !draftClips.empty());

                    if (!rc)
                    {
                        obj->AddComponent<RenderComponent>();
                        rc = obj->GetComponent<RenderComponent>();
                    }
                    if (rc)
                    {
                        rc->SetTextureID(draftTexID);
                        rc->SetTexturePath(draftPath);
                        rc->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
                    }

                    if (wantsAnimated)
                    {
                        if (ss)
                            ss->LoadNewSpritesheet(
                                draftPath.c_str(), draftRows, draftCols);
                        else
                        {
                            obj->AddComponent<Spritesheet>(
                                draftPath.c_str(), draftRows, draftCols);
                            ss = obj->GetComponent<Spritesheet>();
                        }

                        if (rc && ss)
                        {
                            rc->SetTextureID(ss->GetTextureID());
                            rc->SetTexturePath(draftPath);
                            rc->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
                        }

                        if (ss)
                        {
                            if (selectedDraftClip >= 0 && selectedDraftClip < (int)draftClips.size())
                            {
                                const auto& c = draftClips[selectedDraftClip];
                                if (!c.name.empty())
                                {
                                    ss->RegisterAnimation(
                                        c.name, c.filepath,
                                        c.rows, c.cols,
                                        c.startR, c.startC,
                                        c.endR, c.endC,
                                        c.fps, c.pingPong, 
                                        c.flipRows);
                                    ss->PlayAnimation(c.name);
                                    std::cout << "[AnimPreview] Applied clip '" << c.name
                                        << "' to '" << obj->GetName() << "'.\n";
                                }
                            }
                            else
                            {
                                // No saved clip selected � apply from the live name/range inputs
                                std::string oneName(actionName);
                                if (!oneName.empty())
                                {
                                    ss->RegisterAnimation(
                                        oneName, draftPath,
                                        draftRows, draftCols,
                                        startR, startC,
                                        endR, endC,
                                        actionFPS, actionPingPong,
                                        actionFlipRows);
                                    ss->PlayAnimation(oneName);
                                    std::cout << "[AnimPreview] Applied unsaved clip '" << oneName
                                        << "' to '" << obj->GetName() << "'.\n";
                                }
                                else
                                {
                                    std::cout << "[AnimPreview] No clip selected and no name entered � nothing applied.\n";
                                }
                            }
                        }
                    }
                    else
                    {
                        // Static texture � remove Spritesheet if it was there
                        if (ss)
                        {
                            obj->RemoveComponent<Spritesheet>();
                            ss = nullptr;
                        }
                    }
                }

                // Record Undo
                if (editor && !beforeJson.is_null())
                    UndoTracker::RecordInstantChange(obj, editor, beforeJson);

                // Sync the left-column preview with the newly applied state
                ss = obj->GetComponent<Spritesheet>();
                if (ss)
                {
                    lastPreviewedAnim = ss->GetCurrentAnimationName();
                    goFrameRow = ss->GetCurrentRow();
                    goFrameCol = ss->GetCurrentCol();
                    goFrameTimer = 0.0f;
                }
                isPlayingCurrent = false;   // reset so user explicitly hits PLAY

                std::cout << "[AnimPreview] Applied to '"
                    << obj->GetName() << "'.\n";
            }
            catch (const std::exception& e)
            {
                std::cerr << "[AnimPreview] Apply exception: "
                    << e.what() << "\n";
                g_showEditorErrorPopup = true;
                g_editorErrorMessage = 
                    std::string("[AnimPreview] Apply failed: ") + e.what();
            }

            ResetDraftPlayback();
        }
        ImGui::PopStyleColor();

        ImGui::TextDisabled(
            "Ctrl+Z to undo after applying. Clip list saves automatically.");
    }

    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::End();
}

#pragma endregion

// Helpers
// cover your eyes this will be hardcoded based off what we have rn
// [WIP] Consider adding explanation text in the tuple for specific explanation.
void PopulateComponentList()
{
    //transform physics collider camera render text animator
    // stores Display Text, icon texture, std::function to check if component is alr present, if not add it.
    // ^ might have to rethink this because we might want multiple of the same components 

    // push_back to vector the component details
        // lambda for adding component checks if already has component
            // if dont have, then add
    AllComponents.push_back({ ComponentType::Transform, "Layer Component"     , ImTextureID(icon_active_ui), [](GameObject* go){
        if (!go->GetComponent<LayerComponent>()) {
            go->AddComponent<LayerComponent>(); std::cout << "Added Layer Component to " << go->GetName() << std::endl; } } });

    AllComponents.push_back({ ComponentType::Transform, "Transform Component" , ImTextureID(icon_active_ui), [](GameObject* go){
        if (!go->GetComponent<TransformComponent>()){
            go->AddComponent<TransformComponent>();     std::cout << "Added Transform Component to " << go->GetName() << std::endl; } } });
                
    AllComponents.push_back({ ComponentType::Transform, "Physics Component"   , ImTextureID(icon_active_ui), [](GameObject* go){ 
            if (!go->GetComponent<PhysicsComponent>()){ 
                go->AddComponent<PhysicsComponent>();   std::cout << "Added Physics Component to " << go->GetName() << std::endl; } } });

    AllComponents.push_back({ ComponentType::Transform, "Collider Component"  , ImTextureID(icon_active_ui), [](GameObject* go){ 
            if (!go->GetComponent<ColliderComponent>()) { 
                go->AddComponent<ColliderComponent>();  std::cout << "Added Collider Component to " << go->GetName() << std::endl; } } });

    // Camera component doesnt work like that LMAO
    //AllComponents.push_back({ ComponentType::Camera, "Camera Component"    , ImTextureID(icon_active_ui), [](GameObject* go){ 
    //        if (!go->GetComponent<CameraComponent>()) { 
    //            go->AddComponent<CameraComponent>();    std::cout << "Added Camera Component to " << go->GetName() << std::endl; } } });

    AllComponents.push_back({ ComponentType::Render, "Render Component"    , ImTextureID(icon_active_ui), [](GameObject* go){ 
            if (!go->GetComponent<RenderComponent>()) { 
                go->AddComponent<RenderComponent>();    std::cout << "Added Render Component to " << go->GetName() << std::endl; } } });
    AllComponents.push_back({ ComponentType::Event, "Event Component"    , ImTextureID(icon_active_ui), [](GameObject* go) {
            if (!go->GetComponent<EventComponent>()) {
                go->AddComponent<EventComponent>();    std::cout << "Added Event Component to " << go->GetName() << std::endl; } } });

    AllComponents.push_back({ ComponentType::Script, "Script Component"    , ImTextureID(icon_active_ui), [](GameObject* go) {
        if (!go->GetComponent<ScriptComponent>()) {
            go->AddComponent<ScriptComponent>();    std::cout << "Added Script Component to " << go->GetName() << std::endl;
} } });

    AllComponents.push_back({ ComponentType::Render, "Text Component", ImTextureID(icon_active_ui), [](GameObject* go) {
        if (!go->GetComponent<TextComponent>()) {
            go->AddComponent<TextComponent>("../Assets/FrederickatheGreat-Regular.ttf", GLuint(36), std::string("New Text"));
            std::cout << "Added Text Component to " << go->GetName() << std::endl;
        } } });
    //AllComponents.push_back({ "Animator Component"  , ImTextureID(icon_active_ui), [](GameObject* go) { go->AddComponent<Spritesheet>(); } });
    // cant add these because theyre abit special, not just a simple slap on component
}

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
        return "Pickup";
    case 6:
        return "Default";
    default:
        return "Unknown";
    }
}

void TileMap_Render(float dt)
{
    // Renders the tilemap - putting up here so game opens showing viewport first
    if (showTilemapEditor)
    {
        tilemapEditor->SetVisible(true);
        tilemapEditor->Update(dt);
        tilemapEditor->Render();

        //handle window close button
        if (!tilemapEditor->IsVisible())
        {
            showTilemapEditor = false;
        }
    }
    else
    {
        tilemapEditor->SetVisible(false);
    }
}
