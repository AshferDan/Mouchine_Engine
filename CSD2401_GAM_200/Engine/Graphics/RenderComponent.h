#pragma once
#include "Component.h"
#include <GL/glew.h>


class RenderComponent : public IComponent {
public:
    enum class ShapeType {
        Square,
        Circle,
        Line,
        Point
    };

    struct LineData {
        glm::vec2 start;
        glm::vec2 end;
        glm::vec4 color;
    };

    struct PointData {
        glm::vec2 pos;
        glm::vec4 color;
        float size;
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
    std::unique_ptr<IComponent>Clone(GameObject* NewClone) const override;
    
    // Getter for texture
    GLuint GetTextureID() const { return textureID; }
    ShapeType GetShapeType() const { return shape; }
    glm::vec4 GetColor() const { return color; }
    
    void SetColor(glm::vec4 newColor) {
        color = newColor ;
    }
    void setAlpha(float newAlpha) {
        color.a = newAlpha;
    }

    //setters for point and line
    void SetLine(const glm::vec2& start, const glm::vec2& end) { line = { start, end, color }; }
    void SetPoint(const glm::vec2& pos, float size) { point = { pos, color, size }; }

    LineData GetLine() const { return line; }
    PointData GetPoint() const { return point; }

    const char* GetShapeTypeName()
    {
        switch (static_cast<int>(GetShapeType()))
        {
        case static_cast<int>(ShapeType::Square):
            return "Square";
        case static_cast<int>(ShapeType::Circle):
            return "Circle";
        case static_cast<int>(ShapeType::Line):
            return "Line";
        case static_cast<int>(ShapeType::Point):
            return "Point";
        }
    }

private:
    ShapeType shape;
    GLuint textureID;
    glm::vec4 color;

    LineData line;
    PointData point;
};
