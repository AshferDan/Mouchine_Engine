/* Start Header ************************************************************************/
/*!
\file       SpatialGrid.cpp
\author     Spatial Partitioning System
\date       November, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "SpatialGrid.h"
#include "GameObject.h"
#include "ColliderComponent.h"
#include <cmath>
#include <algorithm>

/**
 * @brief Constructs spatial grid and calculates grid dimensions
 */
SpatialGrid::SpatialGrid(float worldWidth, float worldHeight, float cellSize)
    : worldWidth(worldWidth), worldHeight(worldHeight), cellSize(cellSize)
{
    // Calculate number of cells needed
    numCols = static_cast<int>(std::ceil(worldWidth / cellSize));
    numRows = static_cast<int>(std::ceil(worldHeight / cellSize));

    // World is centered at origin (0, 0)
    worldMin = vec2(-worldWidth * 0.5f, -worldHeight * 0.5f);
}

/**
 * @brief Clears all objects from the grid
 */
void SpatialGrid::Clear() {
    cells.clear();
}

/**
 * @brief Converts world position to grid cell coordinates
 */
void SpatialGrid::WorldToCell(float worldX, float worldY, int& outCol, int& outRow) const {
    // Translate world position to grid space (0-based)
    float localX = worldX - worldMin.x;
    float localY = worldY - worldMin.y;

    // Calculate cell indices
    outCol = static_cast<int>(std::floor(localX / cellSize));
    outRow = static_cast<int>(std::floor(localY / cellSize));

    // Clamp to valid range
    ClampCell(outCol, outRow);
}

/**
 * @brief Clamps cell coordinates to grid bounds
 */
void SpatialGrid::ClampCell(int& col, int& row) const {
    if (col < 0) col = 0;
    if (col >= numCols) col = numCols - 1;
    if (row < 0) row = 0;
    if (row >= numRows) row = numRows - 1;
}

/**
 * @brief Converts cell coordinates to unique hash key
 */
int SpatialGrid::CellToHash(int col, int row) const {
    return row * numCols + col;
}

/**
 * @brief Inserts object into all cells it overlaps
 */
void SpatialGrid::Insert(GameObject* obj, ColliderComponent* collider) {
    if (!obj || !collider) return;

    AABB aabb = collider->GetAABB();

    // Find which cells the AABB overlaps
    int minCol, minRow, maxCol, maxRow;
    WorldToCell(aabb.min.x, aabb.min.y, minCol, minRow);
    WorldToCell(aabb.max.x, aabb.max.y, maxCol, maxRow);

    // Insert into all overlapping cells
    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            int hash = CellToHash(col, row);
            cells[hash].push_back(obj);
        }
    }
}

/**
 * @brief Queries all objects in cells near the given object
 */
std::vector<GameObject*> SpatialGrid::Query(GameObject* obj, ColliderComponent* collider) const {
    std::vector<GameObject*> results;
    if (!obj || !collider) return results;

    AABB aabb = collider->GetAABB();

    // Find which cells to query (object's cells + neighbors)
    int minCol, minRow, maxCol, maxRow;
    WorldToCell(aabb.min.x, aabb.min.y, minCol, minRow);
    WorldToCell(aabb.max.x, aabb.max.y, maxCol, maxRow);

    // Expand by 1 cell in each direction to catch neighbors
    minCol = (minCol > 0) ? minCol - 1 : 0;
    minRow = (minRow > 0) ? minRow - 1 : 0;
    maxCol = (maxCol < numCols - 1) ? maxCol + 1 : numCols - 1;
    maxRow = (maxRow < numRows - 1) ? maxRow + 1 : numRows - 1;

    // Collect all objects from relevant cells
    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            int hash = CellToHash(col, row);
            auto it = cells.find(hash);
            if (it != cells.end()) {
                // Add all objects from this cell
                for (GameObject* other : it->second) {
                    // Avoid duplicates and self-collision
                    if (other != obj &&
                        std::find(results.begin(), results.end(), other) == results.end()) {
                        results.push_back(other);
                    }
                }
            }
        }
    }

    return results;
}

/**
 * @brief Calculates statistics about grid usage
 */
void SpatialGrid::GetStats(float& avgObjectsPerCell, int& maxObjectsInCell, int& occupiedCells) const {
    occupiedCells = 0;
    maxObjectsInCell = 0;
    int totalObjects = 0;

    for (const auto& pair : cells) {
        int count = static_cast<int>(pair.second.size());
        if (count > 0) {
            occupiedCells++;
            totalObjects += count;
            if (count > maxObjectsInCell) {
                maxObjectsInCell = count;
            }
        }
    }

    avgObjectsPerCell = (occupiedCells > 0)
        ? static_cast<float>(totalObjects) / occupiedCells
        : 0.0f;
}