#pragma once
#include "Message.h"
#include <iostream>

//Interface for anythat that handles messages
class IMessageHandler {
public:
	virtual ~IMessageHandler() = default;

	 // Generic fallback
	virtual void HandleMessage(const Message&) {
		std::cout << "[IMessageHandler] Unhandled message.\n";
	}

	// Specific overloads for each type
	virtual void HandleMessage(const KeyMessage& msg) { (void)msg; }
	virtual void HandleMessage(const PlaySoundMessage& msg) { (void)msg; }
	virtual void HandleMessage(const PauseSoundMessage& msg) { (void)msg; }
	virtual void HandleMessage(const CollisionMessage& msg) { (void)msg; }
	virtual void HandleMessage(const CreateObjectMessage& msg) { (void)msg; }

};
