/* Start Header ************************************************************************/
/*!
\file       TextComponent.h
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Component.h"
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "gameObject.h"
#include "MathLib.h"
#include "ResourceManager.h"

/**
 * @class TextComponent
 * @brief Manages the data required to render text on a GameObject.
 * * Holds the text content, color, scale, and a pointer to the shared,
 * globally managed FontResource.
 */
class TextComponent : public IComponent {
public:
	/**
	 * @brief Constructor for the TextComponent.
	 * Loads the font resource if it hasn't been loaded already.
	 * @param owner The GameObject this component belongs to.
	 * @param fontPath The file path to the font (e.g., "fonts/arial.ttf").
	 * @param fontSize The size of the font glyphs to be generated.
	 * @param text The initial string content.
	 * @param color The initial color tint (RGBA).
	 * @param screenSpace If true, text is rendered in screen coordinates; otherwise, in world coordinates.
	 */
	TextComponent(GameObject* owner, const char* fontPath, GLuint fontSize, const std::string& text, vec4 color = { 1, 1, 1, 1 }, bool screenSpace = true);

	/**
	 * @brief Virtual destructor.
	 */
	virtual ~TextComponent();

	/// --- Getters and Setters ---

	/**
	 * @brief Sets the text content string.
	 * @param text The new string to display.
	 */
	void SetText(const std::string& text);

	/**
	 * @brief Retrieves the current text content string.
	 * @return The displayed text.
	 */
	std::string GetText() const { return text; }

	/**
	 * @brief Sets the text color tint.
	 * @param newColor The new color (RGBA).
	 */
	void SetColor(const vec4& newColor) { color = newColor; }

	/**
	 * @brief Retrieves the current text color tint.
	 * @return The color vector (RGBA).
	 */
	vec4 GetColor() const { return color; }

	/**
	 * @brief Sets the scaling factor for the text size.
	 * @param newScale The new scale factor (1.0 is original size).
	 */
	void SetScale(float newScale) { scale = newScale; }

	/**
	 * @brief Retrieves the current scaling factor.
	 * @return The scale factor.
	 */
	float GetScale() const { return scale; }

	/**
	 * @brief Retrieves the map of character metrics and texture IDs from the shared FontResource.
	 * This is used by the GraphicsSystem during rendering.
	 * @return A constant reference to the map of glyph data.
	 */
	const std::map<char, Character>& GetCharacters() const;

	/**
	 * @brief Sets whether the text is rendered in screen space (fixed GUI) or world space (follows camera).
	 * @param isScreenSpace True for screen space, false for world space.
	 */
	void SetScreenSpace(bool isScreenSpace) { screenSpace = isScreenSpace; }

	/**
	 * @brief Checks if the text is set to render in screen space.
	 * @return True if screen space is active.
	 */
	bool isScreenSpace() const { return screenSpace; }

	/**
	 * @brief Retrieves the file path used to load the font.
	 * @return The font file path string.
	 */
	std::string GetFontPath() { return fontPath; }

	/**
	 * @brief Retrieves the font size used for glyph generation.
	 * @return The font size (in pixels).
	 */
	GLuint GetFontSize() { return fontSize; }

	/**
	 * @brief Reloads the font with a new path and/or size.
	 *        Only accepts .ttf files. Silently ignores non-.ttf paths.
	 * @param newFontPath File path to the new .ttf font.
	 * @param newFontSize Desired font size in pixels.
	 */
	void SetFont(const char* newFontPath, GLuint newFontSize)
	{
		std::string path = newFontPath;
		if (path.size() < 4 || path.substr(path.size() - 4) != ".ttf")
		{
			std::cerr << "ERROR::TextComponent::SetFont: Only .ttf files are accepted.\n";
			return;
		}
		fontPath = path;
		fontSize = newFontSize;
		LoadFont(newFontPath, newFontSize);
	}

	/// --- Component Interface ---

	/**
	 * @brief Returns the name of the component type.
	 * @return "TextComponent".
	 */
	std::string GetName() const override { return "TextComponent"; }

	/**
	 * @brief Creates a deep copy of the component for use by a new GameObject.
	 * @param newOwner The GameObject that will own the cloned component.
	 * @return A unique pointer to the cloned component.
	 */
	virtual std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override;

	/**
	 * @brief Update function (unused for static text data).
	 */
	virtual void Update(float) override {}

private:
	// --- User-Defined Data ---
	std::string text;
	vec4 color;
	float scale;
	bool screenSpace;

	// --- Resource Identification ---
	std::string fontPath;
	GLuint fontSize;
	std::string fontResourceName; // Unique identifier (path + size) for the font in the resource manager.

	// --- Resource Pointer ---
	FontResource* fontResource; // Pointer to the shared font resource holding the glyph atlas data.

	/**
	 * @brief Internal function to load the shared FontResource via the ResourceManager.
	 * @param fontPath The file path of the font.
	 * @param fontSize The desired font size.
	 */
	void LoadFont(const char* fontPath, GLuint fontSize);
};