/* Start Header ************************************************************************/
/*!
\file       main.cpp
\author     Lee Hwee Min [100% - 95 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "main.h"

namespace CrashHandler
{
    std::string GetStackTrace();
    void LogCrash(const char *type, const std::exception *e = nullptr);
}

/**
 * @brief Enables memory leak checking in debug mode.
 * @param breakAlloc, Allocation number to break on for debugging.
 */
void EnableMemoryLeakChecking(int breakAlloc = -1)
{
    // Set the leak checking flag
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);

    // If a valid break alloc provided set the breakAlloc
    if (breakAlloc != -1)
        _CrtSetBreakAlloc(breakAlloc);
}

/**
 * @brief Main entry point of the program.
 *
 * Initializes the engine, resource manager, audio, and game objects.
 * Loads textures, audio files, sets up the scene, then enters the main loop.
 *
 * @return Program exit code (0 = success).
 */
int main()
{
    try
    {

        EnableMemoryLeakChecking();
        CoreEngine engine;
        AudioEngine audio;

        API::Init_Engine(engine, audio);
        
        //Serialization::LoadScene("Prefab_TestScene.json");
        Serialization::LoadScene("../Assets/Scenes/Prefab_TestScene.json");
        /*LevelEditor::LoadLevel("../Assets/Prefabs/Prefab_TestScene.json");*/

        GameObject* textObj = GameObjectFactory::Instance().CreateGameObject("ScoreText");
        textObj->AddComponent<TransformComponent>(100.0f, 100.0f);  // Position on screen
        auto* text = textObj->AddComponent<TextComponent>(
            "./Assets/FrederickatheGreat-Regular.ttf",
            48,  // Font size
            "Score: 0",
            vec4(1.0f, 1.0f, 1.0f, 1.0f),
            true
        );
        text->SetScale(1.0f);

        // Later, update the text:
        text->SetText("Score: 100");

        GameObject* mainCamera = GameObjectFactory::Instance().CreateGameObject("MainCamera");
        auto* cam = mainCamera->AddComponent<CameraComponent>();

        GameObject* player = GameObjectFactory::Instance().FindGameObject("Hero");
        if (player) {
            cam->SetMode(CameraComponent::CameraMode::FOLLOW_TARGET);
            //WHEN CAMERA MODE IS FIXED
            //cam->SetPosition(0.f, 0.f);
            cam->SetFollowTarget(player);
            cam->SetFollowOffset(vec2(0.f, 0.f));
            cam->SetSmoothing(true, 2.f);
        }
        CameraManager::Instance().SetActiveCamera(mainCamera);

    
        API::StartEngineLoop(engine);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[TEST] Exception caught: " << e.what() << "\n";
        CrashHandler::LogCrash("EXCEPTION", &e);
    }
    catch (...)
    {
        CrashHandler::LogCrash("UNKNOWN EXCEPTION");
    }
    return 0;

}



namespace CrashHandler
{

#if defined(_WIN32)
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif

    std::string GetStackTrace()
    {
#if defined(_WIN32)
        void *stack[62];
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);
        WORD frames = CaptureStackBackTrace(0, 62, stack, NULL);

        std::ostringstream oss;
        for (WORD i = 0; i < frames; ++i)
        {
            DWORD64 address = (DWORD64)(stack[i]);
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;

            DWORD64 displacement = 0;
            if (SymFromAddr(process, address, &displacement, symbol))
            {
                oss << "[" << i << "] " << symbol->Name
                    << " - 0x" << std::hex << symbol->Address << std::dec << "\n";
            }
        }
        return oss.str();
#else
        return "Stacktrace not supported on this platform.";
#endif
    }

    void LogCrash(const char *type, const std::exception *e)
    {
        std::filesystem::create_directories("../Logs");
        std::ofstream log("../Logs/CrashLog.txt", std::ios::app);
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        log << "\n[" << type << "] " << std::put_time(std::localtime(&now), "%F %T") << "\n";
        if (e)
            log << e->what() << "\n";
        log << GetStackTrace() << "\n";
        log.close();
    }
} // namespace CrashHandler
