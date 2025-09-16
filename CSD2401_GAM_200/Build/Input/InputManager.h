#include <iostream>
#include <GLFW/glfw3.h>
namespace InputManager {
	static bool keyA;
	static bool keyD;
	static bool keyW;
	static bool keyS;

	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
}