#include "ColliderComponent.h"
#include "LevelEditor.h"
#include "physics.h"
#include "ResourceManager.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

/**
 * @brief Constructor - initialise level editor
 */
LevelEditor::LevelEditor()
: factory(GameObjectFactory::Instance()),
    selectedGameObjectID(0),
    showPropertyEditor(true),
    showSaveDialog(false),
    showLoadDialog(false)
{
    strcpy(saveFilepath, "../Engine/LevelEditor/Levels/level.txt");
    strcpy(loadFilepath, "../Engine/LevelEditor/Levels/level.txt");
}

/**
 * @brief Destructor
 */
LevelEditor::~LevelEditor()
{
}

/**
 * @brief Initialise the level editor system
 */
void LevelEditor::Initialize()
{
    std::cout << "LevelEditor initialised" << std::endl;

    // create default level
    currentLevel.levelName = "Untitled Level";
    currentLevel.levelDescription = "Beninging";
}

/**
 * @brief Update - Render all editor GUI panels
 */
void LevelEditor::Update(float dt)
{
    if (!InputManager::KeyStateImgui)
        return;

    // render menu bar
    RenderMenuBar();

    // render hierarchy panel
    RenderHierarchy();
    
    // render property editor
    if (showPropertyEditor && selectedGameObjectID != 0)
    {
        RenderPropertyEditor();
    }

    // save dialog
    if (showSaveDialog)
    {
        ImGui::OpenPopup("Save Level");
        if (ImGui::BeginPopupModal("Save Level", &showSaveDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Save level to file:");
            ImGui::InputText("Filepath", saveFilepath, sizeof(saveFilepath));

            if (ImGui::Button("Save", ImVec2(120, 0)))
            {
                if (SaveLevel(saveFilepath))
                {
                    std::cout << "Level saved successfully to: " << saveFilepath << std::endl;
                    showSaveDialog = false;
                }
                else
                {
                    std::cerr << "Failed to save level!" << std::endl;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                showSaveDialog = false;
            }

            ImGui::EndPopup();
        }
    }

    // load dialog
    if (showLoadDialog)
    {
        ImGui::OpenPopup("Load Level");
        if (ImGui::BeginPopupModal("Load Level", &showLoadDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Load level from file:");
            ImGui::InputText("Filepath", loadFilepath, sizeof(loadFilepath));

            if (ImGui::Button("Load", ImVec2(120, 0)))
            {
                if (LoadLevel(loadFilepath))
                {
                    std::cout << "Level loaded successfully from: " << loadFilepath << std::endl;
                    showLoadDialog = false;
                }
                else
                {
                    std::cerr << "Failed to load level!" << std::endl;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                showLoadDialog = false;
            }

            ImGui::EndPopup();
        }
    }
}

/**
 * @brief Render the main menu bar
 */
void LevelEditor::RenderMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Level", "Ctrl+N"))
            {
                NewLevel();
            }

            if (ImGui::MenuItem("Save Level", "Ctrl+S"))
            {
                showSaveDialog = true;
            }

            if (ImGui::MenuItem("Load Level", "Ctrl+O"))
            {
                showLoadDialog = true;
            }

            ImGui::Separator();

            // recent levels submenu
            if (ImGui::BeginMenu("Recent Levels"))
            {
                if (recentLevels.empty())
                {
                    ImGui::MenuItem("(No recent levels)", nullptr, false, false);
                }
                else
                {
                    for (const auto& path : recentLevels)
                    {
                        if (ImGui::MenuItem(path.c_str()))
                        {
                            LoadLevel(path);
                        }
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Properties", nullptr, &showPropertyEditor);

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }
}

/**
 * @brief Render hierarchy panel listing all game objects
 */
void LevelEditor::RenderHierarchy()
{
    ImGui::Begin("Hierarchy");

    ImGui::Text("Level: %s", currentLevel.levelName.c_str());
    ImGui::Separator();

    // display count
    auto& allObjects = factory.GetAllGameObjects();
    ImGui::Text("Game Objects: %zu", allObjects.size());
    ImGui::Separator();

    // list all game objects
    for (const auto& objPtr : allObjects)
    {
        if (!objPtr->IsActive())
            continue;

        GameObject* obj = objPtr.get();
        EntityID id = obj->GetID();

        // highlight if selected
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (selectedGameObjectID == id)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        // display as selectable tree node
        bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)id, flags, "%s (ID: %u)",
            obj->GetName().c_str(), id);

        // select on click
        if (ImGui::IsItemClicked())
        {
            SetSelectedGameObject(id);
        }

        // context menu
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Select"))
            {
                SetSelectedGameObject(id);
            }
            if (ImGui::MenuItem("Clone"))
            {
                factory.CloneGameObject(id);
            }
            if (ImGui::MenuItem("Delete"))
            {
                RemoveGameObject(id);
                if (selectedGameObjectID == id)
                    selectedGameObjectID = 0;
            }
            ImGui::EndPopup();
        }
    }

    ImGui::End();
}

/**
 * @brief Render property editor for selected game object
 */
void LevelEditor::RenderPropertyEditor()
{
    GameObject* obj = GetSelectedGameObject();
    if (!obj)
        return;

    ImGui::Begin("Level Editor Properties", &showPropertyEditor);

    ImGui::Text("GameObject: %s", obj->GetName().c_str());
    ImGui::Text("ID: %u", obj->GetID());
    ImGui::Separator();

    bool isActive = obj->IsActive();
    if (ImGui::Checkbox("Active", &isActive))
    {
        obj->SetActive(isActive);
    }

    ImGui::Spacing();

    // transform component
    if (auto* transform = obj->GetComponent<TransformComponent>())
    {
        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        {
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

    // render component
    if (auto* render = obj->GetComponent<RenderComponent>())
    {
        if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Shape Type: %s", GetShapeTypeName((int)render->GetShapeType()));

            // colour editor if no texture (check if textureID is 0)
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

    // physics component
    if (auto* physics = obj->GetComponent<PhysicsComponent>())
    {
        if (ImGui::CollapsingHeader("Physics"))
        {
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

    // collider component
    if (auto* collider = obj->GetComponent<ColliderComponent>())
    {
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

    ImGui::Spacing();
    ImGui::Separator();

    // component management
    if (ImGui::Button("Add Component"))
    {
        ImGui::OpenPopup("AddComponentPopup");
    }

    if (ImGui::BeginPopup("AddComponentPopup"))
    {
        if (!obj->HasComponent<TransformComponent>() && ImGui::MenuItem("Transform"))
        {
            obj->AddComponent<TransformComponent>(0.0f, 0.0f);
        }
        if (!obj->HasComponent<RenderComponent>() && ImGui::MenuItem("Render"))
        {
            obj->AddComponent<RenderComponent>(RenderComponent::ShapeType::Square, vec4(1, 1, 1, 1));
        }
        if (!obj->HasComponent<PhysicsComponent>() && ImGui::MenuItem("Physics"))
        {
            obj->AddComponent<PhysicsComponent>(1.0f, false);
        }
        if (!obj->HasComponent<ColliderComponent>() && ImGui::MenuItem("Collider"))
        {
            obj->AddComponent<ColliderComponent>(ColliderComponent::Type::Square, 50.0f, 50.0f, CollisionLayer::Default);
        }
        ImGui::EndPopup();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // game object actions
    ImGui::Text("GameObject Actions:");

    // clone button
    if (ImGui::Button("Clone GameObject", ImVec2(-1, 30)))
    {
        GameObject* clone = factory.CloneGameObject(selectedGameObjectID);
        if (clone)
        {
            std::cout << "Cloned object: " << clone->GetName() << std::endl;
            SetSelectedGameObject(clone->GetID()); // select the clone
        }
    }

    // delete button (red color)
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));

    if (ImGui::Button("Delete GameObject", ImVec2(-1, 30)))
    {
        RemoveGameObject(selectedGameObjectID);
        selectedGameObjectID = 0; // dselect after deletion
        ImGui::PopStyleColor(3);
        ImGui::End();
        return;
    }

    ImGui::PopStyleColor(3);
    ImGui::End();
}

/**
 * @brief Create a new empty level
 */
void LevelEditor::NewLevel()
{
    // confirm if there are unsaved changes
    auto& allObjects = factory.GetAllGameObjects();
    if (!allObjects.empty())
    {
        std::cout << "Warning: Creating new level will clear all existing objects!" << std::endl;
    }

    // clear all game objects
    factory.Clear();

    // reset level data
    currentLevel = LevelData();
    currentLevel.levelName = "New Level";
    selectedGameObjectID = 0;

    std::cout << "Created new level" << std::endl;
}

/**
 * @brief Save level to file
 */
bool LevelEditor::SaveLevel(const std::string& filepath)
{
    // serialise all active game objects
    currentLevel.gameObjects.clear();

    for (const auto& objPtr : factory.GetAllGameObjects())
    {
        if (objPtr->IsActive())
        {
            GameObjectData data = SerializeGameObject(objPtr.get());
            currentLevel.gameObjects.push_back(data);
        }
    }

    // save to file
    bool success = SaveLevelToFile(currentLevel, filepath);

    if (success)
    {
        // add to recent levels
        auto it = std::find(recentLevels.begin(), recentLevels.end(), filepath);
        if (it != recentLevels.end())
        {
            recentLevels.erase(it);
        }
        recentLevels.insert(recentLevels.begin(), filepath);
        if (recentLevels.size() > MAX_RECENT_LEVELS)
        {
            recentLevels.pop_back();
        }
    }

    return success;
}

/**
 * @brief Load level from file
 */
bool LevelEditor::LoadLevel(const std::string& filepath)
{
    LevelData loadedLevel;
    if (!LoadLevelFromFile(filepath, loadedLevel))
    {
        return false;
    }

    // clear existing level
    factory.Clear();
    selectedGameObjectID = 0;

    // load new level
    currentLevel = loadedLevel;

    // instantiate all game objects
    for (const auto& objData : currentLevel.gameObjects)
    {
        DeserializeGameObject(objData);
    }

    // ad to recent levels
    auto it = std::find(recentLevels.begin(), recentLevels.end(), filepath);
    if (it != recentLevels.end())
    {
        recentLevels.erase(it);
    }
    recentLevels.insert(recentLevels.begin(), filepath);
    if (recentLevels.size() > MAX_RECENT_LEVELS)
    {
        recentLevels.pop_back();
    }

    std::cout << "Loaded level: " << currentLevel.levelName << " with "
        << currentLevel.gameObjects.size() << " objects" << std::endl;

    return true;
}

/**
 * @brief Add a game object to the level
 */
GameObject* LevelEditor::AddGameObject(const GameObjectData& data)
{
    return DeserializeGameObject(data);
}

/**
 * @brief Remove a game object from the level
 */
void LevelEditor::RemoveGameObject(EntityID id)
{
    factory.DestroyGameObject(id);
    std::cout << "Removed GameObject ID: " << id << std::endl;
}

/**
 * @brief Get the selected game object
 */
GameObject* LevelEditor::GetSelectedGameObject() const
{
    if (selectedGameObjectID == 0)
        return nullptr;
    return factory.GetGameObject(selectedGameObjectID);
}

/**
 * @brief Set the selected game object
 */
void LevelEditor::SetSelectedGameObject(EntityID id)
{
    selectedGameObjectID = id;
}

/**
 * @brief Serialise a game object to data structure
 */
GameObjectData LevelEditor::SerializeGameObject(GameObject* obj)
{
    GameObjectData data;
    data.name = obj->GetName();
    data.originalID = obj->GetID();
    data.isActive = obj->IsActive();

    // serialise transform
    if (auto* transform = obj->GetComponent<TransformComponent>())
    {
        data.hasTransform = true;
        transform->GetPosition(data.posX, data.posY);
        transform->GetScale(data.scaleX, data.scaleY);
        data.rotation = transform->GetRotation();
    }

    // serialise render
    if (auto* render = obj->GetComponent<RenderComponent>())
    {
        data.hasRender = true;
        data.shapeType = (int)render->GetShapeType();
        data.hasTexture = (render->GetTextureID() != 0);

        if (!data.hasTexture)
        {
            vec4 color = render->GetColor();
            data.colorR = color.x;
            data.colorG = color.y;
            data.colorB = color.z;
            data.colorA = color.w;
        }
    }

    // serialise physics
    if (auto* physics = obj->GetComponent<PhysicsComponent>())
    {
        data.hasPhysics = true;
        data.mass = physics->GetMass();
        physics->GetVelocity(data.velocityX, data.velocityY);
        data.isStatic = physics->IsStatic();
        data.hasGravity = physics->useGravity;
    }

    // serialise collider
    if (auto* collider = obj->GetComponent<ColliderComponent>())
    {
        data.hasCollider = true;
        data.colliderType = (int)collider->GetType();

        if (collider->GetType() == ColliderComponent::Type::Circle)
        {
            Circle circle = collider->GetCircle();
            data.colliderWidth = circle.radius * 2.0f; // diameter
            data.colliderHeight = circle.radius * 2.0f;
        }
        else
        {
            AABB aabb = collider->GetAABB();
            data.colliderWidth = aabb.max.x - aabb.min.x;
            data.colliderHeight = aabb.max.y - aabb.min.y;
        }

        data.collisionLayer = (int)collider->GetLayer();
    }

    return data;
}

/**
 * @brief Deserialise game object data and create instance
 */
GameObject* LevelEditor::DeserializeGameObject(const GameObjectData& data)
{
    // create base game object
    GameObject* obj = factory.CreateGameObject(data.name);

    if (!obj)
        return nullptr;

    // add transform component
    if (data.hasTransform)
    {
        auto* transform = obj->AddComponent<TransformComponent>(data.posX, data.posY);
        transform->SetScale(data.scaleX, data.scaleY);
        transform->SetRotation(data.rotation);
    }

    // add render component
    if (data.hasRender)
    {
        RenderComponent::ShapeType shape = (RenderComponent::ShapeType)data.shapeType;

        if (data.hasTexture && !data.texturePath.empty())
        {
            // load texture (would need GraphicsSystem access)
            // GLuint texID = GraphicsSystem::LoadTexture(data.texturePath.c_str());
            // obj->AddComponent<RenderComponent>(texID);
        }
        else
        {
            vec4 color(data.colorR, data.colorG, data.colorB, data.colorA);
            obj->AddComponent<RenderComponent>(shape, color);
        }
    }

    // add physics component
    if (data.hasPhysics)
    {
        auto* physics = obj->AddComponent<PhysicsComponent>(data.mass, data.isStatic);
        physics->SetVelocity(data.velocityX, data.velocityY);
        physics->SetGravity(data.hasGravity);
    }

    // add collider component
    if (data.hasCollider)
    {
        ColliderComponent::Type colliderType = (ColliderComponent::Type)data.colliderType;
        CollisionLayer layer = (CollisionLayer)data.collisionLayer;
        obj->AddComponent<ColliderComponent>(colliderType, data.colliderWidth, data.colliderHeight, layer);
    }

    // set active state
    obj->SetActive(data.isActive);

    return obj;
}

/**
 * @brief Save level data to file (simple text format)
 */
bool LevelEditor::SaveLevelToFile(const LevelData& data, const std::string& filepath)
{
    std::ofstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for writing: " << filepath << std::endl;
        return false;
    }

    // write level metadata
    file << "LEVEL_BEGIN\n";
    file << "Name: " << data.levelName << "\n";
    file << "Description: " << data.levelDescription << "\n";
    file << "ObjectCount: " << data.gameObjects.size() << "\n";
    file << "\n";

    // write each game object
    for (const auto& obj : data.gameObjects)
    {
        file << "GAMEOBJECT_BEGIN\n";
        file << "Name: " << obj.name << "\n";
        file << "Active: " << obj.isActive << "\n";

        // transform
        if (obj.hasTransform)
        {
            file << "Transform: " << obj.posX << " " << obj.posY << " "
                << obj.scaleX << " " << obj.scaleY << " " << obj.rotation << "\n";
        }

        // render
        if (obj.hasRender)
        {
            file << "Render: " << obj.shapeType << " " << obj.hasTexture << " "
                << obj.colorR << " " << obj.colorG << " " << obj.colorB << " " << obj.colorA << "\n";
            if (obj.hasTexture)
            {
                file << "Texture: " << obj.texturePath << "\n";
            }
        }

        // physics
        if (obj.hasPhysics)
        {
            file << "Physics: " << obj.mass << " " << obj.velocityX << " " << obj.velocityY << " "
                << obj.isStatic << " " << obj.hasGravity << "\n";
        }

        // collider
        if (obj.hasCollider)
        {
            file << "Collider: " << obj.colliderType << " " << obj.colliderWidth << " "
                << obj.colliderHeight << " " << obj.collisionLayer << "\n";
        }

        // animation
        if (obj.hasAnimation)
        {
            file << "Animation: " << obj.spritesheetPath << " " << obj.numRows << " " << obj.numCols << " "
                << obj.startFrame << " " << obj.endFrame << " " << obj.fps << "\n";
        }

        file << "GAMEOBJECT_END\n\n";
    }

    file << "LEVEL_END\n";
    file.close();

    return true;
}

/**
 * @brief Load level data from file
 */
bool LevelEditor::LoadLevelFromFile(const std::string& filepath, LevelData& outData)
{
    std::ifstream file(filepath);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file for reading: " << filepath << std::endl;
        return false;
    }

    std::string line;
    outData = LevelData(); // reset

    // read level metadata
    while (std::getline(file, line))
    {
        if (line == "LEVEL_BEGIN")
        {
            continue;
        }
        else if (line.find("Name: ") == 0)
        {
            outData.levelName = line.substr(6);
        }
        else if (line.find("Description: ") == 0)
        {
            outData.levelDescription = line.substr(13);
        }
        else if (line == "GAMEOBJECT_BEGIN")
        {
            break; // start reading game objects
        }
    }

    // read game objects
    GameObjectData currentObj;
    bool readingObject = true;

    while (readingObject && std::getline(file, line))
    {
        if (line == "GAMEOBJECT_BEGIN")
        {
            currentObj = GameObjectData(); // reset for new object
        }
        else if (line.find("Name: ") == 0)
        {
            currentObj.name = line.substr(6);
        }
        else if (line.find("Active: ") == 0)
        {
            currentObj.isActive = (line.substr(8) == "1");
        }
        else if (line.find("Transform: ") == 0)
        {
            std::istringstream iss(line.substr(11));
            iss >> currentObj.posX >> currentObj.posY >> currentObj.scaleX >> currentObj.scaleY >> currentObj.rotation;
            currentObj.hasTransform = true;
        }
        else if (line.find("Render: ") == 0)
        {
            std::istringstream iss(line.substr(8));
            int hasTexInt;
            iss >> currentObj.shapeType >> hasTexInt >> currentObj.colorR >> currentObj.colorG
                >> currentObj.colorB >> currentObj.colorA;
            currentObj.hasRender = true;
            currentObj.hasTexture = (hasTexInt == 1);
        }
        else if (line.find("Texture: ") == 0)
        {
            currentObj.texturePath = line.substr(9);
        }
        else if (line.find("Physics: ") == 0)
        {
            std::istringstream iss(line.substr(9));
            int staticInt, gravityInt;
            iss >> currentObj.mass >> currentObj.velocityX >> currentObj.velocityY >> staticInt >> gravityInt;
            currentObj.hasPhysics = true;
            currentObj.isStatic = (staticInt == 1);
            currentObj.hasGravity = (gravityInt == 1);
        }
        else if (line.find("Collider: ") == 0)
        {
            std::istringstream iss(line.substr(10));
            iss >> currentObj.colliderType >> currentObj.colliderWidth >> currentObj.colliderHeight >> currentObj.collisionLayer;
            currentObj.hasCollider = true;
        }
        else if (line.find("Animation: ") == 0)
        {
            std::istringstream iss(line.substr(11));
            iss >> currentObj.spritesheetPath >> currentObj.numRows >> currentObj.numCols
                >> currentObj.startFrame >> currentObj.endFrame >> currentObj.fps;
            currentObj.hasAnimation = true;
        }
        else if (line == "GAMEOBJECT_END")
        {
            outData.gameObjects.push_back(currentObj);
            currentObj = GameObjectData();
        }
        else if (line == "LEVEL_END")
        {
            readingObject = false;
            break;
        }
    }

    file.close();
    return true;
}

/**
 * @brief Get shape type name from enum value
 */
const char* LevelEditor::GetShapeTypeName(int type)
{
    switch (type)
    {
    case 0: return "Square";
    case 1: return "Circle";
    case 2: return "Line";
    case 3: return "Point";
    default: return "Unknown";
    }
}

/**
 * @brief Get collision layer name from enum value
 */
const char* LevelEditor::GetCollisionLayerName(int layer)
{
    switch (layer)
    {
    case 0: return "Player";
    case 1: return "Ground";
    case 2: return "Wall";
    case 3: return "Enemy";
    case 4: return "Platform";
    case 5: return "Default";
    default: return "Unknown";
    }
}
