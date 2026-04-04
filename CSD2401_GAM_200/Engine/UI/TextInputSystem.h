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
 * @struct DialogStyle
 * @brief Visual configuration for the TextInputSystem modal dialog.
 *
 * Game code fills this in and passes it to TextInputSystem::SetStyle().
 * Texture paths are relative paths (e.g. "../Assets/Sprites/...").
 * Leave a texture path empty to fall back to a plain coloured quad.
 */
struct DialogStyle
{
    // --- Texture paths (empty = use plain colour quad) ---
    std::string titleTexPath;   ///< Header bar background texture.
    std::string fieldTexPath;   ///< Input field background texture.
    std::string hintTexPath;    ///< Hint bar background texture.

    // --- Panel colours (used as colour tint when a texture is set) ---
    vec4 overlayColor    = {0.0f,  0.0f,  0.05f, 0.75f};
    vec4 titlePanelColor = {0.1f,  0.1f,  0.25f, 0.95f};
    vec4 fieldPanelColor = {0.85f, 0.85f, 0.9f,  1.0f };
    vec4 hintPanelColor  = {0.1f,  0.1f,  0.25f, 0.85f};

    // --- Text colours ---
    vec4 titleTextColor  = {1.0f,  1.0f,  1.0f,  1.0f };
    vec4 fieldTextColor  = {0.05f, 0.05f, 0.1f,  1.0f };
    vec4 hintTextColor   = {0.8f,  0.8f,  0.8f,  1.0f };
};

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

    /**
     * @brief Set a world-space offset for the input field from the camera centre.
     *        Pass (0, 0) to restore the default centred position.
     */
    void SetFieldOffset(float dx, float dy) { m_fieldDX = dx; m_fieldDY = dy; }

    /**
     * @brief Override the input field size in world-space units.
     *        Pass (0, 0) to use the default size.
     */
    void SetFieldSize(float sx, float sy) { m_fieldSX = sx; m_fieldSY = sy; }

    /**
     * @brief When false the overlay, title bar and hint bar are not created,
     *        leaving only the input field.  Use this to embed the field inline
     *        inside an existing screen.
     */
    void SetShowAuxPanels(bool show) { m_showAuxPanels = show; }

    /**
     * @brief Programmatically confirm the dialog with the current typed text,
     *        invoking onConfirm and closing the dialog.  No-op if not active.
     */
    void Confirm();

    /**
     * @brief Override the visual style of the dialog (textures and colours).
     *        Call before Show(). Safe to call multiple times.
     */
    void SetStyle(const DialogStyle& style) { m_style = style; }

    /**
     * @brief Set a callback invoked on every accepted keystroke (character or backspace).
     *        Use this to play typing sound effects. Cleared automatically on Hide().
     */
    static void SetOnCharTyped(std::function<void()> cb) { s_onCharTyped = std::move(cb); }

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

    /** @return True if the dialog consumed an ESC press this frame (even if it just closed). */
    bool ConsumedEscapeThisFrame() const { return m_consumedEscapeThisFrame; }

    /** @return The text currently typed by the player (valid while IsActive()). */
    std::string GetCurrentText() const { return s_buffer; }

private:
    // ===== Runtime state =====
    bool m_isActive  = false;
    int  m_maxLength = 20;
    bool m_hasUI     = false;

    // ===== Inline-field overrides =====
    float m_fieldDX      = 0.f;   ///< World-space X offset from camera centre.
    float m_fieldDY      = 0.f;   ///< World-space Y offset from camera centre.
    float m_fieldSX      = 0.f;   ///< World-space field width  (0 = use default).
    float m_fieldSY      = 0.f;   ///< World-space field height (0 = use default).
    bool  m_showAuxPanels = true;  ///< Whether to create overlay/title/hint panels.

    std::function<void(const std::string&)> m_onConfirm;
    std::function<void()>                   m_onCancel;

    bool m_backspaceWasHeld        = false;
    bool m_enterWasHeld            = false;
    bool m_escapeWasHeld           = false;
    bool m_consumedEscapeThisFrame = false;

    // ===== Appearance =====
    // NOTE: No engine-side defaults for font. Game code MUST call SetFont() before Show().
    std::string m_fontPath;
    int         m_fontSize = 0;   ///< 0 = not yet configured by game code.
    float       m_uiScale  = 2.0f;
    DialogStyle m_style;          ///< Visual style (textures + colours). Defaults to plain quads.

    // ===== Static GLFW char-callback state =====
    static std::string           s_buffer;       ///< Accumulates typed characters.
    static bool                  s_active;       ///< Guards the callback against stale events.
    static int                   s_maxLength;    ///< Forwarded from Show().
    static GLFWcharfun           s_prevCallback; ///< Saved callback to restore on Hide().
    static std::function<void()> s_onCharTyped;  ///< Optional sound hook set by game code.

    /** GLFW char callback — appends printable ASCII while the dialog is open. */
    static void CharCallback(GLFWwindow* window, unsigned int codepoint);

    // ===== Helpers =====
    void CreateUI(const std::string& title);
    void DestroyUI();
    void UpdateUI(); ///< Repositions all dialog elements to follow the camera.
};
