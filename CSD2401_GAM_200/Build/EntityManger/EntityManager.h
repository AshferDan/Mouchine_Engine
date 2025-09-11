#pragma once

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <map>
#include "./graphics.h"
#include "assert.h"
#include <bitset>
#include <queue>
#include <array>
#include <glm/glm.hpp>

//struct Tranform {
//	glm::vec2 pos;
//	glm::vec2 rot;
//	glm::vec2 scale;
//};
//
//struct gravity {
//	int grav;
//};
//
//using ENTITY = std::uint32_t;
//using COMPONENT_TYPE = std::uint8_t;
//
//const ENTITY MAX_ENTITIES = 5000;
//const COMPONENT_TYPE MAX_COMPONENTS = 32;
//
//using Signature = std::bitset<MAX_COMPONENTS>;
//
//enum ENTITY_ID {
//	INVAILD,
//	Player,
//	Enemies,
//};
//
//class EntityManager {
//public:
//	EntityManager()
//	{
//		// Initialize the queue with all possible entity IDs
//		for (ENTITY entity = 0; entity < MAX_ENTITIES; ++entity)
//		{
//			mAvailableEntities.push(entity);
//		}
//	}
//
//	ENTITY CreateEntity()
//	{
//		assert(mLivingEntityCount < MAX_ENTITIES && "Too many entities in existence.");
//
//		// Take an ID from the front of the queue
//		ENTITY id = mAvailableEntities.front();
//		mAvailableEntities.pop();
//		++mLivingEntityCount;
//
//		return id;
//	}
//
//	void DestroyEntity(ENTITY entity)
//	{
//		assert(entity < MAX_ENTITIES && "Entity out of range.");
//
//		// Invalidate the destroyed entity's signature
//		mSignatures[entity].reset();
//
//		// Put the destroyed ID at the back of the queue
//		mAvailableEntities.push(entity);
//		--mLivingEntityCount;
//	}
//
//	void SetSignature(ENTITY entity, Signature signature)
//	{
//		assert(entity < MAX_ENTITIES && "Entity out of range.");
//
//		// Put this entity's signature into the array
//		mSignatures[entity] = signature;
//	}
//
//	Signature GetSignature(ENTITY entity)
//	{
//		assert(entity < MAX_ENTITIES && "Entity out of range.");
//
//		// Get this entity's signature from the array
//		return mSignatures[entity];
//	}
//
//private:
//	// Queue of unused entity IDs
//	std::queue<ENTITY> mAvailableEntities{};
//
//	// Array of signatures where the index corresponds to the entity ID
//	std::array<Signature, MAX_ENTITIES> mSignatures{};
//
//	// Total living entities - used to keep limits on how many exist
//	uint32_t mLivingEntityCount{};
//
//};
//
//class ComponentManager {
//public:
//	void RegisterComp() { std::cout << " RegisteredComp "; };
//	COMPONENT_TYPE GetComp();
//	void DestroyComp();
//
//private:
//	std::unordered_map<const char*, COMPONENT_TYPE> mCompType{};
//
//};