/* Start Header ************************************************************************/
/*!
\file       DebugSystem.h
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "System.h"
#include "GraphicsSystem.h"

/**
 * @class DebugSystem
 * @brief Provides debug drawing utilities (lines, rectangles, circles, points) for visualization.
 *
 * This system allows adding debug shapes to be drawn each frame.
 * Shapes are stored internally and cleared every frame (if desired).
 */
class DebugSystem : public ISystem {
public:
    /**
     * @brief Construct a new DebugSystem object.
     */
    DebugSystem() {};

    /**
     * @brief Destroy the DebugSystem object.
     */
    ~DebugSystem() {};

    /**
     * @brief Initializes the DebugSystem.
     */
    void Initialize() override;

    /**
     * @brief Updates the DebugSystem each frame.
     * @param dt Delta time in seconds since the last frame.
     */
    void Update(float dt) override;

    /**
     * @brief Returns the system's name (for debugging/logging).
     * @return std::string Name of the system.
     */
    std::string GetName() override;

    /**
     * @brief Draw a line for debugging.
     * @param start Starting point of the line.
     * @param end Ending point of the line.
     * @param color Color of the line (RGBA).
     */
    void DrawLine(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color);

    /**
     * @brief Draw a rectangle for debugging.
     * @param pos Center position of the rectangle.
     * @param size Width (x) and height (y) of the rectangle.
     * @param color Color of the rectangle outline (RGBA).
     */
    void DrawRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color);

    /**
     * @brief Draw a circle for debugging.
     * @param center Center of the circle.
     * @param radius Radius of the circle.
     * @param color Color of the circle outline (RGBA).
     * @param segments Number of line segments used to approximate the circle (default = 36).
     */
    void DrawCircle(const glm::vec2& center, float radius, const glm::vec4& color, int segments = 36);

    /**
     * @brief Draw a point for debugging.
     * @param ppos Position of the point.
     * @param color Color of the point (RGBA).
     */
    void DrawPoint(const glm::vec2& ppos, glm::vec4 color);

    /**
     * @brief Clears all debug shapes stored in the system.
     */
    void Clear();

    /**
     * @struct DebugShape
     * @brief Represents a single debug shape (line, rectangle, circle, point).
     */
    struct DebugShape {
        /**
         * @enum Type
         * @brief The type of debug shape.
         */
        enum Type { LINE, RECT, CIRCLE, POINT } type;

        glm::vec2 p1;     ///< First position (meaning depends on shape type).
        glm::vec2 p2;     ///< Second position (used for lines/rectangles).
        float radius;     ///< Radius (used for circles).
        glm::vec4 color;  ///< Color (RGBA).
        int segments;     ///< Segments (used for circle approximation).
    };

private:
    std::vector<DebugShape> shapes; ///< Internal list of shapes to be drawn.

public:
    /**
     * @brief Returns the list of all shapes currently stored.
     * @return const std::vector<DebugShape>& Read-only reference to shapes.
     */
    const std::vector<DebugShape>& GetShapes() const { return shapes; }
};
