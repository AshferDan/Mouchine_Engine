/* Start Header ************************************************************************/
/*!
\file       Utils.h
\author     Muhammad Ashfer [100% - xx lines]
\par        m.binafzalkhan@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once 

#include "Core.h"
#include "Factory.h"
#include "GameObjectSystem.h"
#include <iostream>
#include <cmath>
#include "ScriptAPI.h"
#include "ScriptingSystem.h"
#include "Serialization.h"
#include "ScriptComponent.h"

// Just a bunch of game related utility/ helper functions

//namespace Pickups
//{
//	
//
//	struct Pickup
//	{
//		PickupType pickupType;
//		float x, y;
//		float value;
//		bool collected = false;
//	};
//
//	void PopulatePickups(std::vector<Pickup>& LevelPickups);
//}
//enum PickupType
//{
//	Coin = 0,
//	Shell
//};

class PickupSystem
{
public:
	PickupSystem() : _coins{}, _shells{}, currCoins{}, currShells{}
	{
		for (auto const& go : GameObjectFactory::Instance().GetAllGameObjects())
		{
			if (go.get()->GetName() == "PU_Coin")
			{
				_coins++;
			}
			else if (go.get()->GetName() == "PU_Shell")
			{
				_shells++;
			}
		}
	}

	void CollectCoin() { currCoins++; }
	void CollectShell() { currShells++; }

	void SetTotalCoins(int val) { _coins = val; }
	void SetTotalShells(int val) { _shells = val; }
	int GetTotalCoins(int) const { return _coins; }
	int GetTotalShells(int) const { return _shells; }

	void SetCurrentCoins(int val) { currCoins = val; }
	void SetCurrentShells(int val) { currShells = val; }
	int GetCurrentCoins() const { return currCoins; }
	int GetCurrentShells() const { return currShells; }
private:
	int _coins,  _shells;
	int currCoins, currShells;
};