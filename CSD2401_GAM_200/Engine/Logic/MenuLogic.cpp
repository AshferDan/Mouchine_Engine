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
    GameObject* StartButton = factory.FindGameObject("PlayButton");
    GameObject* QuitButton = factory.FindGameObject("QuitButton");
    GameObject* SettingButton = factory.FindGameObject("SettingsButton");
    if (StartButton)
    {
        auto* transform = StartButton->GetComponent<TransformComponent>();
        if (!transform) return;

        vec2 buttonPos{ transform->posX, transform->posY };
        vec2 buttonSize{ transform->scaleX, transform->scaleY };
        bool checker = IsAreaClicked(buttonPos.x, buttonPos.y,
            buttonSize.x, buttonSize.y,
            mouseX, mouseY);

        // 5. If clicked, load the new scene
        if (checker == true) {
            std::cout << "inside the square";
            std::cout << "PlayButton Clicked! Loading level_1" << std::endl;
            Serialization::LoadScene("Prefab_TestScene");
        }
    }

    if (SettingButton)
    {
        auto* transform = SettingButton->GetComponent<TransformComponent>();
        if (!transform) return;

        vec2 buttonPos{ transform->posX, transform->posY };
        vec2 buttonSize{ transform->scaleX, transform->scaleY };
        bool checker = IsAreaClicked(buttonPos.x, buttonPos.y,
            buttonSize.x, buttonSize.y,
            mouseX, mouseY);

       
        if (checker == true) {
            std::cout << "inside the square";
            std::cout << "PlayButton Clicked! Loading level_2" << std::endl;
            Serialization::LoadScene("Prefab_TestScene2");
        }
    }

    if (QuitButton)
    {
        auto* transform = QuitButton->GetComponent<TransformComponent>();
        if (!transform) return;

        vec2 buttonPos{ transform->posX, transform->posY };
        vec2 buttonSize{ transform->scaleX, transform->scaleY };
        bool checker = IsAreaClicked(buttonPos.x, buttonPos.y,
            buttonSize.x, buttonSize.y,
            mouseX, mouseY);


        if (checker == true) {
            std::cout << "inside the square";
            std::cout << "PlayButton Clicked! Loading level_2" << std::endl;
            GraphicsSystem* graphics = CORE->GetGraphicsSystem();
            if (graphics) {
                GLFWwindow* window = graphics->getWindow();
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }
    }
}