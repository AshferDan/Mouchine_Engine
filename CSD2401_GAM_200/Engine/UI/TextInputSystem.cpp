/* Start Header ************************************************************************/
/*!
\file       TextInputSystem.cpp
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "TextInputSystem.h"
#include "CameraManager.h"
#include "factory.h"
#include "transform.h"
#include "GraphicsSystem.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "core.h"
#include <algorithm>
#include <iostream>

// ===== Static member definitions =====
std::string  TextInputSystem::s_buffer;
bool         TextInputSystem::s_active     = false;
int          TextInputSystem::s_maxLength  = 20;
GLFWcharfun  TextInputSystem::s_prevCallback = nullptr;

// ===== GLFW char callback =====

void TextInputSystem::CharCallback(GLFWwindow*, unsigned int codepoint)
{
    if (!s_active) return;
    if (static_cast<int>(s_buffer.size()) >= s_maxLength) return;
    if (codepoint >= 32 && codepoint < 127)   // printable ASCII
        s_buffer += static_cast<char>(codepoint);
}

// ===== ISystem =====

void TextInputSystem::Initialize()
{
    m_isActive          = false;
    m_hasUI             = false;
    m_backspaceWasHeld  = false;
    m_enterWasHeld      = false;
    m_escapeWasHeld     = false;
    s_buffer.clear();
    s_active = false;
    std::cout << "Initializing system: TextInputSystem" << std::endl;
}

void TextInputSystem::Update(float dt)
{
    (void)dt;
    if (!m_isActive) return;

    GLFWwindow* win = glfwGetCurrentContext();
    if (!win) return;

    // --- Backspace: edge-detected removal of last character ---
    bool backNow = (glfwGetKey(win, GLFW_KEY_BACKSPACE) == GLFW_PRESS);
    if (backNow && !m_backspaceWasHeld && !s_buffer.empty())
        s_buffer.pop_back();
    m_backspaceWasHeld = backNow;

    // --- Enter: confirm ---
    bool enterNow = (glfwGetKey(win, GLFW_KEY_ENTER)    == GLFW_PRESS) ||
                    (glfwGetKey(win, GLFW_KEY_KP_ENTER) == GLFW_PRESS);
    if (enterNow && !m_enterWasHeld)
    {
        auto  cb     = m_onConfirm;
        const std::string result = s_buffer;
        Hide();
        if (cb) cb(result);
        return;
    }
    m_enterWasHeld = enterNow;

    // --- Escape: cancel ---
    bool escNow = (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS);
    if (escNow && !m_escapeWasHeld)
    {
        auto cb = m_onCancel;
        Hide();
        if (cb) cb();
        return;
    }
    m_escapeWasHeld = escNow;

    // --- Reposition UI and refresh the typed text each frame ---
    UpdateUI();
}

// ===== Configuration =====

void TextInputSystem::SetFont(const std::string& fontPath, int fontSize)
{
    m_fontPath = fontPath;
    m_fontSize = fontSize;
}

// ===== Dialog control =====

void TextInputSystem::Show(const std::string& title,
                           int maxLength,
                           std::function<void(const std::string&)> onConfirm,
                           std::function<void()>                   onCancel,
                           const std::string&                      defaultText)
{
    if (m_isActive) return;   // already open

    // Font must be supplied by game code via SetFont() — the engine has no default.
    if (m_fontPath.empty() || m_fontSize <= 0)
    {
        std::cerr << "[TextInputSystem] ERROR: Show() called before SetFont(). "
                     "Call CORE->GetTextInputSystem()->SetFont(path, size) "
                     "from game code before opening the dialog.\n";
        return;
    }

    m_maxLength        = std::max(1, std::min(maxLength, 64));
    m_onConfirm        = onConfirm;
    m_onCancel         = onCancel;
    m_isActive         = true;
    m_backspaceWasHeld = false;
    m_enterWasHeld     = false;
    m_escapeWasHeld    = false;

    // Activate the char callback
    s_buffer    = defaultText;
    s_active    = true;
    s_maxLength = m_maxLength;

    GLFWwindow* win = glfwGetCurrentContext();
    if (win)
        s_prevCallback = glfwSetCharCallback(win, CharCallback);

    CreateUI(title);
    std::cout << "[TextInputSystem] Dialog opened.\n";
}

void TextInputSystem::Hide()
{
    if (!m_isActive) return;

    // Restore previous GLFW char callback
    s_active = false;
    GLFWwindow* win = glfwGetCurrentContext();
    if (win)
        glfwSetCharCallback(win, s_prevCallback);
    s_prevCallback = nullptr;

    DestroyUI();

    m_isActive  = false;
    m_onConfirm = nullptr;
    m_onCancel  = nullptr;
    std::cout << "[TextInputSystem] Dialog closed.\n";
}

// ===== Helpers =====

void TextInputSystem::CreateUI(const std::string& title)
{
    if (m_hasUI) return;

    auto&       factory   = GameObjectFactory::Instance();
    const vec2  camPos    = CameraManager::Instance().GetCameraPosition();
    const float s         = m_uiScale;
    const int   baseLayer = 200;   // draws on top of all game objects

    // Helper: build a panel RenderComponent with optional texture
    auto applyPanel = [&](RenderComponent* r,
                          const std::string& texPath,
                          vec4 color,
                          int layer)
    {
        if (!texPath.empty())
            r->SetTextureID(GraphicsSystem::LoadTexture(texPath.c_str()));
        r->SetColor(color);
        r->SetLayer(layer);
    };

    // --- Full-screen semi-transparent dark overlay ---
    GameObject* overlay = factory.CreateGameObject("TextInput_Overlay");
    auto* ovT = overlay->AddComponent<TransformComponent>();
    ovT->posX   = camPos.x;
    ovT->posY   = camPos.y;
    ovT->scaleX = 1920.0f * s;
    ovT->scaleY = 1080.0f * s;
    auto* ovR = overlay->AddComponent<RenderComponent>();
    applyPanel(ovR, {}, m_style.overlayColor, baseLayer);

    // --- Title bar ---
    GameObject* titleObj = factory.CreateGameObject("TextInput_Title");
    auto* titleT = titleObj->AddComponent<TransformComponent>();
    titleT->posX   = camPos.x;
    titleT->posY   = camPos.y + 200.0f * s;
    titleT->scaleX = 800.0f * s;
    titleT->scaleY = 180.0f * s;
    auto* titleR = titleObj->AddComponent<RenderComponent>();
    applyPanel(titleR, m_style.titleTexPath, m_style.titlePanelColor, baseLayer + 1);
    titleObj->AddComponent<TextComponent>(
        m_fontPath.c_str(),
        static_cast<int>(m_fontSize * s),
        title,
        m_style.titleTextColor,
        false);

    // --- Input field ---
    GameObject* fieldObj = factory.CreateGameObject("TextInput_Field");
    auto* fieldT = fieldObj->AddComponent<TransformComponent>();
    fieldT->posX   = camPos.x;
    fieldT->posY   = camPos.y;
    fieldT->scaleX = 800.0f * s;
    fieldT->scaleY = 130.0f * s;
    auto* fieldR = fieldObj->AddComponent<RenderComponent>();
    applyPanel(fieldR, m_style.fieldTexPath, m_style.fieldPanelColor, baseLayer + 1);
    fieldObj->AddComponent<TextComponent>(
        m_fontPath.c_str(),
        static_cast<int>(m_fontSize * s),
        s_buffer + "_",
        m_style.fieldTextColor,
        false);

    // --- Hint bar ---
    std::string hintText = "ENTER = confirm";
    if (m_onCancel)
        hintText += "     ESC = cancel";

    GameObject* hintObj = factory.CreateGameObject("TextInput_Hint");
    auto* hintT = hintObj->AddComponent<TransformComponent>();
    hintT->posX   = camPos.x;
    hintT->posY   = camPos.y - 200.0f * s;
    hintT->scaleX = 800.0f * s;
    hintT->scaleY = 100.0f * s;
    auto* hintR = hintObj->AddComponent<RenderComponent>();
    applyPanel(hintR, m_style.hintTexPath, m_style.hintPanelColor, baseLayer + 1);
    hintObj->AddComponent<TextComponent>(
        m_fontPath.c_str(),
        static_cast<int>(m_fontSize * 0.55f * s),
        hintText,
        m_style.hintTextColor,
        false);

    m_hasUI = true;
}

void TextInputSystem::DestroyUI()
{
    if (!m_hasUI) return;

    auto& factory = GameObjectFactory::Instance();
    for (const std::string& name : {"TextInput_Overlay",
                                    "TextInput_Title",
                                    "TextInput_Field",
                                    "TextInput_Hint"})
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj) factory.DestroyGameObject(obj->GetID());
    }

    m_hasUI = false;
}

void TextInputSystem::UpdateUI()
{
    if (!m_hasUI) return;

    const float s      = m_uiScale;
    const vec2  camPos = CameraManager::Instance().GetCameraPosition();

    struct Elem { const char* name; float dy; };
    constexpr Elem elems[] = {
        {"TextInput_Overlay",  0.0f},
        {"TextInput_Title",    200.0f},
        {"TextInput_Field",      0.0f},
        {"TextInput_Hint",    -200.0f},
    };

    auto& factory = GameObjectFactory::Instance();
    for (const auto& e : elems)
    {
        GameObject* obj = factory.FindGameObject(e.name);
        if (!obj) continue;
        auto* t = obj->GetComponent<TransformComponent>();
        if (t) { t->posX = camPos.x; t->posY = camPos.y + e.dy * s; }
    }

    // Refresh the typed text in the input field
    GameObject* field = factory.FindGameObject("TextInput_Field");
    if (field)
    {
        auto* tc = field->GetComponent<TextComponent>();
        if (tc) tc->SetText(s_buffer + "_");
    }
}
