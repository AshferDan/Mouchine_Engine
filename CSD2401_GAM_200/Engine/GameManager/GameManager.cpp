#include "GameManager.h"

GameManager& GameManager::Instance() {
	static GameManager instance;
	return instance;
}

// Replace current state with a new one
void GameManager::ChangeState(std::unique_ptr<GameState> state) {
	if (!m_stateStack.empty()) {
		std::cout << "[GAME MANAGER] CLEANUP: " << m_stateStack.top()->GetName() << std::endl;
		m_stateStack.top()->Free();
		m_stateStack.top()->Unload();
		m_stateStack.pop();
	}

	std::cout << "[GAME MANAGER] CHANGE TO " << state->GetName() << "\n";
	state->Load();
	state->Init();
	m_stateStack.push(std::move(state));
}

// Push new state on top of old one (eg. for Pause)
void GameManager::PushState(std::unique_ptr<GameState> state) {
	std::cout << "[GAME MANAGER] PUSH - " << state->GetName() << "\n";
	state->Load();
	state->Init();
	m_stateStack.push(std::move(state));
}

// Pop top state (return to previous)
void GameManager::PopState() {
	if (!m_stateStack.empty()) {
		std::cout << "[GAME MANAGER] POP - " << m_stateStack.top()->GetName() << "\n";
		m_stateStack.top()->Free();
		m_stateStack.top()->Unload();
		m_stateStack.pop();
	}
}

void GameManager::Update(float dt) {
	if (!m_stateStack.empty()) {
		m_stateStack.top()->Update(dt);
	}
}

void GameManager::Draw() {
	if (!m_stateStack.empty()) {
		m_stateStack.top()->Draw();
	}
}

const char* GameManager::GetActiveStateName() const {
	if (m_stateStack.empty()) return "[NO ACTIVE STATE]";
	return m_stateStack.top()->GetName();
}

unsigned int GameManager::GetStackSize() const {
	return (unsigned int)m_stateStack.size();
}

void GameManager::GameStateLoop(float dt)
{
	GameManager::Instance().Update(dt);
	GameManager::Instance().Draw();
}