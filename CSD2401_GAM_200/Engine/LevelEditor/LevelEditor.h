#pragma once

#include "Animator.hpp"
#include "factory.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "System.h"
#include "transform.h"
#include "PrefabManager.h"

#include <imgui.h>
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class GameObjectFactory;
using json = nlohmann::json;

/**
 * @enum EditorMode
 * @brief Represents the current state of the level editor
 */
enum class EditorMode
{
    Play,   // objects running but cant edit
    Pause,  // game paused and objects frozen
    Stop    // stopped, back to editor state
};

/**
 * @struct GameObjectData
 * @brief Serialisable data structure for saving/loading game object properties
 */
struct GameObjectData
{
    std::string name;
    EntityID originalID;
    bool isActive;

    // transform data
    bool hasTransform;
    float posX, posY;
    float scaleX, scaleY;
    float rotation;

    // render data
    bool hasRender;
    int shapeType;
    bool hasTexture;
    std::string texturePath;
    float colorR, colorG, colorB, colorA;

    // physics data
    bool hasPhysics;
    float mass;
    float velocityX, velocityY;
    bool isStatic;
    bool hasGravity;

    // collider data
    bool hasCollider;
    int colliderType;
    float colliderWidth;
    float colliderHeight;
    int collisionLayer;

    // animation data
    bool hasAnimation;
    std::string spritesheetPath;
    int numRows, numCols;
    int startFrame, endFrame;
    float fps;
    int currentRow;
    int currentCol;
    float currentFrameTime;

    GameObjectData() :
        originalID(0), isActive(true),
        hasTransform(false), posX(0), posY(0), scaleX(1), scaleY(1), rotation(0),
        hasRender(false), shapeType(0), hasTexture(false), colorR(1), colorG(1), colorB(1), colorA(1),
        hasPhysics(false), mass(1), velocityX(0), velocityY(0), isStatic(false), hasGravity(false),
        hasCollider(false), colliderType(0), colliderWidth(0), colliderHeight(0), collisionLayer(0),
        hasAnimation(false), numRows(1), numCols(1), startFrame(0), endFrame(0), fps(1),
        currentRow(0), currentCol(0), currentFrameTime(0.0f)
    {}
};

/**
 * @struct LevelData
 * @brief Container for an entire level's game objects and metadata
 */
struct LevelData
{
    std::vector<GameObjectData> gameObjects;
};

/**
 * @class LevelEditor
 * @brief System for editing, saving, and loading game levels through ImGui interface
 */
class LevelEditor : public ISystem
{
public:
    /**
     * @brief Construct a new level editor
     */
    LevelEditor();

    /**
     * @brief Destroy the level editor
     */
    ~LevelEditor();

    /**
     * @brief Initialise the level editor system
     */
    void Initialize() override;

    /**
     * @brief Update the level editor GUI (call every frame)
     * @param dt Delta time
     */
    void Update(float dt) override;

    /**
     * @brief Get the system name
     * @return System name string
     */
    std::string GetName() override { return "LevelEditor"; }

    // Json
    bool SaveLevelJSON(const std::string& filepath);
    bool LoadLevelJSON(const std::string& filepath);
    std::vector<std::string> GetAllSceneFiles();

    // Level management
    /**
     * @brief Save the current level to a file
     * @param filepath Path to save the level file
     * @return true if save successful, false otherwise
     */
    bool SaveLevel(const std::string& filepath);

    /**
     * @brief Load a level from a file
     * @param filepath Path to the level file
     * @return true if load successful, false otherwise
     */
    bool LoadLevel(const std::string& filepath);

    /**
     * @brief Create a new empty level
     */
    void NewLevel();

    /**
     * @brief Get current level data
     * @return Reference to current level data
     */
    LevelData& GetCurrentLevel() { return currentLevel; }

    // Game object management
    /**
     * @brief Add a new game object through the editor
     * @param data GameObject data to instantiate
     * @return Pointer to created GameObject, or nullptr if failed
     */
    GameObject* AddGameObject(const GameObjectData& data);

    /**
     * @brief Remove a game object from the level
     * @param id EntityID of the game object to remove
     */
    void RemoveGameObject(EntityID id);

    /**
     * @brief Get the currently selected game object
     * @return Pointer to selected GameObject, or nullptr if none selected
     */
    GameObject* GetSelectedGameObject() const;

    /**
     * @brief Set the currently selected game object
     * @param id EntityID of the game object to select
     */
    void SetSelectedGameObject(EntityID id);

    // Play/Stop functionality
    /**
     * @brief Get the current editor mode
     * @return Current EditorMode
     */
    EditorMode GetEditorMode() const { return editorMode; }

    /**
     * @brief Check if the editor is in play mode
     * @return true if in play mode, false otherwise
     */
    //bool IsPlaying() const { return editorMode == EditorMode::Play; }

    /**
     * @brief Check if the editor is paused
     * @return true if paused, false otherwise
     */
    //bool IsPaused() const { return editorMode == EditorMode::Paused; }

    /**
     * @brief Enter play mode (save state and start playing)
     */
    void EnterPlayMode();

    /**
     * @brief Stop play mode (restore saved state)
     */
    void StopPlayMode();

    /**
     * @brief Pause/unpause the game
     */
    void TogglePause();

    /**
     * @brief Play and stop the game
     */
    void TogglePlayStop();

    // Camera control functions
    // render the camera control panel
    void RenderCameraControls();

    // update
    void UpdateCameraControls();

    // reset camera
    void ResetCamera();

    // camera transformations to the rendering system
    void ApplyCameraTransform();

    // get the editor camera view-projection matrix
    mat4 GetEditorViewProjection(float windowWidth, float windowHeight) const;

    // camera pos x
    float GetCameraX() const { return cameraX; }

    // camera pos y
    float GetCameraY() const { return cameraY; }

    // camera zoom
    float GetCameraZoom() const { return cameraZoom; }

private:
    // GUI rendering functions
    /**
     * @brief Render the main menu bar (File menu with New/Save/Load)
     */
    void RenderMenuBar();

    /**
     * @brief Render the play controls toolbar
     */
    void RenderPlayControls();

    /**
     * @brief Render the hierarchy panel listing all game objects
     */
    void RenderHierarchy();

    /**
     * @brief Render the property editor for selected game object
     */
    void RenderPropertyEditor();

    // Play mode functions
    /**
     * @brief Save the current scene state for later restoration
     */
    void SaveSceneState();

    /**
     * @brief Restore the scene state from saved data
     */
    void RestoreSceneState();

    // Helper functions
    /**
     * @brief Serialise a GameObject to GameObjectData structure
     * @param obj GameObject to serialise
     * @return GameObjectData structure with object's data
     */
    GameObjectData SerialiseGameObject(GameObject* obj);

    /**
     * @brief Deserialise GameObjectData and create GameObject instance
     * @param data GameObject data to deserialise
     * @return Pointer to created GameObject
     */
    GameObject* DeserialiseGameObject(const GameObjectData& data);

    /**
     * @brief Load level data from JSON file
     * @param filepath File path to load from
     * @param outData Output level data
     * @return true if successful, false otherwise
     */
    bool LoadLevelFromFile(const std::string& filepath, LevelData& outData);

    /**
     * @brief Get shape type name from enum
     * @param type Shape type enum value
     * @return String name of shape type
     */
    const char* GetShapeTypeName(int type);

    /**
     * @brief Get collision layer name from enum
     * @param layer Collision layer enum value
     * @return String name of collision layer
     */
    const char* GetCollisionLayerName(int layer);

private:
    GameObjectFactory& factory;           // reference to game object factory
    LevelData currentLevel;               // current level being edited
    EntityID selectedGameObjectID;        // currently selected game object
    bool showPropertyEditor;              // show property editor panel
    std::string currentFilename;          // show current filename

    // file dialog states
    bool showSaveDialog;
    bool showLoadDialog;
    char saveFilepath[256];
    char loadFilepath[256];

    std::vector<std::string> recentLevels;
    static constexpr int MAX_RECENT_LEVELS = 5;

    // Play / stop state
    EditorMode editorMode;                // current editor mode
    LevelData savedSceneState;            // saved state for when entering play mode
    bool hasUnsavedChanges;               // track if there are unsaved changes
    EntityID savedEntityIDCounter;
    std::vector<GameObjectData> savedSceneData;

    // Editor camera state
    float cameraX;                        // camera position X
    float cameraY;                        // camera position Y
    float cameraZoom;                     // camera zoom level
    bool isPanning;                       // is currently panning with mouse
    float lastMouseX;                     // last mouse X position for panning
    float lastMouseY;                     // last mouse Y position for panning
    float panSpeed;                       // camera pan sensitivity
    float zoomSpeed;                      // camera zoom sensitivity
    float minZoom;                        // minimum zoom level
    float maxZoom;                        // maximum zoom level
};
