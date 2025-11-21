#pragma once
#include <stack>
#include <memory>
#include <iostream>

extern bool g_UseGameStates;

// Base class for all game states
class GameState {
public:
	virtual ~GameState() {}

	virtual const char* GetName() const = 0;

	virtual void Load() {}
	virtual void Init() {}
	virtual void Update(float dt) {}
	virtual void Draw() {}
	virtual void Free() {}
	virtual void Unload() {}
};

// Push/pop state system
class GameManager {
public:
	static GameManager& Instance();

	void ChangeState(std::unique_ptr<GameState> state); // Remove current, add new
	void PushState(std::unique_ptr<GameState> state);	// Add on top of current
	void PopState();									// Remove top state
	
	void Update(float dt); // Call update on active state only
	void Draw();		   // Call Draw on active state only

	void GameStateLoop(float dt);

	const char* GetActiveStateName() const;
	unsigned int GetStackSize() const;

	static bool g_UseGameStates;
private:
	GameManager() {}
	std::stack<std::unique_ptr<GameState>> m_stateStack;
};