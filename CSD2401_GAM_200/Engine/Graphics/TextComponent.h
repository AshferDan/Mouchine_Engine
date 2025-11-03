#pragma once
#include "Component.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "gameObject.h"
#include "MathLib.h"



struct Character {
	GLuint TextureID;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint advance;
};

class TextComponent : public IComponent {
public:
	TextComponent(GameObject* owner, const char* fontPath, GLuint fontSize, const std::string& text, vec4 color = { 1, 1, 1, 1 }, bool screenSpace = true);
	virtual ~TextComponent();

	void SetText(const std::string& text);
	std::string GetText() const { return text; }

	void SetColor(const vec4& newColor) { color = newColor; }
	vec4 GetColor() const { return color; }

	void SetScale(float newScale) { scale = newScale; }
	float GetScale() const { return scale; }

	const std::map<char, Character>& GetCharacters() const { return characters; }

	void SetScreenSpace(bool isScreenSpace) { screenSpace = isScreenSpace; }
	bool isScreenSpace() const { return screenSpace; }

	std::string GetName() const override { return "TextComponent";}
	virtual std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override;
	virtual void Update(float) override {}

private:
	std::string text;
	vec4 color;
	float scale;
	bool screenSpace;
	std::string fontPath;
	GLuint fontSize;

	std::map<char, Character> characters;

	void LoadFont(const char* fontPath, GLuint fontSize);
};