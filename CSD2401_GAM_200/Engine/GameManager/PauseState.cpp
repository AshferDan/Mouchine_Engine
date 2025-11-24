#include "PauseState.h"
#include <iostream>
#include <Windows.h>

void PauseState::Load() {
	std::cout << "[PAUSE] Load Pause UI Assets\n";
}

void PauseState::Init() {
	std::cout << "[PAUSE] Init\n";
}

void PauseState::Update(float) {
	std::cout << "[PAUSE] Frozen... press ESC to resume\n";

	if (GetAsyncKeyState(VK_ESCAPE) & 1)
		GameManager::Instance().PopState();
}

void PauseState::Draw() {
	std::cout << "[PAUSE] Draw Pause Overlay\n";
}

void PauseState::Free() {
	std::cout << "[PAUSE] Free\n";
}

void PauseState::Unload() {
	std::cout << "[PAUSE] Unload\n";
}