#pragma once
#include "Component.h"
#include <GL/glew.h>

class RenderComponent : public IComponent {
public:
    RenderComponent(GameObject* owner, GLuint texID)
        : IComponent(owner), textureID(texID) {}

    // Lifecycle overrides
    void Initialize() override {}
    void Update(float dt) override {}
    void Destroy() override {}

    // Required by IComponent
    std::string GetName() const override { return "RenderComponent"; }

    // Getter for texture
    GLuint GetTextureID() const { return textureID; }

private:
    GLuint textureID;
};
