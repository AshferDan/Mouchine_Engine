#include "MenuLogic.h"
#include <iostream> // For testing

void MenuLogicSystem::Update(float dt)
{
    // 1. Check for a single-frame click
    if (!InputManager::IsMouseLeftClicked()) {
        return; // No click, do nothing
    }

    // 2. Get mouse position (in world coordinates)
    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();

    // 3. Get the "PlayButton"
    auto& factory = GameObjectFactory::Instance();
    GameObject* obj = factory.FindGameObject("PlayButton");

    if (obj)
    {
        auto* transform = obj->GetComponent<TransformComponent>();
        if (!transform) return;

        vec2 buttonPos{ transform->posX, transform->posY };
        vec2 buttonSize{ transform->scaleX, transform->scaleY };

        // 4. Run the check
        bool checker = IsAreaClicked(buttonPos.x, buttonPos.y,
            buttonSize.x, buttonSize.y,
            mouseX, mouseY);

        // 5. If clicked, load the new scene
        if (checker == true) {
            std::cout << "inside the square";
            std::cout << "PlayButton Clicked! Loading level_1" << std::endl;

            // This is the correct call. It will run all the cleanup
            // logic (stop music, destroy objects).
            Serialization::LoadScene("Prefab_TestScene");
        }
    }
}