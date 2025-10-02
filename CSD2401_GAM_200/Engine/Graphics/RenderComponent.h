/* Start Header ************************************************************************/
/*!
\file       GraphicsSystem.h
\author     Guan Shao Jun [50%]
\co-author 
\par        shaojun.g@digipen.edu,
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include "Component.h"
#include <GL/glew.h>


/*! @class RenderComponent
    @brief Component for rendering shapes or textured objects.
*/
class RenderComponent : public IComponent {
public:

    /*! @enum ShapeType
        @brief Types of shapes supported by RenderComponent.
    */
    enum class ShapeType {
        Square, /*!< Square shape */
        Circle, /*!< Circle shape */
        Line,   /*!< Line shape */
        Point   /*!< Point shape */
    };

    /*! @struct LineData
        @brief Data structure for a line (start/end points and color).
    */
    struct LineData {
        glm::vec2 start; /*!< Start position of the line */
        glm::vec2 end;   /*!< End position of the line */
        glm::vec4 color; /*!< Line color */
    };

    /*! @struct PointData
        @brief Data structure for a point (position, color, size).
    */
    struct PointData {
        glm::vec2 pos;   /*!< Position of the point */
        glm::vec4 color; /*!< Point color */
        float size;      /*!< Point size */
    };

    /*! @brief Constructor for a textured object
        @param owner The GameObject this component belongs to
        @param texID OpenGL texture ID
    */
    RenderComponent(GameObject* owner, GLuint texID)
        : IComponent(owner), shape(ShapeType::Square), textureID(texID), color(1.0f), line(), point() {

    }

    /*! @brief Constructor for a shape
        @param owner The GameObject this component belongs to
        @param type Shape type
        @param color Shape color
    */
    RenderComponent(GameObject* owner, ShapeType type, const glm::vec4& color)
        : IComponent(owner), shape(type), textureID(0), color(color), line(), point() {
    };

    /*! @brief Lifecycle override: initialize component */
    void Initialize() override {}

    /*! @brief Lifecycle override: update component */
    void Update(float) override {}

    /*! @brief Lifecycle override: destroy component */
    void Destroy() override {}

    /*! @brief Get the component name
        @return Name of the component
    */
    std::string GetName() const override { return "RenderComponent"; }

    /*! @brief Clone this component for a new GameObject
        @param NewClone Pointer to the new GameObject
        @return Unique pointer to the cloned component
    */
    std::unique_ptr<IComponent> Clone(GameObject* NewClone) const override;

    /*! @brief Get texture ID
        @return OpenGL texture ID
    */
    GLuint GetTextureID() const { return textureID; }

    /*! @brief Set texture ID
        @param newID New OpenGL texture ID
    */
    void SetTextureID(GLuint newID) { textureID = newID; }

    /*! @brief Get shape type
        @return ShapeType of this component
    */
    ShapeType GetShapeType() const { return shape; }

    /*! @brief Get the current color
        @return Color as glm::vec4
    */
    glm::vec4 GetColor() const { return color; }

    /*! @brief Set the color of the shape
        @param newColor New color
    */
    void SetColor(glm::vec4 newColor) { color = newColor; }

    /*! @brief Set alpha (transparency) of the shape
        @param newAlpha New alpha value
    */
    void setAlpha(float newAlpha) { color.a = newAlpha; }

    /*! @brief Set line data
        @param start Start position
        @param end End position
    */
    void SetLine(const glm::vec2& start, const glm::vec2& end) { line = { start, end, color }; }

    /*! @brief Set point data
        @param pos Position of the point
        @param size Point size
    */
    void SetPoint(const glm::vec2& pos, float size) { point = { pos, color, size }; }

    /*! @brief Get line data
        @return LineData structure
    */
    LineData GetLine() const { return line; }

    /*! @brief Get point data
        @return PointData structure
    */
    PointData GetPoint() const { return point; }

    /**
     * @brief Returns the name of the current shape type as a C-string.
     *
     * This function converts the ShapeType enum of this RenderComponent
     * into a human-readable string.
     *
     * @return const char* Name of the shape ("Square", "Circle", "Line", or "Point").
     */
    const char* GetShapeTypeName()
    {
        switch (shape) // use the member variable directly
        {
        case ShapeType::Square:
            return "Square";
        case ShapeType::Circle:
            return "Circle";
        case ShapeType::Line:
            return "Line";
        case ShapeType::Point:
            return "Point";
        default:
            return "Unknown"; // handle unexpected values
        }
    }



private:
    ShapeType shape; /*!< Shape type of this component */
    GLuint textureID; /*!< OpenGL texture ID */
    glm::vec4 color; /*!< Color of the shape */

    LineData line; /*!< Line data if ShapeType::Line */
    PointData point; /*!< Point data if ShapeType::Point */
};

/**
 * @brief Synchronizes a shape name string and a ShapeType enum.
 *
 * If `shapeName` is not empty, this function converts it to the corresponding
 * `RenderComponent::ShapeType` enum. If `shapeName` is empty, it converts the
 * given `shapeEnum` to its string representation.
 *
 * @param shapeName Reference to the string representing the shape (input or output)
 * @param shapeEnum Reference to the ShapeType enum (input or output)
 */
inline void ShapeEnumToString(std::string& shapeName, RenderComponent::ShapeType& shapeEnum) {
    // If the string is already provided, map it to the enum
    if (!shapeName.empty())
    {
        if (shapeName == "Square") {
            shapeEnum = RenderComponent::ShapeType::Square;
        }
        else if (shapeName == "Circle") {
            shapeEnum = RenderComponent::ShapeType::Circle;
        }
        else if (shapeName == "Line") {
            shapeEnum = RenderComponent::ShapeType::Line;
        }
        else if (shapeName == "Point") {
            shapeEnum = RenderComponent::ShapeType::Point;
        }
        else return; // Unrecognized string, do nothing
    }
    else // If the string is empty, convert enum to string
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
            break; // Unknown enum value, do nothing
        }
    }
}
