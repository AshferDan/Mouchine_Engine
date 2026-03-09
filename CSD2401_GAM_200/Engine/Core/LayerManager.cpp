/* Start Header ************************************************************************/
/*!
\file       LayerManager.cpp
\author     Khalil Belabadia
\par        khalil.b@digipen.edu
\date       March 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "LayerManager.h"
#include <algorithm>

// All layers enabled by default; layer 0 is "Default"
bool LayerManager::s_layerEnabled[MAX_LAYERS] = {
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true,
    true, true, true, true, true, true, true, true
};

std::string LayerManager::s_layerNames[MAX_LAYERS] = {
    "Default", "Background", "Player", "Enemies", "UI", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", ""
};

void LayerManager::SetLayerEnabled(int layer, bool enabled)
{
    if (layer < 0 || layer >= MAX_LAYERS)
        return;
    s_layerEnabled[layer] = enabled;
}

bool LayerManager::IsLayerEnabled(int layer)
{
    if (layer < 0 || layer >= MAX_LAYERS)
        return true; // Unknown layers are treated as enabled
    return s_layerEnabled[layer];
}

void LayerManager::SetLayerName(int layer, const std::string& name)
{
    if (layer < 0 || layer >= MAX_LAYERS)
        return;
    s_layerNames[layer] = name;
}

const std::string& LayerManager::GetLayerName(int layer)
{
    static const std::string empty;
    if (layer < 0 || layer >= MAX_LAYERS)
        return empty;
    return s_layerNames[layer];
}

int LayerManager::GetLayerByName(const std::string& name)
{
    for (int i = 0; i < MAX_LAYERS; ++i)
    {
        if (s_layerNames[i] == name)
            return i;
    }
    return -1;
}
