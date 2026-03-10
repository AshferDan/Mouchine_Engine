/* Start Header ************************************************************************/
/*!
\file       LayerComponent.h
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
#include "Component.h"

/**
 * @brief Editor component that exposes a GameObject's game layer assignment.
 *
 * Adding this component to a GameObject makes its layer selectable in the
 * level editor inspector. The actual layer index is stored on the GameObject
 * itself (via GetGameLayer / SetGameLayer); this component is purely a UI anchor.
 */
class LayerComponent : public IComponent
{
public:
    explicit LayerComponent(GameObject* owner) : IComponent(owner) {}

    void Initialize() override {}
    void Update(float) override {}
    void Destroy() override {}

    std::string GetName() const override { return "LayerComponent"; }

    std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override;
};
