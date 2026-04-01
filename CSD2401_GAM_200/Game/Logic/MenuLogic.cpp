/* Start Header ************************************************************************/
/*!
\file       MenuLogic.cpp
\author     Jonathan Fam Qian Wei [24% - 83 lines],
            Lee Hwee Min [5% - 16 lines],
            Leck Xin Yee [71% - 253 lines]
\par        Fam.q@digipen.edu,
            l.hweemin@digipen.edu,
            xinyee.leck@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "MenuLogic.h"
#include "stb_image.h"
#include <cmath>
#include <cfloat>
#include <cstdio>
#include "GhostSystem.h"
#include "CameraManager.h"
#include "TextComponent.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

std::string introPath = "../Assets/cutscene_M5.mpeg";
std::string creditsPath = "../Assets/Credits.mpeg";
std::string level1Path = "../Assets/Scenes/Level_1.json";
std::string level2Path = "../Assets/Scenes/Level_2.json";
std::string level3Path = "../Assets/Scenes/Level_2_V2.json";

// abit overkill but if we want more screens in the menu this is better
enum MenuScreens
{
    Home = 0, // Normal landing page
    Settings,
    LevelSelect,
    Leaderboard,
    Extra,
    HTP,
    QUIT
};
MenuScreens currScreen = MenuScreens::Home;

void MenuLogicSystem::Update(float dt)
{
    // Get commonly used singletons
    AudioManager *audioManager = CORE->GetAudioManager();
    auto &factory = GameObjectFactory::Instance();

    // Update audio fade
    if (audioManager)
    {
        audioManager->UpdateFade(dt);
    }

    // Check if credits were played and have finished, then restart BGM
    if (creditsWerePlayed)
    {
        CutsceneSystem *cutscene = CORE->GetCutsceneSystem();
        if (cutscene && !cutscene->IsPlaying())
        {
            // Credits finished, restart BGM
            if (audioManager)
            {
                const auto &sounds = audioManager->GetSoundDatabase();
                if (sounds.find("MainMenuBGM") != sounds.end())
                {
                    // std::cout << "[MenuLogic] MainMenuBGM found, starting playback" << std::endl;
                    audioManager->StopSound("bgm_channel");
                    audioManager->PlaySound("MainMenuBGM", "bgm_channel");

                    // fade in to user's saved volume setting
                    float targetVol = audioManager->GetBaseCategoryVolume("music") * audioManager->GetMasterVolume();
                    audioManager->FadeInCategory("music", 3.0f, targetVol);
                    //std::cout << "[MenuLogic] Main menu BGM restarted with 3 second fade in to volume " << targetVol << std::endl;
                }
                else
                {
                    std::cout << "[MenuLogic] ERROR: MainMenuBGM not found in sound database!" << std::endl;
                }
            }
            else
            {
                std::cout << "[MenuLogic] ERROR: AudioManager is null!" << std::endl;
            }

            creditsWerePlayed = false; // Reset
        }
    }

    // if a scene change is pending, count down timer first so the click sound can be played
    if (pendingSceneChange)
    {
        pendingTimer -= dt;
        if (pendingTimer <= 0.0f) // timer delay to play the sound effect before loading into scene
        {
            if (audioManager && audioManager->IsAnyFadeActive())
            {
                pendingTimer = 0.1f; // wait another 100ms and check again
                return;
            }

            if (currScreen == MenuScreens::Home)
            {
                if (hoveredButtonName == "PlayButton")
                {
                    std::cout << "PlayButton Clicked!" << std::endl;
                    currScreen = MenuScreens::LevelSelect;
                }
                else if (hoveredButtonName == "SettingsButton")
                {
                    std::cout << "SettingsButton Clicked!" << std::endl;
                    currScreen = MenuScreens::Settings;
                }
                else if (hoveredButtonName == "CreditsButton")
                {
                    std::cout << "CreditsButton Clicked!" << std::endl;
                    // to add credits screen here
                    CutsceneSystem *cutscene = CORE->GetCutsceneSystem();
                    if (!cutscene->IsPlaying())
                    {
                        cutscene->PlayVideo(creditsPath, "CreditsAudio");
                    }
                }
                else if (hoveredButtonName == "YesButton")
                {
                    std::cout << "YesButton Clicked! Quitting..." << std::endl;
#ifndef _DEBUG
                    glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
#else
                    CORE->GetLevelEditor()->StopPlayMode();
#endif
                }
            }

            else if (currScreen == MenuScreens::Home && !selectedLevelScenePath.empty() && !m_selectedReplay.empty())
            {
                CORE->RequestSceneLoad(selectedLevelScenePath, true);
                m_selectedReplay.clear();
                selectedLevelScenePath.clear();
            }
            else if (currScreen == MenuScreens::Leaderboard)
            {
                if (!selectedLevelScenePath.empty())
                {
                    CORE->RequestSceneLoad(selectedLevelScenePath, true);
                    selectedLevelScenePath.clear();
                    currScreen = MenuScreens::Home;
                }
            }
            else if (currScreen == MenuScreens::LevelSelect)
            {
                if (hoveredButtonName == "LevelSelectedButton" && !selectedLevelScenePath.empty())
                {
                    std::cout << "Starting level: " << selectedLevelScenePath << std::endl;
                    CutsceneSystem *cutscene = CORE->GetCutsceneSystem();

                    if (selectedLevelButtonName == "Level1Button" && !cutscene->HasPlayed(introPath))
                    {
                        if (!cutscene->IsPlaying())
                        {
                            cutscene->PlayVideo(introPath, "CutsceneAudio");
                            currScreen = MenuScreens::LevelSelect;
                            std::cout << "[MenuLogic] Playing cutscene, music remains faded out" << std::endl;
                        }
                    }
                    else
                    {
                        // No cutscene
                        CORE->RequestSceneLoad(selectedLevelScenePath, true);
                    }
                }
            }

            // clear
            pendingSceneChange = false;
            hoveredButtonName.clear();
        }

        return;
    }

    if (currScreen == MenuScreens::Home)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(true);
        }

        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
        {

            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(false);
        }

        // Hide settings text labels when not on settings screen
        if (auto *masterText = GameObjectFactory::Instance().FindGameObject("MasterText"))
            masterText->SetActive(false);
        if (auto *musicText = GameObjectFactory::Instance().FindGameObject("MusicText"))
            musicText->SetActive(false);
        if (auto *sfxText = GameObjectFactory::Instance().FindGameObject("SFXText"))
            sfxText->SetActive(false);
        if (auto *rText = GameObjectFactory::Instance().FindGameObject("Rating"))
            rText->SetActive(false);
        if (auto *rText2 = GameObjectFactory::Instance().FindGameObject("Rating(1)"))
            rText2->SetActive(false);
        if (auto *rText3 = GameObjectFactory::Instance().FindGameObject("Rating(2)"))
            rText3->SetActive(false);
        if (auto *lb = GameObjectFactory::Instance().FindGameObject("LeaderBoards_Menu"))
            lb->SetActive(false);
        if (auto *extra = GameObjectFactory::Instance().FindGameObject("Screen_Extra"))
            extra->SetActive(false);
        if (auto *htp = GameObjectFactory::Instance().FindGameObject("Screen_HTP"))
            htp->SetActive(false);
    }
    else if (currScreen == MenuScreens::Settings)
    {
        // hoveredButtonName.clear();

        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            auto *settingsScreen = GameObjectFactory::Instance().FindGameObject("Screen_Settings");
            if (!settingsScreen->IsActive())
            {
                settingsScreen->SetActive(true);
                justEnteredSettings = true;

                // load current volumes from Audio Manager
                audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    savedMasterVolume = audioManager->GetMasterVolume();
                    savedMusicVolume = audioManager->GetBaseCategoryVolume("music");
                    savedSfxVolume = audioManager->GetBaseCategoryVolume("sfx");
                    std::cout << "[MenuLogic] Entering Settings - Loaded volumes: Master=" << savedMasterVolume
                              << ", Music=" << savedMusicVolume << ", SFX=" << savedSfxVolume << std::endl;
                }

                // load saved volumes into slider states
                // std::cout << "[MenuLogic Enter Settings] Loading saved volumes - Master: " << savedMasterVolume
                //          << ", Music: " << savedMusicVolume << ", SFX: " << savedSfxVolume << std::endl;
                masterSlider.currentVolume = savedMasterVolume;
                musicSlider.currentVolume = savedMusicVolume;
                sfxSlider.currentVolume = savedSfxVolume;
            }
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
        {

            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(false);
        }
        if (auto *lb = GameObjectFactory::Instance().FindGameObject("LeaderBoards_Menu"))
            lb->SetActive(false);
        // Show settings text labels when on settings screen
        if (auto *masterText = GameObjectFactory::Instance().FindGameObject("MasterText"))
            masterText->SetActive(true);
        if (auto *musicText = GameObjectFactory::Instance().FindGameObject("MusicText"))
            musicText->SetActive(true);
        if (auto *sfxText = GameObjectFactory::Instance().FindGameObject("SFXText"))
            sfxText->SetActive(true);

        // update sliders based on saved base volumes
        if (audioManager)
        {
            // std::cout << "[MenuLogic Settings] Saved base volumes - Master: " << savedMasterVolume
            //           << ", Music: " << savedMusicVolume << ", SFX: " << savedSfxVolume
            //           << " | Slider states - Master: " << masterSlider.currentVolume
            //           << ", Music: " << musicSlider.currentVolume
            //           << ", SFX: " << sfxSlider.currentVolume << std::endl;

            // update if slider states don't match saved values OR if just entered settings
            bool needsUpdate = justEnteredSettings ||
                               std::abs(musicSlider.currentVolume - savedMusicVolume) > 0.01f ||
                               std::abs(sfxSlider.currentVolume - savedSfxVolume) > 0.01f ||
                               std::abs(masterSlider.currentVolume - savedMasterVolume) > 0.01f;
            if (needsUpdate)
            {
                masterSlider.currentVolume = savedMasterVolume;
                musicSlider.currentVolume = savedMusicVolume;
                sfxSlider.currentVolume = savedSfxVolume;

                // update visual sliders
                UpdateSlider("Slider_Bar_Master", masterSlider, masterSlider.currentVolume);
                UpdateSlider("Slider_Bar_Music", musicSlider, musicSlider.currentVolume);
                UpdateSlider("Slider_Bar_SFX", sfxSlider, sfxSlider.currentVolume);

                // update knob(aka scissors) positions to matc

                // master knob
                GameObject *masterBar = factory.FindGameObject("Slider_Bar_Master");
                GameObject *masterKnob = factory.FindGameObject("Slider_Knob_Master");
                if (masterBar && masterKnob)
                {
                    auto *barTransform = masterBar->GetComponent<TransformComponent>();
                    auto *knobTransform = masterKnob->GetComponent<TransformComponent>();
                    if (barTransform && knobTransform)
                    {
                        float barCenterX, barCenterY, barScaleX, barScaleY;
                        barTransform->GetPosition(barCenterX, barCenterY);
                        barTransform->GetScale(barScaleX, barScaleY);
                        float knobX = barCenterX + barScaleX * 0.5f - 285.0f;
                        knobTransform->SetPosition(knobX, barCenterY);
                    }
                }

                // music knob
                GameObject *musicBar = factory.FindGameObject("Slider_Bar_Music");
                GameObject *musicKnob = factory.FindGameObject("Slider_Knob_Music");
                if (musicBar && musicKnob)
                {
                    auto *barTransform = musicBar->GetComponent<TransformComponent>();
                    auto *knobTransform = musicKnob->GetComponent<TransformComponent>();
                    if (barTransform && knobTransform)
                    {
                        float barCenterX, barCenterY, barScaleX, barScaleY;
                        barTransform->GetPosition(barCenterX, barCenterY);
                        barTransform->GetScale(barScaleX, barScaleY);
                        float knobX = barCenterX + barScaleX * 0.5f - 285.0f;
                        knobTransform->SetPosition(knobX, barCenterY);
                    }
                }

                // SFX knob
                GameObject *sfxBar = factory.FindGameObject("Slider_Bar_SFX");
                GameObject *sfxKnob = factory.FindGameObject("Slider_Knob_SFX");
                if (sfxBar && sfxKnob)
                {
                    auto *barTransform = sfxBar->GetComponent<TransformComponent>();
                    auto *knobTransform = sfxKnob->GetComponent<TransformComponent>();
                    if (barTransform && knobTransform)
                    {
                        float barCenterX, barCenterY, barScaleX, barScaleY;
                        barTransform->GetPosition(barCenterX, barCenterY);
                        barTransform->GetScale(barScaleX, barScaleY);
                        float knobX = barCenterX + barScaleX * 0.5f - 285.0f;
                        knobTransform->SetPosition(knobX, barCenterY);
                    }
                }
            }
        }
    }

    else if (currScreen == MenuScreens::LevelSelect)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        if (auto *levelSelectScreen = GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
        {
            if (!levelSelectScreen->IsActive()) // only true on the first frame
            {
                justEnteredLevelSelect = true;
            }
            levelSelectScreen->SetActive(true);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(false);
    }
    else if (currScreen == MenuScreens::QUIT)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(true);
        }
        if (auto *lb = GameObjectFactory::Instance().FindGameObject("LeaderBoards_Menu"))
            lb->SetActive(false);
    }
    else if (currScreen == MenuScreens::Leaderboard)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
            GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(false);
        if (auto *lb = GameObjectFactory::Instance().FindGameObject("LeaderBoards_Menu"))
            lb->SetActive(true);
        if (auto *extra = GameObjectFactory::Instance().FindGameObject("Screen_Extra"))
            extra->SetActive(false);
    }
    else if (currScreen == MenuScreens::Extra)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
            GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(false);
        if (auto *lb = GameObjectFactory::Instance().FindGameObject("LeaderBoards_Menu"))
            lb->SetActive(false);
        if (auto *htp = GameObjectFactory::Instance().FindGameObject("Screen_HTP"))
            htp->SetActive(false);
        if (auto *extra = GameObjectFactory::Instance().FindGameObject("Screen_Extra"))
            extra->SetActive(true);
    }
    else if (currScreen == MenuScreens::HTP)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
            GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(false);
        if (auto *lb = GameObjectFactory::Instance().FindGameObject("LeaderBoards_Menu"))
            lb->SetActive(false);
        if (auto *extra = GameObjectFactory::Instance().FindGameObject("Screen_Extra"))
            extra->SetActive(false);
        if (auto *htp = GameObjectFactory::Instance().FindGameObject("Screen_HTP"))
            htp->SetActive(true);
    }

    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();
    bool isMouseDown = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // handle slider input when on settings screen
    if (currScreen == MenuScreens::Settings)
    {
        if (!justEnteredSettings)
        {
            HandleSliderInput(mouseX, mouseY, isMouseDown);
        }
        else
        {
            // First frame entering settings - update mouse state but don't process input
            // This prevents the next frame from thinking the mouse was just clicked
            wasMouseDownOnSlider = isMouseDown;
            isDraggingSlider = false;
            currentSliderCategory = "";
            justEnteredSettings = false;
        }
    }

    if (currScreen == MenuScreens::LevelSelect && justEnteredLevelSelect)
    {
        if (auto *selectedLevel1 = GameObjectFactory::Instance().FindGameObject("Level_select_selected"))
            selectedLevel1->SetActive(false);
        if (auto *selectedLevel2 = GameObjectFactory::Instance().FindGameObject("Level_select_selected(1)"))
            selectedLevel2->SetActive(false);
        if (auto *selectedLevel3 = GameObjectFactory::Instance().FindGameObject("Level_select_selected(2)"))
            selectedLevel3->SetActive(false);

        justEnteredLevelSelect = false;
    }

    std::string currentHover = FindHoveredButton(mouseX, mouseY);

    // Keyboard selection: UP/DOWN to navigate buttons, SPACE to confirm
    bool spacePressed = false;
    {
        static MenuScreens kbPrevScreen = MenuScreens::Home;
        static int  kbIdx    = 0;
        static bool kbActive = false;

        // Reset when the screen changes
        if (currScreen != kbPrevScreen) { kbIdx = 0; kbActive = false; kbPrevScreen = currScreen; }

        // Deactivate nav when not on the main menu (prevents stale selection from
        // firing SPACE-confirm while the player is inside a level)
        {
            IGameCallbacksSystem* gameCb = CORE ? CORE->GetGameCallbacks() : nullptr;
            if (!gameCb || !gameCb->IsOnMainMenu()) { kbActive = false; }
        }

        static const char* homeButtons[]        = { "PlayButton", "SettingsButton", "ExtraButton", "QuitButton" };
        static const char* quitButtons[]        = { "YesButton", "NoButton" };
        static const char* settingsButtons[]    = { "MasterMuteButton", "MusicMuteButton", "SFXMuteButton", "BackButton" };
        static const char* levelSelectButtons[] = { "Level1Button", "Level2Button", "Level3Button", "LevelSelectedButton", "Level_BackButton" };
        static const char* extraButtons[]       = { "HowToPlay_Button", "Leaderboard_Button", "Credit_Button", "BackButton_Extra" };
        static const char* htpButtons[]         = { "BackButton_HTP" };
        static const char* leaderboardButtons[] = { "Leaderboards_tab_1st", "Leaderboards_tab_2nd", "Leaderboards_tab_3rd", "ReplayButton", "Vs_Ghost_Button", "BackButton" };

        const char** kbButtons = nullptr;
        int kbCount = 0;
        switch (currScreen)
        {
        case MenuScreens::Home:        kbButtons = homeButtons;        kbCount = 4; break;
        case MenuScreens::QUIT:        kbButtons = quitButtons;        kbCount = 2; break;
        case MenuScreens::Settings:    kbButtons = settingsButtons;    kbCount = 4; break;
        case MenuScreens::LevelSelect: kbButtons = levelSelectButtons; kbCount = 5; break;
        case MenuScreens::Extra:       kbButtons = extraButtons;       kbCount = 4; break;
        case MenuScreens::HTP:         kbButtons = htpButtons;         kbCount = 1; break;
        case MenuScreens::Leaderboard: kbButtons = leaderboardButtons; kbCount = 6; break;
        default: break;
        }

        if (kbCount > 0)
        {
            InputManager* inp = CORE->GetInputManager();
            if (inp)
            {
                if (inp->IsKeyPressed(GLFW_KEY_DOWN)) { kbIdx = (kbIdx + 1) % kbCount;              kbActive = true; }
                if (inp->IsKeyPressed(GLFW_KEY_UP))   { kbIdx = ((kbIdx - 1) + kbCount) % kbCount;  kbActive = true; }
                // D-pad maps to the same navigation as arrow keys
                if (inp->IsGamepadConnected())
                {
                    if (inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN)) { kbIdx = (kbIdx + 1) % kbCount;              kbActive = true; }
                    if (inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP))   { kbIdx = ((kbIdx - 1) + kbCount) % kbCount;  kbActive = true; }
                }
                if (isClicked && !currentHover.empty()) kbActive = false;
                // Deselect keyboard nav when mouse interacts with a slider
                if (isMouseDown && isDraggingSlider) kbActive = false;
                if (kbActive) currentHover = kbButtons[kbIdx];
                spacePressed = kbActive && (inp->IsKeyPressed(GLFW_KEY_SPACE) ||
                    (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A)));

                // adjust slider when on Settings screen and a row is selected
                static float kMenuSliderRepeatTimer = 0.0f;
                if (currScreen == MenuScreens::Settings && kbActive && kbIdx <= 2) // 0=Master,1=Music,2=SFX
                {
                    const float kRepeatDelay = 0.3f;
                    const float kRepeatRate  = 0.08f;

                    bool leftHeld  = inp->IsKeyHeld(GLFW_KEY_LEFT)  || inp->IsKeyPressed(GLFW_KEY_LEFT)
                                   || (inp->IsGamepadConnected() && inp->IsGamepadButtonHeld(GLFW_GAMEPAD_BUTTON_DPAD_LEFT));
                    bool rightHeld = inp->IsKeyHeld(GLFW_KEY_RIGHT) || inp->IsKeyPressed(GLFW_KEY_RIGHT)
                                   || (inp->IsGamepadConnected() && inp->IsGamepadButtonHeld(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT));
                    bool leftJust  = inp->IsKeyPressed(GLFW_KEY_LEFT)
                                   || (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT));
                    bool rightJust = inp->IsKeyPressed(GLFW_KEY_RIGHT)
                                   || (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT));

                    bool shouldStep = false;
                    if (leftJust || rightJust)
                    {
                        shouldStep = true;
                        kMenuSliderRepeatTimer = 0.0f;
                    }
                    else if (leftHeld || rightHeld)
                    {
                        kMenuSliderRepeatTimer += dt;
                        if (kMenuSliderRepeatTimer >= kRepeatDelay)
                        {
                            float excess     = kMenuSliderRepeatTimer - kRepeatDelay;
                            float prevExcess = excess - dt;
                            if (prevExcess < 0.0f || static_cast<int>(excess / kRepeatRate) > static_cast<int>(prevExcess / kRepeatRate))
                                shouldStep = true;
                        }
                    }
                    else
                    {
                        kMenuSliderRepeatTimer = 0.0f;
                    }

                    if (shouldStep)
                    {
                        const float kStep = 0.05f;
                        float delta = rightHeld ? kStep : -kStep;

                        SliderState* sliderState = nullptr;
                        std::string barName;
                        std::string category;

                        if (kbIdx == 0)      { sliderState = &masterSlider; barName = "Slider_Bar_Master"; category = "master"; }
                        else if (kbIdx == 1) { sliderState = &musicSlider;  barName = "Slider_Bar_Music";  category = "music"; }
                        else                 { sliderState = &sfxSlider;    barName = "Slider_Bar_SFX";    category = "sfx"; }

                        float newVol = std::clamp(sliderState->currentVolume + delta, 0.0f, 1.0f);
                        sliderState->currentVolume = newVol;
                        UpdateSlider(barName, *sliderState, newVol);

                        // Reposition scissors icon
                        std::string knobName;
                        if (kbIdx == 0)      knobName = "Slider_Knob_Master";
                        else if (kbIdx == 1) knobName = "Slider_Knob_Music";
                        else                 knobName = "Slider_Knob_SFX";

                        GameObject* sliderBarGO = factory.FindGameObject(barName);
                        GameObject* knob        = factory.FindGameObject(knobName);
                        if (sliderBarGO && knob)
                        {
                            auto* st = sliderBarGO->GetComponent<TransformComponent>();
                            auto* kt = knob->GetComponent<TransformComponent>();
                            if (st && kt)
                            {
                                float cx, cy, sx, sy;
                                st->GetPosition(cx, cy);
                                st->GetScale(sx, sy);
                                kt->SetPosition(cx + sx * 0.5f - 285.0f, cy);
                            }
                        }

                        // Apply to audio
                        if (audioManager)
                        {
                            if (category == "master")
                            {
                                savedMasterVolume = newVol;
                                audioManager->SetMasterVolume(newVol);
                                audioManager->SetCategoryVolume("music", savedMusicVolume * newVol);
                                audioManager->SetCategoryVolume("sfx",   savedSfxVolume   * newVol);
                            }
                            else if (category == "music")
                            {
                                savedMusicVolume = newVol;
                                audioManager->SetBaseCategoryVolume("music", newVol);
                                audioManager->SetCategoryVolume("music", newVol * savedMasterVolume);
                            }
                            else
                            {
                                savedSfxVolume = newVol;
                                audioManager->SetBaseCategoryVolume("sfx", newVol);
                                audioManager->SetCategoryVolume("sfx", newVol * savedMasterVolume);
                            }
                        }
                    }
                }
                else if (currScreen == MenuScreens::Leaderboard)
                {
                    // cycle through Level 1 / 2 / 3 tabs
                    bool leftTab  = inp->IsKeyPressed(GLFW_KEY_LEFT)  || (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT));
                    bool rightTab = inp->IsKeyPressed(GLFW_KEY_RIGHT) || (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT));
                    if (leftTab)
                    {
                        int newLevel = ((m_leaderboardLevel - 2 + 3) % 3) + 1;
                        const char* lvlBtns[] = { "Leaderboards_Level_1", "Leaderboards_Level_2", "Leaderboards_Level_3" };
                        HandleImmediateButtonClick(lvlBtns[newLevel - 1]);
                    }
                    else if (rightTab)
                    {
                        int newLevel = (m_leaderboardLevel % 3) + 1;
                        const char* lvlBtns[] = { "Leaderboards_Level_1", "Leaderboards_Level_2", "Leaderboards_Level_3" };
                        HandleImmediateButtonClick(lvlBtns[newLevel - 1]);
                    }
                    kMenuSliderRepeatTimer = 0.0f;
                }
                else
                {
                    kMenuSliderRepeatTimer = 0.0f;
                }
            }
        }
    }

    std::string currentPressed = "";

    // Check if mouse is being pressed on the hovered button
    if (isMouseDown && !currentHover.empty())
    {
        currentPressed = currentHover;
    }

    // If the pressed button has changed since last frame
    if (currentPressed != pressedButtonName)
    {
        // Revert the PREVIOUS pressed button texture
        if (!pressedButtonName.empty())
        {
            // Check if still hovering the old pressed button
            bool stillHovering = (pressedButtonName == hoveredButtonName);
            SetButtonTexture(pressedButtonName, stillHovering, false);
        }

        // Update tracked pressed state
        pressedButtonName = currentPressed;
    }

    // If the hovered button has changed since last frame
    if (currentHover != hoveredButtonName)
    {
        // Revert the PREVIOUS button texture to normal
        if (!hoveredButtonName.empty() && hoveredButtonName != pressedButtonName)
        {
            SetButtonTexture(hoveredButtonName, false, false);
        }

        if (!currentHover.empty())
        {
            // If pressed, show pressed texture, otherwise show hover texture
            bool isCurrentPressed = (isMouseDown && currentPressed == currentHover);
            SetButtonTexture(currentHover, true, isCurrentPressed);
            // Play hover sound once when entering the button

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Hover");
            }
        }

        // Update tracked state
        hoveredButtonName = currentHover;
    }
    else if (!currentHover.empty())
    {
        // Even if hover didn't change, update texture if pressed state changed
        bool isCurrentPressed = (isMouseDown && currentPressed == currentHover);
        bool wasPressed = (!isCurrentPressed && pressedButtonName == currentHover);
        if (isCurrentPressed != wasPressed)
        {
            SetButtonTexture(currentHover, true, isCurrentPressed);
        }
    }

    // Only process clicks if we are actually hovering over a button
    if ((isClicked || spacePressed) && !hoveredButtonName.empty())
    {
        const std::string button = hoveredButtonName;

        // Handle level select screen buttons first
        if (HandleLevelSelectClick(button))
        {
            return;
        }

        // Play general UI sound
        TriggerUISound(button);

        // Handle buttons that act immediately
        if (HandleImmediateButtonClick(button))
        {
            return;
        }

        // Delayed transition buttons
        pendingSceneChange = true;
        pendingTimer = 2.0f;
    }
}

std::string MenuLogicSystem::FindHoveredButton(double mouseX, double mouseY)
{
    auto &factory = GameObjectFactory::Instance();

    switch (currScreen)
    {
    case MenuScreens::Home:
        if (IsMouseOver(factory.FindGameObject("PlayButton"), mouseX, mouseY))
            return "PlayButton";
        if (IsMouseOver(factory.FindGameObject("SettingsButton"), mouseX, mouseY))
            return "SettingsButton";
        if (IsMouseOver(factory.FindGameObject("CreditsButton"), mouseX, mouseY))
            return "CreditsButton";
        if (IsMouseOver(factory.FindGameObject("QuitButton"), mouseX, mouseY))
            return "QuitButton";
        if (IsMouseOver(factory.FindGameObject("ReplayButton"), mouseX, mouseY))
            return "ReplayButton";
        if (IsMouseOver(factory.FindGameObject("LeaderboardButton"), mouseX, mouseY))
            return "LeaderboardButton";
        if (IsMouseOver(factory.FindGameObject("ExtraButton"), mouseX, mouseY))
            return "ExtraButton";
        break;

    case MenuScreens::QUIT:
        if (IsMouseOver(factory.FindGameObject("YesButton"), mouseX, mouseY))
            return "YesButton";
        if (IsMouseOver(factory.FindGameObject("NoButton"), mouseX, mouseY))
            return "NoButton";
        break;

    case MenuScreens::Settings:
        if (IsMouseOver(factory.FindGameObject("MasterMuteButton"), mouseX, mouseY))
            return "MasterMuteButton";
        if (IsMouseOver(factory.FindGameObject("MusicMuteButton"), mouseX, mouseY))
            return "MusicMuteButton";
        if (IsMouseOver(factory.FindGameObject("SFXMuteButton"), mouseX, mouseY))
            return "SFXMuteButton";
        if (IsMouseOver(factory.FindGameObject("BackButton"), mouseX, mouseY))
            return "BackButton";
        break;

    case MenuScreens::LevelSelect:
        if (IsMouseOver(factory.FindGameObject("Level1Button"), mouseX, mouseY))
            return "Level1Button";
        if (IsMouseOver(factory.FindGameObject("Level2Button"), mouseX, mouseY))
            return "Level2Button";
        if (IsMouseOver(factory.FindGameObject("Level3Button"), mouseX, mouseY))
            return "Level3Button";
        if (IsMouseOver(factory.FindGameObject("LevelSelectedButton"), mouseX, mouseY))
            return "LevelSelectedButton";
        if (IsMouseOver(factory.FindGameObject("Level_BackButton"), mouseX, mouseY))
            return "Level_BackButton";
        break;

    case MenuScreens::Leaderboard:
        // Level tabs
        if (IsMouseOver(factory.FindGameObject("Leaderboards_Level_1"), mouseX, mouseY))
            return "Leaderboards_Level_1";
        if (IsMouseOver(factory.FindGameObject("Leaderboards_Level_2"), mouseX, mouseY))
            return "Leaderboards_Level_2";
        if (IsMouseOver(factory.FindGameObject("Leaderboards_Level_3"), mouseX, mouseY))
            return "Leaderboards_Level_3";
        // Rank rows (clickable to select a player)
        if (IsMouseOver(factory.FindGameObject("Leaderboards_tab_1st"), mouseX, mouseY))
            return "Leaderboards_tab_1st";
        if (IsMouseOver(factory.FindGameObject("Leaderboards_tab_2nd"), mouseX, mouseY))
            return "Leaderboards_tab_2nd";
        if (IsMouseOver(factory.FindGameObject("Leaderboards_tab_3rd"), mouseX, mouseY))
            return "Leaderboards_tab_3rd";
        // Action buttons — iterate all GOs so we find the leaderboard's ReplayButton (not home screen's)
        for (auto &obj : factory.GetAllGameObjects())
        {
            if ((obj->GetName() == "ReplayButton" || obj->GetName() == "Vs_Ghost_Button" ||
                 obj->GetName() == "BackButton") &&
                IsMouseOver(obj.get(), mouseX, mouseY))
                return obj->GetName();
        }
        break;

    case MenuScreens::Extra:
        if (IsMouseOver(factory.FindGameObject("HowToPlay_Button"), mouseX, mouseY))
            return "HowToPlay_Button";
        if (IsMouseOver(factory.FindGameObject("Leaderboard_Button"), mouseX, mouseY))
            return "Leaderboard_Button";
        if (IsMouseOver(factory.FindGameObject("Credit_Button"), mouseX, mouseY))
            return "Credit_Button";
        if (IsMouseOver(factory.FindGameObject("BackButton_Extra"), mouseX, mouseY))
            return "BackButton_Extra";
        break;

    case MenuScreens::HTP:
        if (IsMouseOver(factory.FindGameObject("BackButton_HTP"), mouseX, mouseY))
            return "BackButton_HTP";
        break;
    }

    return "";
}

bool MenuLogicSystem::IsMouseOver(GameObject *button, double mouseX, double mouseY)
{
    if (!button)
        return false;
    auto *tx = button->GetComponent<TransformComponent>();
    if (!tx)
        return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

void MenuLogicSystem::TriggerUISound(const std::string &buttonName)
{
    AudioManager *audio = CORE->GetAudioManager();
    if (!audio)
        return;

    if (buttonName == "LevelSelectedButton")
    {
        audio->TriggerEvent("UI_Press");
    }
    else if (buttonName == "QuitButton")
    {
        audio->TriggerEvent("UI_Quit");
    }
    else
    {
        audio->TriggerEvent("UI_Click");
    }

    std::cout << "menu button clicking sound is triggered" << std::endl;
}

void MenuLogicSystem::RefreshLevelSelectButtons()
{
    SetButtonTexture("Level1Button",
                     hoveredButtonName == "Level1Button",
                     pressedButtonName == "Level1Button");

    SetButtonTexture("Level2Button",
                     hoveredButtonName == "Level2Button",
                     pressedButtonName == "Level2Button");

    SetButtonTexture("Level3Button",
                     hoveredButtonName == "Level3Button",
                     pressedButtonName == "Level3Button");

    SetButtonTexture("LevelSelectedButton",
                     hoveredButtonName == "LevelSelectedButton",
                     pressedButtonName == "LevelSelectedButton");
}

void MenuLogicSystem::SelectLevel(const std::string &buttonName, const std::string &scenePath)
{
    selectedLevelButtonName = buttonName;
    selectedLevelScenePath = scenePath;
    RefreshLevelSelectButtons();
}
bool MenuLogicSystem::HandleLevelSelectClick(const std::string &buttonName)
{
    if (currScreen != MenuScreens::LevelSelect)
        return false;

    if (buttonName == "Level1Button")
    {
        TriggerUISound(buttonName);
        SelectLevel("Level1Button", level1Path);
        if (auto *selectedLevel1 = GameObjectFactory::Instance().FindGameObject("Level_select_selected"))
            selectedLevel1->SetActive(true);
        if (auto *selectedLevel2 = GameObjectFactory::Instance().FindGameObject("Level_select_selected(1)"))
            selectedLevel2->SetActive(false);
        if (auto *selectedLevel3 = GameObjectFactory::Instance().FindGameObject("Level_select_selected(2)"))
            selectedLevel3->SetActive(false);
        return true;
    }

    if (buttonName == "Level2Button")
    {
        TriggerUISound(buttonName);
        SelectLevel("Level2Button", level2Path);
        if (auto *selectedLevel1 = GameObjectFactory::Instance().FindGameObject("Level_select_selected"))
            selectedLevel1->SetActive(false);
        if (auto *selectedLevel2 = GameObjectFactory::Instance().FindGameObject("Level_select_selected(1)"))
            selectedLevel2->SetActive(true);
        if (auto *selectedLevel3 = GameObjectFactory::Instance().FindGameObject("Level_select_selected(2)"))
            selectedLevel3->SetActive(false);
        return true;
    }

    if (buttonName == "Level3Button")
    {
        TriggerUISound(buttonName);
        SelectLevel("Level3Button", level3Path);
        if (auto *selectedLevel1 = GameObjectFactory::Instance().FindGameObject("Level_select_selected"))
            selectedLevel1->SetActive(false);
        if (auto *selectedLevel2 = GameObjectFactory::Instance().FindGameObject("Level_select_selected(1)"))
            selectedLevel2->SetActive(false);
        if (auto *selectedLevel3 = GameObjectFactory::Instance().FindGameObject("Level_select_selected(2)"))
            selectedLevel3->SetActive(true);
        return true;
    }

    if (buttonName == "LevelSelectedButton")
    {
        // Start only works after a level has been selected.
        if (selectedLevelButtonName.empty() || selectedLevelScenePath.empty())
        {
            SetButtonTexture("LevelSelectedButton", false, false);
            return true;
        }

        TriggerUISound(buttonName);
        pendingSceneChange = true;
        pendingTimer = 0.2f;
        return true;
    }

    if (buttonName == "Level_BackButton")
    {
        TriggerUISound(buttonName);
        currScreen = MenuScreens::Home;
        ClearLevelSelection();
        return true;
    }

    return false;
}

bool MenuLogicSystem::HandleImmediateButtonClick(const std::string &buttonName)
{
    AudioManager *audioManager = CORE->GetAudioManager();

    if (buttonName == "MasterMuteButton")
    {
        if (audioManager)
        {
            audioManager->ToggleMasterMute();
        }

        SetButtonTexture("MasterMuteButton", true, false);
        return true;
    }

    if (buttonName == "MusicMuteButton")
    {
        if (audioManager)
        {
            audioManager->ToggleCategoryMute("music");
        }

        SetButtonTexture("MusicMuteButton", true, false);
        return true;
    }

    if (buttonName == "SFXMuteButton")
    {
        if (audioManager)
        {
            audioManager->ToggleCategoryMute("sfx");
        }

        SetButtonTexture("SFXMuteButton", true, false);
        return true;
    }

    if (buttonName == "BackButton")
    {
        if (currScreen == MenuScreens::Leaderboard)
            currScreen = MenuScreens::Extra;
        else
            currScreen = MenuScreens::Home; // Settings -> Home
        return true;
    }

    if (buttonName == "BackButton_HTP")
    {
        currScreen = MenuScreens::Extra;
        return true;
    }

    if (buttonName == "BackButton_Extra")
    {
        currScreen = MenuScreens::Home;
        return true;
    }

    if (buttonName == "HowToPlay_Button")
    {
        currScreen = MenuScreens::HTP;
        return true;
    }

    if (buttonName == "Leaderboard_Button")
    {
        m_leaderboardLevel = 1;
        m_selectedRank = -1;
        ScanLeaderboard(m_leaderboardLevel);
        PopulateLeaderboard();
        UpdateLeaderboardTextures();
        currScreen = MenuScreens::Leaderboard;
        return true;
    }

    if (buttonName == "Credit_Button")
    {
        CutsceneSystem *cutscene = CORE->GetCutsceneSystem();
        if (cutscene && !cutscene->IsPlaying())
        {
            audioManager->FadeOutCategory("music", 2.5f);
            cutscene->PlayVideo(creditsPath, "CreditsAudio");
            creditsWerePlayed = true;
        }
        return true;
    }

    if (buttonName == "PlayButton")
    {
        currScreen = MenuScreens::LevelSelect;
        return true;
    }

    if (buttonName == "SettingsButton")
    {
        currScreen = MenuScreens::Settings;
        return true;
    }
    if (buttonName == "ExtraButton") {
        currScreen = MenuScreens::Extra;
        return true;
    }

    if (buttonName == "CreditsButton")
    {
        std::cout << "[MenuLogic] Leaderboard opened via CreditsButton." << std::endl;
        return true;
    }

    if (buttonName == "QuitButton")
    {
        currScreen = MenuScreens::QUIT;
        return true;
    }

    if (buttonName == "YesButton")
    {
        std::cout << "YesButton Clicked! Quitting..." << std::endl;
#ifndef _DEBUG
        glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
#else
        CORE->GetLevelEditor()->StopPlayMode();
#endif
        return true;
    }

    if (buttonName == "NoButton")
    {
        currScreen = MenuScreens::Home;
        return true;
    }

    if (buttonName == "ReplayButton" && currScreen == MenuScreens::Home)
    {
        // Scan all replay folders and load the most recent replay directly
        ScanReplays();
        if (!m_replayNames.empty())
        {
            m_selectedReplay = m_replayNames[0]; // most recent
            std::cout << "[MenuLogic] ReplayButton: loading most recent replay: " << m_selectedReplay << std::endl;
            if (GhostSystem *ghost = CORE->GetGhostSystem())
            {
                ghost->Configure("Hero", "GhostHero", "Hero_vfx", "GhostHero_vfx");
                ghost->RequestReplayOnLoad(m_selectedReplay);
            }
            if (audioManager)
                audioManager->FadeOutCategory("music", 2.5f);
            pendingSceneChange = true;
            pendingTimer = 2.6f;
            selectedLevelScenePath = "../Assets/Scenes/Level_1.json";
        }
        else
        {
            std::cout << "[MenuLogic] ReplayButton: no replays found." << std::endl;
        }
        return true;
    }

    // ===== Leaderboard =====
    if (buttonName == "LeaderboardButton")
    {
        std::cout << "[MenuLogic] Leaderboard opened." << std::endl;
        m_leaderboardLevel = 1;
        m_selectedRank = -1;
        ScanLeaderboard(m_leaderboardLevel);
        PopulateLeaderboard();
        UpdateLeaderboardTextures();
        currScreen = MenuScreens::Leaderboard;
        return true;
    }

    if (buttonName == "Leaderboards_Level_1")
    {
        m_leaderboardLevel = 1;
        m_selectedRank = -1;
        ScanLeaderboard(m_leaderboardLevel);
        PopulateLeaderboard();
        UpdateLeaderboardTextures();
        return true;
    }

    if (buttonName == "Leaderboards_Level_2")
    {
        m_leaderboardLevel = 2;
        m_selectedRank = -1;
        ScanLeaderboard(m_leaderboardLevel);
        PopulateLeaderboard();
        UpdateLeaderboardTextures();
        return true;
    }

    if (buttonName == "Leaderboards_Level_3")
    {
        m_leaderboardLevel = 3;
        m_selectedRank = -1;
        ScanLeaderboard(m_leaderboardLevel);
        PopulateLeaderboard();
        UpdateLeaderboardTextures();
        return true;
    }

    // Rank row selection (toggle)
    if (buttonName == "Leaderboards_tab_1st" || buttonName == "Leaderboards_tab_2nd" || buttonName == "Leaderboards_tab_3rd")
    {
        int clicked = (buttonName == "Leaderboards_tab_1st") ? 0 : (buttonName == "Leaderboards_tab_2nd") ? 1
                                                                                                          : 2;
        // Toggle off if already selected, otherwise select
        m_selectedRank = (m_selectedRank == clicked) ? -1 : clicked;
        UpdateLeaderboardTextures();
        return true;
    }

    // Leaderboard Replay button — watch selected player's replay
    if (buttonName == "ReplayButton" && currScreen == MenuScreens::Leaderboard)
    {
        if (m_selectedRank < 0 || m_selectedRank >= static_cast<int>(m_leaderboardData.size()))
        {
            std::cout << "[MenuLogic] Replay: no player selected." << std::endl;
            return true;
        }
        const std::string replayName = m_leaderboardData[m_selectedRank].name;
        const std::string levelScene = "../Assets/Scenes/Level_" + std::to_string(m_leaderboardLevel) + ".json";
        std::cout << "[MenuLogic] Watching replay: " << replayName << " (Level " << m_leaderboardLevel << ")" << std::endl;
        if (GhostSystem *ghost = CORE->GetGhostSystem())
        {
            ghost->Configure("Hero", "GhostHero", "Hero_vfx", "GhostHero_vfx");
            ghost->SetLevelName("Level_" + std::to_string(m_leaderboardLevel));
            ghost->RequestReplayOnLoad(replayName);
        }
        if (audioManager)
            audioManager->FadeOutCategory("music", 2.5f);
        pendingSceneChange = true;
        pendingTimer = 2.6f;
        // Store scene path so the pendingSceneChange block can load it
        selectedLevelScenePath = levelScene;
        return true;
    }

    // Leaderboard Vs Ghost button — play the level with selected player as ghost opponent
    if (buttonName == "Vs_Ghost_Button")
    {
        if (m_selectedRank < 0 || m_selectedRank >= static_cast<int>(m_leaderboardData.size()))
        {
            std::cout << "[MenuLogic] Vs Ghost: no player selected." << std::endl;
            return true;
        }
        const std::string replayName = m_leaderboardData[m_selectedRank].name;
        const std::string levelScene = "../Assets/Scenes/Level_" + std::to_string(m_leaderboardLevel) + ".json";
        std::cout << "[MenuLogic] Vs Ghost: " << replayName << " (Level " << m_leaderboardLevel << ")" << std::endl;
        if (GhostSystem *ghost = CORE->GetGhostSystem())
        {
            ghost->Configure("Hero", "GhostHero", "Hero_vfx", "GhostHero_vfx");
            ghost->SetLevelName("Level_" + std::to_string(m_leaderboardLevel));
            ghost->RequestVsGhostOnLoad(replayName);
        }
        if (audioManager)
            audioManager->FadeOutCategory("music", 2.5f);
        pendingSceneChange = true;
        pendingTimer = 2.6f;
        selectedLevelScenePath = levelScene;
        return true;
    }

    return false;
}

//  swap textures
void MenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered, bool isPressed)
{
    // Leaderboard tabs use selection-only textures managed by UpdateLeaderboardTextures().
    // Do NOT apply hover/press effects to them.
    if (buttonName == "Leaderboards_Level_1" || buttonName == "Leaderboards_Level_2" ||
        buttonName == "Leaderboards_Level_3" || buttonName == "Leaderboards_tab_1st" ||
        buttonName == "Leaderboards_tab_2nd" || buttonName == "Leaderboards_tab_3rd")
        return;

    auto &factory = GameObjectFactory::Instance();

    // Multiple GOs may share the name "BackButton" (Settings + Leaderboard).
    // Always operate on the active one so the correct instance gets updated.
    GameObject *btn = nullptr;
    if (buttonName == "BackButton")
    {
        for (auto &obj : factory.GetAllGameObjects())
        {
            if (obj->GetName() == "BackButton" && obj->IsActive())
            {
                btn = obj.get();
                break;
            }
        }
    }
    else
    {
        btn = factory.FindGameObject(buttonName);
    }
    if (!btn)
        return;

    auto *rc = btn->GetComponent<RenderComponent>();
    if (!rc)
        return;

    std::string texturePath;

    if (buttonName == "PlayButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_play_NEW_HOVER.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_play_NEW_bright.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Menu_play.png";
        }
    }
    else if (buttonName == "SettingsButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_settings_NEW_HOVER.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_settings_NEW_bright.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Menu_settings.png";
        }
    }
    else if (buttonName == "QuitButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_quit_NEW_HOVER.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_quit_NEW_bright.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Menu_quit.png";
        }
    }

    else if (buttonName == "ReplayButton" || buttonName == "Vs_Ghost_Button")
    {
        if (isPressed)
            texturePath = "../Assets/Sprites/Icons/Hovering_button_light.png";
        else if (isHovered)
            texturePath = "../Assets/Sprites/Icons/Hovering_button_dark.png";
        else
            texturePath = "../Assets/Sprites/Icons/blankbtn.png";
    }

    else if (buttonName == "CreditsButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_credits_NEW_HOVER.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_credits_NEW_bright.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Menu_credits_NEW.png";
        }
    }
    else if (buttonName == "ExtraButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_Extras_dark.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_Extras_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Menu_Extras.png";
        }
    }
    else if (buttonName == "HowToPlay_Button" || buttonName == "Leaderboard_Button" || buttonName == "Credit_Button")
    {
        if (isPressed)
            texturePath = "../Assets/Sprites/Icons/Hovering_button_dark.png";
        else if (isHovered)
            texturePath = "../Assets/Sprites/Icons/Hovering_button_light.png";
        else
            texturePath = "../Assets/Sprites/Icons/blankbtn.png";
    }
    else if (buttonName == "MasterMuteButton")
    {
        bool isMuted = false;
        AudioManager *audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsMuted();
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "MusicMuteButton")
    {
        bool isMuted = false;
        AudioManager *audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsCategoryMuted("music");
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "SFXMuteButton")
    {
        bool isMuted = false;
        AudioManager *audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsCategoryMuted("sfx");
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "YesButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Hovering_button_dark.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Hovering_button_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/blankbtn.png";
        }
    }
    else if (buttonName == "NoButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Hovering_button_dark.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Hovering_button_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/blankbtn.png";
        }
    }
    else if (buttonName == "BackButton" || buttonName == "Level_BackButton" ||
             buttonName == "BackButton_HTP" || buttonName == "BackButton_Extra")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Back_button_dark.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Back_button_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Back_button.png";
        }
    }
    else if (buttonName == "Level1Button")
    {
        bool isSelected = (selectedLevelButtonName == "Level1Button");

        if (isSelected || isPressed || isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Level_select_01_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Level_select_01_dark.png";
        }
    }
    else if (buttonName == "Level2Button")
    {
        bool isSelected = (selectedLevelButtonName == "Level2Button");

        if (isSelected || isPressed || isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Level_select_02_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Level_select_02_dark.png";
        }
    }
    else if (buttonName == "Level3Button")
    {
        bool isSelected = (selectedLevelButtonName == "Level3Button");

        if (isSelected || isPressed || isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Level_select_03_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Level_select_03_dark.png";
        }
    }
    else if (buttonName == "LevelSelectedButton")
    {
        bool canStart = !selectedLevelButtonName.empty();

        if (!canStart)
        {
            texturePath = "../Assets/Sprites/Icons/win_lose_button_dark.png";
        }
        else if (isPressed || isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/win_lose_button.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/win_lose_button.png";
        }
    }

    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}

void MenuLogicSystem::ClearLevelSelection()
{
    selectedLevelButtonName.clear();
    selectedLevelScenePath.clear();
    hoveredButtonName.clear();
    pressedButtonName.clear();
    RefreshLevelSelectButtons();
}

void MenuLogicSystem::Reset()
{
    currScreen = MenuScreens::Home;

    // reset the bars
    masterSlider = SliderState();
    musicSlider = SliderState();
    sfxSlider = SliderState();
    isDraggingSlider = false;
    currentSliderCategory = "";
}

void MenuLogicSystem::ComputeSliderUVBounds(RenderComponent *renderComp, SliderState &sliderState)
{
    std::string texturePath = renderComp->GetTexturePath();

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char *pixels = stbi_load(texturePath.c_str(), &width, &height, &channels, 4);

    if (pixels && width > 0 && height > 0)
    {
        int minX = width;
        int maxX = -1;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = (y * width + x) * 4;
                unsigned char alpha = pixels[index + 3];

                if (alpha > 0)
                {
                    minX = std::min(minX, x);
                    maxX = std::max(maxX, x);
                }
            }
        }

        if (maxX >= minX)
        {
            sliderState.uvBarMin = static_cast<float>(minX) / static_cast<float>(width);
            sliderState.uvBarMax = static_cast<float>(maxX + 1) / static_cast<float>(width);
            sliderState.uvBarWidth = sliderState.uvBarMax - sliderState.uvBarMin;
        }
        else
        {
            sliderState.uvBarMin = 0.0f;
            sliderState.uvBarMax = 1.0f;
            sliderState.uvBarWidth = 1.0f;
        }

        stbi_image_free(pixels);
    }
    else
    {
        sliderState.uvBarMin = 0.0f;
        sliderState.uvBarMax = 1.0f;
        sliderState.uvBarWidth = 1.0f;
    }

    sliderState.uvBoundsReady = true;
}

void MenuLogicSystem::UpdateSlider(const std::string &barName, SliderState &sliderState, float volumePercent)
{
    auto &factory = GameObjectFactory::Instance();
    GameObject *sliderBarGO = factory.FindGameObject(barName);

    if (!sliderBarGO)
    {
        return;
    }

    auto *renderComp = sliderBarGO->GetComponent<RenderComponent>();
    auto *transformComp = sliderBarGO->GetComponent<TransformComponent>();

    if (!renderComp || !transformComp)
    {
        return;
    }

    // compute the visible UV range once from the actual slider texture
    if (!sliderState.uvBoundsReady)
    {
        ComputeSliderUVBounds(renderComp, sliderState);
    }

    if (sliderState.originalScaleX <= 0.0f)
    {
        transformComp->GetScale(sliderState.originalScaleX, sliderState.originalScaleY);
        transformComp->GetPosition(sliderState.originalCenterX, sliderState.originalCenterY);

        // world-space start of the ACTUAL visible slider bar
        sliderState.leftEdge = (sliderState.originalCenterX - sliderState.originalScaleX * 0.5f) + (sliderState.originalScaleX * sliderState.uvBarMin);

        // usable visible slider width only
        sliderState.rightEdge = sliderState.originalScaleX * sliderState.uvBarWidth;
    }

    float fillPercentage = std::clamp(volumePercent, 0.0f, 1.0f);
    float safeFill = std::max(fillPercentage, 0.001f);

    // world width of remaining visible slider
    float newScaleX = sliderState.rightEdge * safeFill;
    float newCenterX = sliderState.leftEdge + (newScaleX * 0.5f);

    transformComp->SetScale(newScaleX, sliderState.originalScaleY);
    transformComp->SetPosition(newCenterX, sliderState.originalCenterY);

    // crop inside the painted slider bar
    float croppedUWidth = sliderState.uvBarWidth * safeFill;
    renderComp->SetUVScale(vec2(croppedUWidth, 1.0f));
    renderComp->SetUVOffset(vec2(sliderState.uvBarMin, 0.0f));
}

void MenuLogicSystem::HandleSliderInput(double mouseX, double mouseY, bool isMouseDown)
{
    auto &factory = GameObjectFactory::Instance();
    AudioManager *audioManager = CORE->GetAudioManager();
    bool isMouseClicked = isMouseDown && !wasMouseDownOnSlider;

    if (isMouseClicked && !isDraggingSlider)
    {
        GameObject *masterBarBack = factory.FindGameObject("Slider_Bar_Back_Master");
        GameObject *musicBarBack = factory.FindGameObject("Slider_Bar_Back_Music");
        GameObject *sfxBarBack = factory.FindGameObject("Slider_Bar_Back_SFX");

        if (IsMouseOver(masterBarBack, mouseX, mouseY))
        {
            isDraggingSlider = true;
            currentSliderCategory = "master";
        }
        else if (IsMouseOver(musicBarBack, mouseX, mouseY))
        {
            isDraggingSlider = true;
            currentSliderCategory = "music";
        }
        else if (IsMouseOver(sfxBarBack, mouseX, mouseY))
        {
            isDraggingSlider = true;
            currentSliderCategory = "sfx";
        }
    }

    wasMouseDownOnSlider = isMouseDown;

    if (!isMouseDown)
    {
        isDraggingSlider = false;
        currentSliderCategory = "";
    }

    // update slider position while dragging
    if (isDraggingSlider && !currentSliderCategory.empty())
    {
        std::string barBackName;
        SliderState *sliderState = nullptr;
        std::string category;

        if (currentSliderCategory == "master")
        {
            barBackName = "Slider_Bar_Back_Master";
            sliderState = &masterSlider;
            category = "";
        }
        else if (currentSliderCategory == "music")
        {
            barBackName = "Slider_Bar_Back_Music";
            sliderState = &musicSlider;
            category = "music";
        }
        else if (currentSliderCategory == "sfx")
        {
            barBackName = "Slider_Bar_Back_SFX";
            sliderState = &sfxSlider;
            category = "sfx";
        }

        if (sliderState)
        {
            // get background bar to determine the slider range
            GameObject *barBack = factory.FindGameObject(barBackName);
            if (barBack)
            {
                auto *barTransform = barBack->GetComponent<TransformComponent>();
                if (barTransform)
                {
                    float barCenterX, barCenterY, barScaleX, barScaleY;
                    barTransform->GetPosition(barCenterX, barCenterY);
                    barTransform->GetScale(barScaleX, barScaleY);

                    float leftBound = (barCenterX - barScaleX * 0.5f) + (barScaleX * sliderState->uvBarMin);
                    float usableWidth = barScaleX * sliderState->uvBarWidth;
                    float rightBound = leftBound + usableWidth;
                    float mouseXf = static_cast<float>(mouseX);

                    mouseXf = std::clamp(mouseXf, leftBound, rightBound);

                    // calculate volume based on mouse position within the usable slider area
                    float volumePercent = (mouseXf - leftBound) / usableWidth;
                    volumePercent = std::clamp(volumePercent, 0.0f, 1.0f);

                    sliderState->currentVolume = volumePercent;

                    // update the visual slider
                    std::string barName;
                    if (currentSliderCategory == "master")
                        barName = "Slider_Bar_Master";
                    else if (currentSliderCategory == "music")
                        barName = "Slider_Bar_Music";
                    else if (currentSliderCategory == "sfx")
                        barName = "Slider_Bar_SFX";

                    UpdateSlider(barName, *sliderState, volumePercent);

                    // this for the scissors icon
                    std::string knobName;
                    if (currentSliderCategory == "master")
                        knobName = "Slider_Knob_Master";
                    else if (currentSliderCategory == "music")
                        knobName = "Slider_Knob_Music";
                    else if (currentSliderCategory == "sfx")
                        knobName = "Slider_Knob_SFX";

                    GameObject *sliderBar = factory.FindGameObject(barName);
                    GameObject *knob = factory.FindGameObject(knobName);
                    if (sliderBar && knob)
                    {
                        auto *sliderTransform = sliderBar->GetComponent<TransformComponent>();
                        auto *knobTransform = knob->GetComponent<TransformComponent>();
                        if (sliderTransform && knobTransform)
                        {
                            float sliderCenterX, sliderCenterY, sliderScaleX, sliderScaleY;
                            sliderTransform->GetPosition(sliderCenterX, sliderCenterY);
                            sliderTransform->GetScale(sliderScaleX, sliderScaleY);
                            float knobX = sliderCenterX + sliderScaleX * 0.5f - 285.0f;
                            knobTransform->SetPosition(knobX, sliderCenterY);
                        }
                    }

                    // update audio volume and save it
                    if (audioManager)
                    {
                        if (currentSliderCategory == "master")
                        {
                            savedMasterVolume = volumePercent;
                            audioManager->SetMasterVolume(savedMasterVolume);

                            // apply master as a multiplier to individual category volumes
                            float finalMusicVol = savedMusicVolume * savedMasterVolume;
                            float finalSfxVol = savedSfxVolume * savedMasterVolume;

                            audioManager->SetCategoryVolume("music", finalMusicVol);
                            audioManager->SetCategoryVolume("sfx", finalSfxVol);

                            std::cout << "[MenuLogic] Master changed to " << savedMasterVolume << " - Music: " << finalMusicVol
                                      << " (base: " << savedMusicVolume << "), SFX: " << finalSfxVol
                                      << " (base: " << savedSfxVolume << ")" << std::endl;
                        }
                        else
                        {
                            // individual category, save base volume and apply master multiplier
                            if (currentSliderCategory == "music")
                            {
                                savedMusicVolume = volumePercent;
                                audioManager->SetBaseCategoryVolume("music", volumePercent);
                            }
                            else if (currentSliderCategory == "sfx")
                            {
                                savedSfxVolume = volumePercent;
                                audioManager->SetBaseCategoryVolume("sfx", volumePercent);
                            }

                            // apply master multiplier to get final volume
                            float finalVolume = volumePercent * savedMasterVolume;
                            audioManager->SetCategoryVolume(category, finalVolume);

                            //std::cout << "[MenuLogic] " << category << " changed: base=" << volumePercent
                            //         << ", master=" << savedMasterVolume << ", final=" << finalVolume << std::endl;
                        }
                    }
                }
            }
        }
    }
}

void MenuLogicSystem::ScanReplays()
{
    m_replayNames.clear();
    m_replayLevels.clear();
    namespace fs = std::filesystem;

    const std::string replayDir = "../Assets/Replay/";
    if (!fs::exists(replayDir))
    {
        std::cout << "[MenuLogic] Replay directory not found: " << replayDir << std::endl;
        return;
    }

    std::vector<std::pair<fs::file_time_type, std::string>> files;
    for (const auto &entry : fs::directory_iterator(replayDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            files.push_back({entry.last_write_time(), entry.path().stem().string()});
    }

    // Sort newest-first
    std::sort(files.begin(), files.end(),
              [](const auto &a, const auto &b)
              { return a.first > b.first; });

    for (size_t i = 0; i < std::min(files.size(), size_t(3)); ++i)
    {
        const std::string &stem = files[i].second;
        m_replayNames.push_back(stem);

        // Read the "level" field from the JSON header
        std::string levelName;
        std::ifstream f(replayDir + stem + ".json");
        if (f)
        {
            try
            {
                nlohmann::json root;
                f >> root;
                levelName = root.value("level", std::string{});
            }
            catch (...)
            {
            }
        }
        m_replayLevels.push_back(levelName);
    }

    std::cout << "[MenuLogic] Found " << m_replayNames.size() << " replay(s)." << std::endl;
}

// ===== Leaderboard =====

void MenuLogicSystem::ScanLeaderboard(int level)
{
    m_leaderboardData.clear();
    m_selectedRank = -1;
    namespace fs = std::filesystem;

    const std::string dir = "../Assets/Replay/Level_" + std::to_string(level) + "/";
    if (!fs::exists(dir))
    {
        std::cout << "[MenuLogic] Leaderboard dir not found: " << dir << std::endl;
        return;
    }

    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
            continue;

        std::string stem = entry.path().stem().string();
        float bestTime = FLT_MAX;

        std::ifstream f(entry.path().string());
        if (f)
        {
            try
            {
                nlohmann::json root;
                f >> root;
                bestTime = root.value("bestTime", FLT_MAX);
            }
            catch (...)
            {
            }
        }

        if (bestTime < FLT_MAX)
            m_leaderboardData.push_back({stem, bestTime});
    }

    // Sort fastest first, keep top 3
    std::sort(m_leaderboardData.begin(), m_leaderboardData.end(),
              [](const LeaderboardEntry &a, const LeaderboardEntry &b)
              { return a.time < b.time; });
    if (m_leaderboardData.size() > 3)
        m_leaderboardData.resize(3);

    std::cout << "[MenuLogic] Leaderboard Level " << level
              << ": " << m_leaderboardData.size() << " entries." << std::endl;
}

void MenuLogicSystem::UpdateLeaderboardTextures()
{
    auto &factory = GameObjectFactory::Instance();

    const std::string yellow = "../Assets/Leaderboards_tab_yellow.png";
    const std::string grey = "../Assets/Leaderboards_tab_grey.png";

    auto setTex = [](GameObject *obj, const std::string &path)
    {
        if (!obj)
            return;
        if (auto *rc = obj->GetComponent<RenderComponent>())
        {
            rc->SetTexturePath(path);
            rc->SetTextureID(GraphicsSystem::LoadTexture(path.c_str()));
        }
    };

    // Level tabs
    const char *levelTabs[] = {"Leaderboards_Level_1", "Leaderboards_Level_2", "Leaderboards_Level_3"};
    for (int i = 0; i < 3; ++i)
        setTex(factory.FindGameObject(levelTabs[i]), (i + 1 == m_leaderboardLevel) ? yellow : grey);

    // Rank row tabs
    const char *rankTabs[] = {"Leaderboards_tab_1st", "Leaderboards_tab_2nd", "Leaderboards_tab_3rd"};
    for (int i = 0; i < 3; ++i)
        setTex(factory.FindGameObject(rankTabs[i]), (i == m_selectedRank) ? yellow : grey);
}

void MenuLogicSystem::PopulateLeaderboard()
{
    auto &factory = GameObjectFactory::Instance();

    const char *nameSlots[] = {"Name_1st_Text", "Name_2nd_Text", "Name_3rd_Text"};
    const char *timeSlots[] = {"Time_1st_Text", "Time_2nd_Text", "Time_3rd_Text"};

    for (int i = 0; i < 3; ++i)
    {
        auto *nameObj = factory.FindGameObject(nameSlots[i]);
        auto *timeObj = factory.FindGameObject(timeSlots[i]);

        if (i < static_cast<int>(m_leaderboardData.size()))
        {
            const auto &entry = m_leaderboardData[i];

            if (nameObj)
                if (auto *tc = nameObj->GetComponent<TextComponent>())
                    tc->SetText(entry.name);

            if (timeObj)
            {
                if (auto *tc = timeObj->GetComponent<TextComponent>())
                {
                    int mins = static_cast<int>(entry.time) / 60;
                    float secs = entry.time - mins * 60.0f;
                    char buf[32];
                    std::snprintf(buf, sizeof(buf), "%d:%05.2f", mins, secs);
                    tc->SetText(buf);
                }
            }
        }
        else
        {
            if (nameObj)
                if (auto *tc = nameObj->GetComponent<TextComponent>())
                    tc->SetText("---");
            if (timeObj)
                if (auto *tc = timeObj->GetComponent<TextComponent>())
                    tc->SetText("--:--.--");
        }
    }
}
