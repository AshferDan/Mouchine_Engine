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
	
	void DrawLine(const glm::vec2& start, const glm::vec2& end,
		const glm::vec4& color, const glm::mat4& vp);

	void DrawPoint(const glm::vec2& pos,
		const glm::vec4& color, const glm::mat4& vp);

	void DrawRectOutline(float x, float y, float w, float h,
		const glm::vec4& color, const glm::mat4& vp);

	void DrawCircleOutline(float cx, float cy, float r, int segments,
		const glm::vec4& color, const glm::mat4& vp);

private:
	GLFWwindow* window;
	GLSLShader shader;
	GLuint vao = 0, vbo = 0, tex = 0;
	GLint Wwidth, Wheight;

};






