#pragma once 

#include "Core.h"
#include "Factory.h"
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include "ImGuiSystem.h"
#include "DebugSystem.h"
#include "Transform.h"
#include "Collision.h"
#include "Collidercomponent.h"
#include "Physics.h"
#include "RenderComponent.h"
#include <iostream>
#include <cmath>
#include "Animator.hpp"
#include "ResourceManager.h"
#include "audio.h"
#include "InputManager.h"

void Main_init();
void Main_Update();
void Main_Draw();
void Main_Free();
void Main_Unload();