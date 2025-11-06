#include "TextComponent.h"
#include "GraphicsSystem.h"
#include "ResourceManager.h"
#include <iostream>

static FT_Library ft = nullptr;
static int ftRetCount = 0;

TextComponent::TextComponent(GameObject* owner, const char* fontPath, GLuint fontSize, const std::string& text, vec4 color, bool screenSpace)
	:IComponent(owner), text(text), color(color), scale(1.f), fontPath(fontPath), fontSize(fontSize), screenSpace(screenSpace), fontResource(nullptr) {
	
	LoadFont(fontPath, fontSize);
}

TextComponent::~TextComponent() {
	fontResource = nullptr;
}

void TextComponent::LoadFont(const char* fontPath, GLuint fontSize) {
	// create unique resource name based on font path and size
	fontResourceName = std::string(fontPath) + "_" + std::to_string(fontSize);

	// load or get existing font from resource manager
	fontResource = ResourceManager::Instance().LoadFont(fontResourceName, fontPath, fontSize);

	if (!fontResource) {
		std::cerr << "ERROR::TextComponent: Failed to load font: " << fontPath << std::endl;
	}
	else {
		std::cout << "TextComponent using font: " << fontPath << " (size: " << fontSize << ")" << std::endl;
	}
}

const std::map<char, Character>& TextComponent::GetCharacters() const {
	if (fontResource) {
		return fontResource->GetCharacters();
	}

	// return empty map if font not loaded
	static std::map<char, Character> emptyMap;
	return emptyMap;
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
		color,
		screenSpace
	);
	cloned->SetScale(scale);
	cloned->SetScreenSpace(screenSpace);
	cloned->SetEnabled(IsEnabled());
	return cloned;
}

