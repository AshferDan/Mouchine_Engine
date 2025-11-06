/* Start Header ************************************************************************/
/*!
\file       GraphicsSystem.h
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "stb_image.h"
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
#include "DebugSystem.h"
#include <iostream>
#include <RenderComponent.h>
#include <TextComponent.h>
#include "CameraManager.h"
#include <fstream>
#include "MathLib.h"
#include <algorithm>
#include "RenderComponent.h"
#include "GameObjectSystem.h"
#include "Message.h"

/**
 * @class GraphicsSystem
 * @brief Handles window creation, rendering, and debug drawing utilities (lines, points, shapes).
 *
 * Typical usage:
 *   GraphicsSystem gfx(1280, 720);
 *   gfx.Initialize();
 *   while (!glfwWindowShouldClose(gfx.getWindow())) {
 *       gfx.Update(deltaTime);
 *   }
 */
class GraphicsSystem : public ISystem
{
public:
	GraphicsSystem();
	GraphicsSystem(GLint W, GLint H);
	~GraphicsSystem();

	// Initializes OpenGL context, shaders, and buffers.
	void Initialize() override;

	// Called each frame to handle rendering updates.
	void Update(float) override;

	/// Returns system name for debugging/identification.
	std::string GetName() override;

	/**
	 * @brief Loads a 2D texture from file.
	 * @param filepath Path to an image file (e.g., PNG, JPG).
	 * @return GLuint handle to the OpenGL texture.
	 *
	 * Example:
	 *   GLuint tex = GraphicsSystem::LoadTexture("assets/texture.png");
	 */
	static GLuint LoadTexture(const char *filepath);

	/// Returns the GLFW window pointer (for input handling, context checks).
	GLFWwindow *getWindow() { return window; }

	/**
	 * @brief Draws a debug line between two 2D points.
	 * @param start World/Screen coordinate for line start.
	 * @param end World/Screen coordinate for line end.
	 * @param color RGBA color of the line.
	 * @param vp View-projection matrix.
	 *
	 * Example:
	 *   gfx.DrawLine({0,0}, {1,1}, {1,0,0,1}, cameraVP);
	 */
	void DrawLine(const vec2 &start, const vec2 &end,
				  const vec4 &color, const mat4 &viewProjection);

	/**
	 * @brief Draws a debug point at a position.
	 * @param pos Position in 2D space.
	 * @param color RGBA color of the point.
	 * @param vp View-projection matrix.
	 */
	void DrawPoint(const vec2 &pos,
				   const vec4 &color, const mat4 &viewProjection);

	/**
	 * @brief Draws the outline of a rectangle.
	 * @param x Left corner x position.
	 * @param y Bottom corner y position.
	 * @param w Width of rectangle.
	 * @param h Height of rectangle.
	 * @param color RGBA color of outline.
	 * @param vp View-projection matrix.
	 */
	void DrawRectOutline(float x, float y, float w, float h,
						 const vec4 &color, const mat4 &viewProjection);

	/**
	 * @brief Draws a circle outline using line segments.
	 * @param cx Center X position.
	 * @param cy Center Y position.
	 * @param r Radius of circle.
	 * @param segments Number of line segments (higher = smoother).
	 * @param color RGBA color.
	 * @param vp View-projection matrix.
	 *
	 * Example:
	 *   gfx.DrawCircleOutline(0, 0, 1.0f, 32, {0,1,0,1}, cameraVP);
	 */
	void DrawCircleOutline(float cx, float cy, float r, int segments,
						   const vec4 &color, const mat4 &viewProjection);

	// Returns whether debug mode is enabled.
	bool isDebugModeOn() const { return debugMode; }

	// Toggles debug drawing mode (on/off).
	void toggleDebugMode() { debugMode = !debugMode; }

	// Returns the current view-projection matrix.
	const mat4 &getVp() const { return vp; }

	void SetLevelEditor(class LevelEditor *editor) { levelEditor = editor; }
	void DrawBatches();

	void RenderText(const std::map<char, Character> &characters, const std::string &text, float x, float y, float scale, const vec4 &color, bool screenSpace = true);

	void RenderGameViewportImGui(float& viewportSizeX, float& viewportSizeY);

	bool IsEditorMode() const { return editorMode; };

	void SetEditorMode() { editorMode = !editorMode; };

	GLuint GetFrameBufferTexture() const { return frameBufferTexture; };

	int GetWindowWidth() const { return Wwidth; };
	int GetWindowHeight() const { return Wheight; };

	void HandleMessage(const CreateObjectMessage& msg);
private:
	mat4 vp;				// Current view-projection matrix.
	bool debugMode = false; // Debug mode flag for showing shapes.
	GLFWwindow *window;		// GLFW window handle.
	GLSLShader shader;		// Shader used for debug rendering.
	GLSLShader textShader;
	GLSLShader debugShader;
	GLuint vao = 0, vbo = 0, tex = 0;		  // OpenGL buffers and texture.
	GLint Wwidth, Wheight;					  // Window dimensions.
	class LevelEditor *levelEditor = nullptr; // Level editor for camera control

	struct BatchVertex
	{
		vec3 pos;
		vec2 tex;
		vec4 color;
		vec2 uvScale;
		vec2 uvOffset;
		vec3 transform;
		vec2 scale;
	};

	struct RenderBatch
	{
		GLuint textureID;
		bool isCircle;
		std::vector<BatchVertex> vertices;
		std::vector<GLuint> indices;
		int layer;

		RenderBatch(GLuint texID, bool circle, int l) : textureID(texID), isCircle(circle), layer(l) {}
	};

	std::vector<RenderBatch> batches;
	GLuint batchVAO, batchVBO, batchEBO;
	GLuint textVAO, textVBO;
	const size_t MAX_BATCH_VERTICES = 20000; // 2500 quads * 4 vertices
	const size_t MAX_BATCH_INDICES = 30000;	 // 2500 quads * 6 indices

	GLuint frameBuffer, frameBufferTexture, depthRenderBuffer;
	int frameBufferWidth, frameBufferHeight;
	bool editorMode;

	void CreateFrameBuffer(int width, int height);
	void DeleteFrameBuffer();
	void ResizeFrameBuffer(int width, int height);
};
