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

/**
 * @brief System responsible for handling pickup data. Collisions are entirely handled in c#, this is in c++ for easier accessing and serialising.
 */
class PickupSystem
{
public:
	PickupSystem() : _coins{}, _shells{}, currCoins{}, currShells{}
	{
		// prolly dont need this? bc it gets init on scene start anyw
		//InitPickups();
	}

	void CollectCoin() { currCoins++; std::cout << "\n\n\n\n\n\n"; }
	void CollectShell() { currShells++; }

	void SetTotalCoins(int val) { _coins = val; }
	void SetTotalShells(int val) { _shells = val; }
	int GetTotalCoins() const { return _coins; }
	int GetTotalShells() const { return _shells; }

	void SetCurrentCoins(int val) { currCoins = val; }
	void SetCurrentShells(int val) { currShells = val; }
	int GetCurrentCoins() const { return currCoins; }
	int GetCurrentShells() const { return currShells; }

	void InitPickups()
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
public:
	int _coins,  _shells;
	int currCoins, currShells;
};