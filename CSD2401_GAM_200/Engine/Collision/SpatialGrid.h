/* Start Header ************************************************************************/
/*!
\file       SpatialGrid.h
\author     Spatial Partitioning System
\date       November, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <vector>
#include <unordered_map>
#include "MathLib.h"

class GameObject;
class ColliderComponent;

// At top of file
const float WORLD_WIDTH = 4000.0f;
const float WORLD_HEIGHT = 3000.0f;
const float CELL_SIZE = 200.0f;

/**
 * @brief Spatial hash grid for broad-phase collision detection
 * Divides the world into fixed-size cells and hashes objects into them
 */
class SpatialGrid {
public:
    /**
     * @brief Constructs a spatial grid with specified parameters
     * @param worldWidth Width of the game world
     * @param worldHeight Height of the game world
     * @param cellSize Size of each grid cell (same for width and height)
     */
    SpatialGrid(float worldWidth = WORLD_WIDTH, float worldHeight = WORLD_HEIGHT, float cellSize = CELL_SIZE);

    /**
     * @brief Clears all objects from the grid
     */
    void Clear();

    /**
     * @brief Inserts an object into the grid based on its AABB
     * @param obj GameObject to insert
     * @param collider ColliderComponent containing the object's bounds
     */
    void Insert(GameObject* obj, ColliderComponent* collider);

    /**
     * @brief Queries nearby objects that might collide with the given object
     * @param obj GameObject to query around
     * @param collider ColliderComponent containing the object's bounds
     * @return Vector of GameObjects in nearby cells
     */
    std::vector<GameObject*> Query(GameObject* obj, ColliderComponent* collider) const;

    /**
     * @brief Gets the total number of cells in the grid
     * @return Number of cells (columns * rows)
     */
    int GetCellCount() const { return numCols * numRows; }

    /**
     * @brief Gets grid statistics for debugging
     * @param avgObjectsPerCell Output: average objects per occupied cell
     * @param maxObjectsInCell Output: maximum objects in any cell
     * @param occupiedCells Output: number of cells with at least one object
     */
    void GetStats(float& avgObjectsPerCell, int& maxObjectsInCell, int& occupiedCells) const;

private:
    /**
     * @brief Converts world position to grid cell coordinates
     * @param worldX World X position
     * @param worldY World Y position
     * @param outCol Output: column index
     * @param outRow Output: row index
     */
    void WorldToCell(float worldX, float worldY, int& outCol, int& outRow) const;

    /**
     * @brief Converts cell coordinates to a hash key
     * @param col Column index
     * @param row Row index
     * @return Hash key for accessing the cell
     */
    int CellToHash(int col, int row) const;

    /**
     * @brief Clamps cell coordinates to valid range
     * @param col Column to clamp
     * @param row Row to clamp
     */
    void ClampCell(int& col, int& row) const;

    // Grid configuration
    float worldWidth;
    float worldHeight;
    float cellSize;
    int numCols;
    int numRows;
    vec2 worldMin; // Bottom-left corner of world

    // Hash map: cell hash -> list of objects in that cell
    std::unordered_map<int, std::vector<GameObject*>> cells;
};
