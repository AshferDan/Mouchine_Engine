#include "TextComponent.h"
#include "GraphicsSystem.h"
#include <iostream>

static FT_Library ft = nullptr;
static int ftRetCount = 0;

TextComponent::TextComponent(GameObject* owner, const char* fontPath, GLuint fontSize, const std::string& text, vec4 color, bool screenSpace)
	:IComponent(owner), text(text), color(color), scale(1.f), fontPath(fontPath), fontSize(fontSize), screenSpace(screenSpace) {
	if (ftRetCount == 0) {
		if (FT_Init_FreeType(&ft)) {
			std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			return;
		}
	}
	ftRetCount++;

	LoadFont(fontPath, fontSize);
}

TextComponent::~TextComponent() {
	for (auto& pair : characters) {
		glDeleteTextures(1, &pair.second.TextureID);
	}
	characters.clear();

	ftRetCount--;
	if (ftRetCount == 0 && ft) {
		FT_Done_FreeType(ft);
		ft = nullptr;
	}
}

void TextComponent::LoadFont(const char* fontPath, GLuint fontSize) {
	FT_Face face;
	if (FT_New_Face(ft, fontPath, 0, &face)) {
		std::cerr << "ERROR::FREETYPE: Failed to load font: " << fontPath << std::endl;
		return;
	}

	//std::cout << "SUCCESS: Loaded font: " << fontPath << std::endl;

	FT_Set_Pixel_Sizes(face, 0, fontSize);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//int loadedCount = 0;
	for (unsigned char c = 0; c < 128;c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cerr << "ERROR::FREETYPE: Failed to load Glyph for character: " << c << std::endl;
			continue;
		}

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<GLuint>(face->glyph->advance.x)
		};

		characters.insert(std::pair<char, Character>(c, character));
		//loadedCount++;
	}

	//std::cout << "Loaded " << loadedCount << " characters" << std::endl;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	FT_Done_Face(face);
}

void TextComponent::SetText(const std::string& newText) {
	text = newText;
}

std::unique_ptr<IComponent> TextComponent::Clone(GameObject* newOwner) const {
	auto cloned = std::make_unique<TextComponent>(
		newOwner,
		fontPath.c_str(),
		fontSize,
		text,
		color
	);
	cloned->SetScale(scale);
	cloned->SetScreenSpace(screenSpace);
	cloned->SetEnabled(IsEnabled());
	return cloned;
}

