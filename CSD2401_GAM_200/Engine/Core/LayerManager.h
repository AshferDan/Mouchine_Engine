/* Start Header ************************************************************************/
/*!
\file       LayerManager.h
\author     Khalil Belabadia
\par        khalil.b@digipen.edu
\date       March 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>

/**
 * @brief Manages named game object layers and their enabled/disabled state.
 *
 * Each GameObject carries an integer layer index (0-31). Disabling a layer
 * via LayerManager causes all systems (rendering, physics, collision, scripting)
 * to skip GameObjects assigned to that layer.
 *
 * Layer 0 is always named "Default" and cannot be renamed.
 */
class LayerManager
{
public:
    static constexpr int MAX_LAYERS = 32;

    /**
     * @brief Enable or disable an entire layer.
     * @param layer Layer index (0 to MAX_LAYERS-1).
     * @param enabled True to enable, false to disable.
     */
    static void SetLayerEnabled(int layer, bool enabled);

    /**
     * @brief Check whether a layer is currently enabled.
     * @param layer Layer index (0 to MAX_LAYERS-1).
     * @return True if enabled (or index is out of range), false if disabled.
     */
    static bool IsLayerEnabled(int layer);

    /**
     * @brief Set a human-readable name for a layer.
     * @param layer Layer index (0 to MAX_LAYERS-1).
     * @param name  New name string.
     */
    static void SetLayerName(int layer, const std::string& name);

    /**
     * @brief Get the name assigned to a layer.
     * @param layer Layer index.
     * @return Name string, or empty string if index is out of range.
     */
    static const std::string& GetLayerName(int layer);

    /**
     * @brief Find a layer index by its name.
     * @param name Name to search for.
     * @return Layer index, or -1 if not found.
     */
    static int GetLayerByName(const std::string& name);

private:
    static bool        s_layerEnabled[MAX_LAYERS];
    static std::string s_layerNames[MAX_LAYERS];
};
