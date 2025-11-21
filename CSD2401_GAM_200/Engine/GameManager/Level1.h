#pragma once
#include "GameManager.h"
#include "PauseState.h"
#include "InputManager.h"
#include "core.h"
#include "gameObject.h"
#include "factory.h"

class Level1 : public GameState {
public:
	const char* GetName() const { return "LEVEL 1"; }

	void Load();
	void Init();
	void Update(float dt);
	void Draw();
	void Free();
	void Unload();

private:
	float timer;
};