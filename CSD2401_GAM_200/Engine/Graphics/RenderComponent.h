#pragma once
#include "Component.h"
#include <GL/glew.h>
#include <iostream>


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
    void SetTextureID(GLuint newID) { textureID = newID; }
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


    /**
    * @brief Return the shape's name as a c string
    */
    const char* GetShapeTypeName()
    {
        switch (static_cast<int>(GetShapeType()))               // Return corresponding name depending on object's shape enum
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

/**
* @brief Takes in references to a string and a enum int, and depending on which value is null, it'll assign the null value the corresponding value of the shape id.
*        Basically used to get the shape enum from shape string name, and vice versa.
*        If no match for either, print to cout and return out.
* @param shapeName Ref to string containing shape name
* @param shapeEnum Ref to enum int containing value in shape enum
*/
inline void ShapeEnumToString(std::string& shapeName, RenderComponent::ShapeType& shapeEnum) {
    if (!shapeName.empty())                                         // If theres a name in shapeName, find a match and assign the enum int the corresponding shape value 
    {
        if (shapeName == "Square") { shapeEnum = RenderComponent::ShapeType::Square; }
        else if (shapeName == "Circle") { shapeEnum = RenderComponent::ShapeType::Circle; }
        else if (shapeName == "Line") { shapeEnum = RenderComponent::ShapeType::Line; }
        else if (shapeName == "Point") { shapeEnum = RenderComponent::ShapeType::Point; }
        else std::cout << "No matching shape name!\n";  return;
    }
    else                                                            // If string is empty, look to the enum int to assign the string the corresponding shape's name
    {
        switch (shapeEnum)
        {
        case RenderComponent::ShapeType::Square:
            shapeName = "Square";
            break;
        case RenderComponent::ShapeType::Circle:
            shapeName = "Circle";
            break;
        case RenderComponent::ShapeType::Line:
            shapeName = "Line";
            break;
        case RenderComponent::ShapeType::Point:
            shapeName = "Point";
            break;
        default: 
            std::cout << "No matching shape enum!\n";
            break;
        }
    }
}
