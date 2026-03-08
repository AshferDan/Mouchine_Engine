/* Start Header ************************************************************************/
/*!
\file       LayerComponent.cpp
\author     Khalil Belabadia
\par        khalil.b@digipen.edu
\date       March 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "LayerComponent.h"
#include "gameObject.h"

std::unique_ptr<IComponent> LayerComponent::Clone(GameObject* newOwner) const
{
    auto clone = std::make_unique<LayerComponent>(newOwner);
    // Copy the layer assignment from the original owner to the new owner
    newOwner->SetGameLayer(owner->GetGameLayer());
    return clone;
}
