#pragma once
#include "Component.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include "gameObject.h"
#include "../Engine/Extern/MathLib.h"



struct character {
	GLuint TextureID;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint advance;
};

class TextComponent : public IComponent {
public:
	//TextComponent(GameObject* owner, const char* fontPath, GLuint fontSize, const std::string& text, vec3 color = { 1,1,1 });
	virtual ~TextComponent() {};

	void SetText(const std::string& text) {}


private:

};