#pragma once

#include "factory.h"
#include "GameObject.h"
#include "RenderComponent.h"
#include "System.h"
#include "transform.h"

#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

class GameObjectFactory;

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

    GameObjectData() :
        originalID(0), isActive(true),
        hasTransform(false), posX(0), posY(0), scaleX(1), scaleY(1), rotation(0),
        hasRender(false), shapeType(0), hasTexture(false), colorR(1), colorG(1), colorB(1), colorA(1),
        hasPhysics(false), mass(1), velocityX(0), velocityY(0), isStatic(false), hasGravity(false),
        hasCollider(false), colliderType(0), colliderWidth(0), colliderHeight(0), collisionLayer(0),
        hasAnimation(false), numRows(1), numCols(1), startFrame(0), endFrame(0), fps(1)
    {
    }
};

/**
 * @struct LevelData
 * @brief Container for an entire level's game objects and metadata
 */
struct LevelData
{
    std::string levelName;
    std::string levelDescription;
    std::vector<GameObjectData> gameObjects;

    LevelData() : levelName("Untitled Level"), levelDescription("") {}
};

/**
 * @class LevelEditor
 * @brief System for editing, saving, and loading game levels through ImGui interface
 *
 * Implements:
 * 1. Working GUI level editor tool
 * 2. Data-driven level loading
 * 3. Property editor for selected game objects
 * 4. Game object listing in hierarchy
 * 5. Multiple level save/load from editor menu
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

    // level management
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

    // GameObject Management
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

private:
    // GUI rendering functions
    /**
     * @brief Render the main menu bar (File menu with New/Save/Load)
     */
    void RenderMenuBar();

    /**
     * @brief Render the hierarchy panel listing all game objects
     */
    void RenderHierarchy();

    /**
     * @brief Render the property editor for selected game object
     */
    void RenderPropertyEditor();

    // helper Functions
    /**
     * @brief Serialise a GameObject to GameObjectData structure
     * @param obj GameObject to serialise
     * @return GameObjectData structure with object's data
     */
    GameObjectData SerializeGameObject(GameObject* obj);

    /**
     * @brief Deserialise GameObjectData and create GameObject instance
     * @param data GameObject data to deserialise
     * @return Pointer to created GameObject
     */
    GameObject* DeserializeGameObject(const GameObjectData& data);

    /**
     * @brief Save level data to JSON file
     * @param data Level data to save
     * @param filepath File path to save to
     * @return true if successful, false otherwise
     */
    bool SaveLevelToFile(const LevelData& data, const std::string& filepath);

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

    // file dialog states
    bool showSaveDialog;
    bool showLoadDialog;
    char saveFilepath[256];
    char loadFilepath[256];

    std::vector<std::string> recentLevels;
    static constexpr int MAX_RECENT_LEVELS = 5;
};
