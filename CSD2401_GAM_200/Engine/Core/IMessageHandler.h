/* Start Header ************************************************************************/
/*!
\file       IMessage.h
\author     Lee Hwee Min [100% - 14 lines]
\par        l.hweemin@digipen.edu.sg
\date       Novemeber, 3, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Message.h"
#include <iostream>

// Interface for anythat that handles messages
class IMessageHandler
{
public:
    //! @brief Virtual destructor for safe polymorphic deletion.
    virtual ~IMessageHandler() = default;

    /*!
     * @brief Default handler for unrecognized or generic message types.
     * @param msg Reference to a generic Message object.
     */
    virtual void HandleMessage(const Message &msg)
    {
        std::cout << "[IMessageHandler] Unhandled message.\n";
    }

    /*!
     * @brief Handles keyboard input messages.
     * @param msg Reference to the KeyMessage object.
     */
    virtual void HandleMessage(const KeyMessage &msg) { (void)msg; }

    /*!
     * @brief Handles requests to play a sound.
     * @param msg Reference to the PlaySoundMessage object.
     */
    virtual void HandleMessage(const PlaySoundMessage &msg) { (void)msg; }

    /*!
     * @brief Handles requests to pause or resume a sound.
     * @param msg Reference to the PauseSoundMessage object.
     */
    virtual void HandleMessage(const PauseSoundMessage &msg) { (void)msg; }

    /*!
     * @brief Handles collision events between GameObjects.
     * @param msg Reference to the CollisionMessage object.
     */
    virtual void HandleMessage(const CollisionMessage &msg) { (void)msg; }

    /*!
     * @brief Handles requests to create new GameObjects.
     * @param msg Reference to the CreateObjectMessage object.
     */
    virtual void HandleMessage(const CreateObjectMessage &msg) { (void)msg; }
};
