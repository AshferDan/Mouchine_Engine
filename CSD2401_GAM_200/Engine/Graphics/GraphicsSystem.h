#pragma once
#include "System.h"
#include "gameObject.h"
#include "glslshader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "factory.h"
#include "transform.h"
#include <iostream>
#include <RenderComponent.h>
#include <fstream>

class GraphicsSystem : public ISystem {
public:
	GraphicsSystem();
	GraphicsSystem(GLint W, GLint H);
	~GraphicsSystem();

	void Initialize() override;
	void Update(float dt) override;
	std::string GetName() override;
	static GLuint LoadTexture(const char* filepath);

	GLFWwindow* getWindow() {return window;}

	

private:
	GLFWwindow* window;
	GLSLShader shader;
	GLuint vao, vbo, tex;
	GLint Wwidth, Wheight;

};






