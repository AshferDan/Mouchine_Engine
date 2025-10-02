
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

class ISystem
{
public:
    ///Systems can receive all message send to the Core. 
    ///See Message.h for details.
    //virtual void SendMessage(Message* message) { UNREFERENCED_PARAMETER(message); };

    ///All systems are updated every game frame.
    virtual void Update(float timeslice) = 0;

    ///All systems provide a string name for debugging.
    virtual std::string GetName() = 0;

    ///Initialize the system.
    virtual void Initialize() {}

    ///All systems need a virtual destructor to have their destructor called 
    virtual ~ISystem() {}
};
