#pragma once

#include "System.h"
#include <string>
#include <unordered_map>
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/mono-config.h>

class GameObject;

struct ScriptClass {
    MonoClass* monoClass = nullptr;
    MonoMethod* constructor = nullptr;
    MonoMethod* onStart = nullptr;
    MonoMethod* onUpdate = nullptr;
    MonoMethod* onDestroy = nullptr;
};

struct ScriptInstance {
    MonoObject* instance = nullptr;
    ScriptClass* scriptClass = nullptr;
    uint32_t gcHandle = 0;
};

class ScriptSystem : public ISystem {
private:
    MonoDomain* rootDomain = nullptr;
    MonoDomain* appDomain = nullptr;
    MonoAssembly* coreAssembly = nullptr;
    MonoImage* coreImage = nullptr;

    // Cache for loaded script classes
    std::unordered_map<std::string, ScriptClass> scriptClasses;

    // Map GameObject to their script instances
    std::unordered_map<GameObject*, std::vector<ScriptInstance>> gameObjectScripts;

    // Helper methods
    MonoAssembly* LoadAssembly(const std::string& filepath);
    MonoClass* GetClassInAssembly(MonoAssembly* assembly, const char* namespaceName, const char* className);
    MonoObject* InstantiateClass(MonoClass* monoClass);
    void RegisterInternalCalls();

public:
    ~ScriptSystem() override;

    void Initialize() override;
    void Shutdown();

    std::string GetName() override {
        return "ScriptingSystem";
    }

    void Update(float deltaTime) override;

    // Load C# assembly
    bool LoadScriptAssembly(const std::string& filepath);

    // Attach script to GameObject
    bool AttachScript(GameObject* gameObject, const std::string& scriptClassName);

    // Remove script from GameObject
    void RemoveScript(GameObject* gameObject, const std::string& scriptClassName);

    // Call script methods
    void CallOnStart(GameObject* gameObject);
    void CallOnUpdate(GameObject* gameObject, float deltaTime);
    void CallOnDestroy(GameObject* gameObject);

    // Reload scripts (hot reload)
    void ReloadScriptAssembly();
};