#pragma once
#include "Component.h"
#include <GL/glew.h>


class RenderComponent : public IComponent {
public:
    enum class ShapeType {
        Square,
        Circle
    };

    //Constructor for textured object
    RenderComponent(GameObject* owner, GLuint texID) : IComponent(owner), shape(ShapeType::Square), textureID(texID), color(1.0f) {}

    //Constructor for shape
    RenderComponent(GameObject* owner, ShapeType type, const glm::vec4& color) : IComponent(owner), shape(type), textureID(0), color(color) {};

    // Lifecycle overrides
    void Initialize() override {}
    void Update(float dt) override {}
    void Destroy() override {}

    // Required by IComponent
    std::string GetName() const override { return "RenderComponent"; }

    // Getter for texture
    GLuint GetTextureID() const { return textureID; }
    ShapeType GetShapeType() const { return shape; }
    glm::vec4 GetColor() const { return color; }

private:
    ShapeType shape;
    GLuint textureID;
    glm::vec4 color;
};
