/* Start Header ************************************************************************/
/*!
\file       gameObjectSystem.cpp
\author     Jonathan Fam Qian Wei [100% - 20 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameObjectSystem.h"
#include "GameObject.h"
#include "Factory.h"
#include <iostream>
 GameObjectSystem::GameObjectSystem() : factory(GameObjectFactory::Instance()) {}


 /* brief
    this is to inititalize the gameobjectSystem.

    */
 void GameObjectSystem::Initialize() {
     std::cout << "GameObjectSystem initialized" << std::endl;
 }
 /* brief
   this is the update function that loops together engine loop to check whether the gameobjects are active and update accordingly to the 

   */

 void GameObjectSystem::Update(float timeslice) {
    
     auto& gameObjects = factory.GetAllGameObjects();

     for (auto& obj_unique_ptr : gameObjects) {
         GameObject* obj = obj_unique_ptr.get(); 

        
         if (obj == nullptr) {
             std::cerr << "ERROR: Null GameObject pointer found in factory list!" << std::endl;
             continue; 
         }

         if (obj->IsActive()) { 
             obj->Update(timeslice);
         }
     }
 }