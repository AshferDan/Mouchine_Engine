#pragma once
#include "GameManager.h"

class PauseState : public GameState {
public:
	const char* GetName() const { return "PAUSE"; }

	void Load();
	void Init();
	void Update(float dt);
	void Draw();
	void Free();
	void Unload();
};