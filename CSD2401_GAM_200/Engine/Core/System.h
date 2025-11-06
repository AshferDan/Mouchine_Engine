/* Start Header ************************************************************************/
/*!
\file       system.h
\author     Jonathan Fam Qian Wei [100% - 10 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include <string>
#include "Message.h"
#include "IMessageHandler.h"

class ISystem: public IMessageHandler
{
public:
    //virtual void SendMessage(Message* message) { UNREFERENCED_PARAMETER(message); };

    /**
     * @brief Called every game frame to update the system's logic.
     * @param timeslice The time elapsed since the last frame (delta time).
     */
    virtual void Update(float timeslice) = 0;

    /**
     * @brief Gets the debug name of the system.
     * @return A string representing the system's name.
     */
    virtual std::string GetName() = 0;

    /**
     * @brief Initializes the system.
     */
    virtual void Initialize() {}

    /**
     * @brief Virtual destructor to ensure proper cleanup of derived system classes.
     */
    virtual ~ISystem() {}
};