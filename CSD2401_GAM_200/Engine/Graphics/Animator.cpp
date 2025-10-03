/* Start Header ************************************************************************/
/*!
\file       Animator.cpp
\author     Muhammad Ashfer [100% - 117 lines]
\par        m.binafzalkhan@digipen.edu
\date       October, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Animator.hpp"
/**
 * @brief Ctor with GO, texture file path, amount of rows and cols for that spritesheet
 */
Spritesheet::Spritesheet(GameObject *owner, const char *textureFile, int numRows, int numCols)
	: IComponent(owner),
	  rows(numRows),
	  cols(numCols),
	  currRow(0),
	  currCol(0),
	  uv_width(1.0f / (float)numCols),	// Calculate 1/Cols
	  uv_height(1.0f / (float)numRows), // Calculate 1/Rows
	  timePerFrame(0.0f),
	  currentFrameTime(0.0f),
	  startFrame(0),
	  endFrame(0),
	  textureID(GraphicsSystem::LoadTexture(textureFile))
{
}

/**
 * @brief Ctor with ref to existing spritesheet object, and a GO
 */
Spritesheet::Spritesheet(Spritesheet const &rhs, GameObject *NewClone)
	: IComponent(NewClone), // Assign to the new GameObject
	  rows(rhs.rows),
	  cols(rhs.cols),
	  currRow(rhs.currRow),
	  currCol(rhs.currCol),
	  uv_width(rhs.uv_width),
	  uv_height(rhs.uv_height),
	  timePerFrame(rhs.timePerFrame),
	  currentFrameTime(rhs.currentFrameTime),
	  startFrame(rhs.startFrame),
	  endFrame(rhs.endFrame),
	  textureID(rhs.textureID)
{
}

/**
 * @brief Unique ptr for when the object is cloned!
 */
std::unique_ptr<IComponent> Spritesheet::Clone(GameObject *NewClone) const
{
	return std::make_unique<Spritesheet>(*this, NewClone);
}

/**
 * @brief Update spritesheet timers
 */
void Spritesheet::Update(float dt)
{
	if (startFrame > endFrame)
		return; // Invalid!

	currentFrameTime += dt; // Incr timers

	if (currentFrameTime >= timePerFrame) // timer has reached beyond time per frame, change frames
	{
		currentFrameTime -= timePerFrame;

		int currentFrameIndex = currRow * cols + currCol; // Readjust rows and cols
		int nextFrameIndex = currentFrameIndex + 1;

		if (nextFrameIndex > endFrame)
		{
			nextFrameIndex = startFrame;
		}
		currRow = nextFrameIndex / cols; // Reassign new rows and cols, showing the next frame of animation
		currCol = nextFrameIndex % cols;
	}
}

/**
 * @brief Get UV scale of 'mesh'. For drawing texture on
 */
glm::vec2 Spritesheet::GetUVScale() const
{
	return glm::vec2(uv_width, uv_height);
}

/**
 * @brief Padding for spritesheets
 */
glm::vec2 Spritesheet::GetUVOffset() const
{
	return glm::vec2(uv_width * currCol, uv_height * currRow);
}

/**
 * @brief Loading new spritesheet
 */
void Spritesheet::LoadNewSpritesheet(const char *textureFile, int numRows, int numCols)
{
	if (textureID)
	{
		Unload();
	} // If valid textureID, unload texture

	textureID = GraphicsSystem::LoadTexture(textureFile); // Reload texture

	rows = numRows; // Set Rows and Cols
	cols = numCols;
	uv_width = 1.0f / (float)numCols; // Set new uv WxH
	uv_height = 1.0f / (float)numRows;

	currRow = 0; // Reset priv members
	currCol = 0;
	startFrame = 0;
	endFrame = 0;

	if (auto *render = owner->GetComponent<RenderComponent>())
	{ // Set texture ID of new animation
		render->SetTextureID(textureID);
	}
}

/**
 * @brief Set new animation
 */
void Spritesheet::SetAnimation(int startFrameIdx, int endFrameIdx, float fps)
{
	startFrame = startFrameIdx; // Set starting and ending frames and time between frames. (We're still on the existing spritesheet)
	endFrame = endFrameIdx;
	timePerFrame = 1.0f / fps;
	currentFrameTime = 0.0f;

	// Set initial frames to start frame
	currRow = startFrame / cols;
	currCol = startFrame % cols;
}

/**
 * @brief Dtor
 */
Spritesheet::~Spritesheet()
{
	Unload();
}

/**
 * @brief Free memory!
 */
void Spritesheet::Unload()
{
	if (textureID)
	{
		glDeleteTextures(1, &textureID); // Delete texture and remove texture ID
		textureID = 0;
	}
}

/**
 * @brief Getter/ Setter for rows
 */
int Spritesheet::GetRows()
{
	return rows;
}
void Spritesheet::SetRows(int val)
{
	if (val < 0 || val >= rows) // Invalid range
	{
		_CrtDbgBreak();
	}
	currRow = val;
}

/**
 * @brief Getter/ Setter for Cols
 */
int Spritesheet::GetCols()
{
	return cols;
}
void Spritesheet::SetCols(int val)
{
	if (val < 0 || val >= cols) // Invalid range
	{
		_CrtDbgBreak();
	}
	currCol = val;
}

// startFrameIdx - index of first frame of spritesheet
// endFrameIdx	 - index of last frame of spritesheet
// duration		 - length of entire anim (start frame to end frame)
// eg: ~player_sprite.Loop(0, 7, .8f);~ means go from frame 0 to 7 within 0.8s
void Spritesheet::LoopSpriteSheetAnim(int, int, float)
{
}
