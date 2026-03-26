/* Start Header************************************************************************ /
/*!
\file       Level1.cpp
\author     Lee Hwee Min [100% - 32 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Level1.h"
#include "PauseState.h"
//#include "Level2.h"
#include <iostream>
//#include <Windows.h> // Just a helper for now
#include "Utils.h"

std::string shellTextureCollected{ "../Assets/Sprites/Icons/Shell_icon.png" }, shellTextureNotCollected{ "../Assets/Sprites/Icons/Shell_icon_dark.png" };
int cachedCoins{ -1 }, cachedShells{ -1 };

void Level1::Load() {
	std::cout << "[LEVEL 1] Load Assets\n";
    
    //Serialization::LoadScene("Assets/Scenes/Level1.json");
}

void Level1::Init() {
    std::cout << "[LEVEL 1] Initialize\n";
}

// wait for a start() to add initpickups to 
void Level1::Start()
{
    std::cout << "[LEVEL 1] Start - called once on Play\n";
}

void Level1::Update(float dt) {
	timer += dt;
    auto& factory = GameObjectFactory::Instance();
    //std::cout << pickupSys1.currCoins << '\n';

    int currCoins = CORE->GetGameCallbacks()->GetCurrentCoins();
    int currShells = CORE->GetGameCallbacks()->GetCurrentShells();

    // optimisation so dont have to constantly keep updating textures and text
    // skips all of update so anything important in this script put above here!
    if (cachedCoins == currCoins && cachedShells == currShells)
    {
        return;
    }
    else
    {
        cachedShells = currShells;
        cachedCoins = currCoins;
    }

    GameObject* coinText = factory.FindGameObject("CoinCountText");
    GameObject* shellIcon1 = factory.FindGameObject("ShellIcon1");
    GameObject* shellIcon2 = factory.FindGameObject("ShellIcon2");
    GameObject* shellIcon3 = factory.FindGameObject("ShellIcon3");

    if (coinText)
    {
        if (coinText->IsActive())
        {
            //int currCoins = pickupSys1.GetCurrentCoins();

            coinText->GetComponent<TextComponent>()->SetText(std::to_string(currCoins * 10));
        }
    }

    if (shellIcon1 && shellIcon2 && shellIcon3) 
    {
        if (shellIcon1->IsActive() && shellIcon1->GetComponent<RenderComponent>())          // only checking for optimisation but please make sure yall do the same for the other 2
        {
            shellIcon1->GetComponent<RenderComponent>()->SetTextureID(GraphicsSystem::LoadTexture(
                (currShells >= 1) ? shellTextureCollected.c_str() : shellTextureNotCollected.c_str()));
            shellIcon2->GetComponent<RenderComponent>()->SetTextureID(GraphicsSystem::LoadTexture(
                (currShells >= 2) ? shellTextureCollected.c_str() : shellTextureNotCollected.c_str()));
            shellIcon3->GetComponent<RenderComponent>()->SetTextureID(GraphicsSystem::LoadTexture(
                (currShells >= 3) ? shellTextureCollected.c_str() : shellTextureNotCollected.c_str()));
        }
    }

    //old code
    // Press P to enter pause -- for testing, will have to be place in inputmanager
    //if (GetAsyncKeyState('P') & 1)
    //{
    //    GameManager::Instance().PushState(
    //        std::unique_ptr<GameState>(new PauseState()));
    //}

    // Proceed to level 2th
    //if (true) {
    //    GameManager::Instance().ChangeState(std::make_unique<Level2>());
    //}
}

void Level1::Draw() {
    //std::cout << "[LEVEL 1] Drawing Level\n";
}

void Level1::Free() {
    std::cout << "[LEVEL 1] Free Objects\n";
    //GameObjectFactory::Instance().DestroyAllGameObjects();
}

void Level1::Unload() {
    std::cout << "[LEVEL 1] Unload Assets\n";
    GameObjectFactory::Instance().DestroyAllGameObjects();
}