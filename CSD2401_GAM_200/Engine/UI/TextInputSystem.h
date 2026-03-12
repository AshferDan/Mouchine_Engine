/* Start Header ************************************************************************/
/*!
\file       TextInputSystem.h
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "MathLib.h"
#include <string>
#include <functional>
#include <GLFW/glfw3.h>

/**
 * @class TextInputSystem
 * @brief Engine-level modal text-input dialog.
 *
 * Provides a reusable, camera-following text entry prompt that any game
 * system can invoke via a single Show() call.  The system creates its own
 * GameObjects for the dialog UI and destroys them automatically when the
 * dialog is dismissed.
 *
 * Typical usage
 * =============
 * @code
 * // One-time setup (e.g. in a game system's Initialize / firstUpdate):
 * CORE->GetTextInputSystem()->SetFont("../Assets/MyFont.ttf", 50);
 *
 * // Show the dialog at the appropriate moment:
 * CORE->GetTextInputSystem()->Show(
 *     "Name Your Run!", 20,
 *     [](const std::string& name) { ghost->SaveCurrentRun(name); },
 *     []()                        { /* cancelled – nothing to do *\/ });
 * @endcode
 */
class TextInputSystem : public ISystem
{
public:
    TextInputSystem()  = default;
    ~TextInputSystem() = default;

    // ===== ISystem =====
    void Initialize() override;
    void Update(float dt) override;
    std::string GetName() override { return "TextInputSystem"; }

    // ===== Configuration =====

    /**
     * @brief Set the font used for all dialog text labels.
     *        Must be called before the first Show().
     * @param fontPath  Relative path to a TrueType font (e.g. "../Assets/MyFont.ttf").
     * @param fontSize  Glyph size in points.
     */
    void SetFont(const std::string& fontPath, int fontSize);

    /**
     * @brief Override the world-space scale multiplier used when sizing dialog
     *        elements (default 2.0, matching the project's existing UI scale).
     */
    void SetUIScale(float scale) { m_uiScale = scale; }

    // ===== Dialog control =====

    /**
     * @brief Display the modal text-input dialog.
     *
     * The dialog follows the game camera each frame.  Keyboard input is
     * captured via a GLFW character callback; Backspace and Enter are
     * polled directly.  The previous char-callback (e.g. ImGui's) is
     * saved and restored when the dialog closes.
     *
     * @param title       Text shown above the input field (e.g. "Name Your Run!").
     * @param maxLength   Maximum number of characters the player may type (clamped 1-64).
     * @param onConfirm   Called with the typed string when Enter is pressed.
     *                    An empty string is passed if the player typed nothing.
     * @param onCancel    Called (with no arguments) when Escape is pressed.
     *                    Pass nullptr if cancelling should do nothing.
     * @param defaultText Pre-filled text shown in the field on open (optional).
     */
    void Show(const std::string& title,
              int maxLength,
              std::function<void(const std::string&)> onConfirm,
              std::function<void()>                   onCancel    = nullptr,
              const std::string&                      defaultText = "");

    /**
     * @brief Dismiss the dialog without invoking any callback.
     *        Safe to call when the dialog is already hidden.
     */
    void Hide();

    // ===== Queries =====

    /** @return True while the dialog is visible and capturing input. */
    bool IsActive() const { return m_isActive; }

    /** @return The text currently typed by the player (valid while IsActive()). */
    std::string GetCurrentText() const { return s_buffer; }

private:
    // ===== Runtime state =====
    bool m_isActive  = false;
    int  m_maxLength = 20;
    bool m_hasUI     = false;

    std::function<void(const std::string&)> m_onConfirm;
    std::function<void()>                   m_onCancel;

    bool m_backspaceWasHeld = false;
    bool m_enterWasHeld     = false;
    bool m_escapeWasHeld    = false;

    // ===== Appearance =====
    std::string m_fontPath;
    int         m_fontSize = 50;
    bool        m_hasFont  = false;
    float       m_uiScale  = 2.0f;

    // ===== Static GLFW char-callback state =====
    static std::string  s_buffer;      ///< Accumulates typed characters.
    static bool         s_active;      ///< Guards the callback against stale events.
    static int          s_maxLength;   ///< Forwarded from Show().
    static GLFWcharfun  s_prevCallback; ///< Saved callback to restore on Hide().

    /** GLFW char callback — appends printable ASCII while the dialog is open. */
    static void CharCallback(GLFWwindow* window, unsigned int codepoint);

    // ===== Helpers =====
    void CreateUI(const std::string& title);
    void DestroyUI();
    void UpdateUI(); ///< Repositions all dialog elements to follow the camera.
};
