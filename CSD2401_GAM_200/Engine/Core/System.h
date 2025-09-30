#pragma once
#include <string>
#include "Message.h" 

class ISystem
{
public:
    ///Systems can receive all message send to the Core. 
    ///See Message.h for details.
    //virtual void SendMessage(Message* message) { UNREFERENCED_PARAMETER(message); };
     //virtual void SendMessage(Message* message) { 
     //   (void) message;  // suppress unused parameter warning
     //}

    ///All systems are updated every game frame.
    virtual void Update(float timeslice) = 0;

    ///All systems provide a string name for debugging.
    virtual std::string GetName() = 0;

    ///Initialize the system.
    virtual void Initialize() {}

    ///All systems need a virtual destructor to have their destructor called 
    virtual ~ISystem() {}
};
