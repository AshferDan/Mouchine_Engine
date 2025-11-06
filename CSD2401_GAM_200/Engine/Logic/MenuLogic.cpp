#include "MenuLogic.h"
#include <iostream> // For testing

void MenuLogicSystem::Update(float dt)
{
    // 3. Get the "PlayButton"
    auto& factory = GameObjectFactory::Instance();
    GameObject* StartButton = factory.FindGameObject("PlayButton");
    GameObject* QuitButton = factory.FindGameObject("QuitButton");
    GameObject* SettingButton = factory.FindGameObject("SettingsButton");

    if (InputManager::IsMouseLeftClicked()) {
        double mouseX = InputManager::GetMouseWorldX();
        double mouseY = InputManager::GetMouseWorldY();

        auto& factory = GameObjectFactory::Instance();
        GameObject* StartButton = factory.FindGameObject("PlayButton");

        if (StartButton) {
            auto* tx = StartButton->GetComponent<TransformComponent>();
            if (tx) {
                bool clicked = IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, mouseX, mouseY);

                // --- DEBUG PRINT START ---
                std::cout << "\n--- CLICK DEBUG ---" << std::endl;
                std::cout << "Mouse World Pos: (" << mouseX << ", " << mouseY << ")" << std::endl;
                std::cout << "Button Area: [X:" << tx->posX << ", Y:" << tx->posY
                    << "] [W:" << tx->scaleX << ", H:" << tx->scaleY << "]" << std::endl;
                std::cout << "Is Clicked?: " << (clicked ? "YES >> LOAD SCENE" : "NO") << std::endl;
                std::cout << "-------------------\n" << std::endl;
                // --- DEBUG PRINT END ---

                if (clicked) {
                    Serialization::LoadScene("Prefab_TestScene");
                    return; // RETURN IMMEDIATELY so you don't click multiple things at once
                }
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
}