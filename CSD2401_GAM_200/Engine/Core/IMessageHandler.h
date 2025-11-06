#pragma once
#include "Message.h"
#include <iostream>

//Interface for anythat that handles messages
class IMessageHandler {
public:
	virtual ~IMessageHandler() = default;

	 // Generic fallback
	virtual void HandleMessage(const Message& msg) {
		std::cout << "[IMessageHandler] Unhandled message.\n";
	}

	// Specific overloads for each type
	virtual void HandleMessage(const KeyMessage& msg) { (void)msg; }
	virtual void HandleMessage(const PlaySoundMessage& msg) { (void)msg; }
	virtual void HandleMessage(const PauseSoundMessage& msg) { (void)msg; }
	virtual void HandleMessage(const CollisionMessage& msg) { (void)msg; }
	virtual void HandleMessage(const CreateObjectMessage& msg) { (void)msg; }

};


// -------------- Inline dispatch implementations --------------
//inline void KeyMessage::Dispatch(IMessageHandler& handler) const {
//	handler.HandleMessage(*this);
//}
//
//inline void PlaySoundMessage::Dispatch(IMessageHandler& handler) const {
//	std::cout << "[Message] Dispatching PlaySoundMessage...\n";
//	handler.HandleMessage(*this);
//	std::cout << "[Message] Finished dispatch\n";
//}
//
//inline void PauseSoundMessage::Dispatch(IMessageHandler& handler) const {
//	std::cout << "[Message] Dispatching PauseSoundMessage...\n";
//	handler.HandleMessage(*this);
//	std::cout << "[Message] Finished dispatch\n";
//}
//
//inline void CollisionMessage::Dispatch(IMessageHandler& handler) const {
//	std::cout << "[Message] Dispatching CollisionMessage...\n";
//	handler.HandleMessage(*this);
//	std::cout << "[Message] Finished dispatch\n";
//}
//
//inline void CreateObjectMessage::Dispatch(IMessageHandler& handler) const {
//	std::cout << "[Message] Dispatching CreateObjectMessage..." << std::endl;
//	handler.HandleMessage(*this);
//	std::cout << "[Message] Finished dispatch" << std::endl;
//}
//
//inline void PlayAnimationMessage::Dispatch(IMessageHandler& handler) const {
//	std::cout << "[Message] Dispatching PlayAnimationMessage..." << std::endl;
//	handler.HandleMessage(*this);
//	std::cout << "[Message] Finished dispatch" << std::endl;
//}
