/* Start Header ************************************************************************/
/*!
\file       Animator.hpp
\author     Muhammad Ashfer [100% - 40 lines]
\par        m.binafzalkhan@digipen.edu
\date       October, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "crtdbg.h"
#include "core.h"
#include "GraphicsSystem.h"
#include "RenderComponent.h"
#include "gameObject.h"

/**
 * @brief Spritesheet class, derived from Components class, to get GO/ Comp IDs etc. But should not be viewed as a component (We can build upon this in the future)
 */
class Spritesheet : public IComponent
{
public:
	/**
	 * @brief Ctor, with GO ptr, and parameters specifying spritesheet details.
	 * @param textureFile Filepath for spritesheet
	 * @param intRows     Number of rows in spritesheet
	 * @param lintCols	 Number of cols in spritesheet
	 */
	Spritesheet(GameObject *owner, const char *textureFile, int numRows, int numCols);

	/**
	 * @brief Ctor, with GO ptr, and parameters specifying spritesheet details.
	 * @param rhs		  reference to spritesheet
	 * @param newClone     Ptr to Game Object
	 */
	Spritesheet(Spritesheet const &rhs, GameObject *newClone);

	/**
	 * @brief Dtor. Will properly destruct the spritesheet.
	 */
	~Spritesheet();

	/**
	 * @brief Overloaded virtual functions, since derived from IComponents. (TBD if spritesheet will stay as a component in the future)
	 */
	std::string GetName() const override { return "Spritesheet"; }
	void Initialize() override {}
	void Update(float dt) override;
	void Destroy() override {}
	std::unique_ptr<IComponent> Clone(GameObject *NewClone) const override;

	/**
	 * @brief Return texture ID
	 */
	GLuint GetTextureID() const { return textureID; }

	/**
	 * @brief Loads a new spritesheet file within the same object.
	 * @param textureFile Filepath for spritesheet
	 * @param intRows     Number of rows in spritesheet
	 * @param lintCols	 Number of cols in spritesheet
	 */
	void LoadNewSpritesheet(const char *textureFile, int numRows, int numCols);

	/**
	 * @brief Sets the animation of the object, within the same sprite sheet
	 * @param startFrameIdx     Starting frame of animation
	 * @param endFrameIdx	   End frame
	 * @param fps		       Time per frame
	 */
	void SetAnimation(int startFrameIdx, int endFrameIdx, float fps);

	/**
	 * @brief Frees up assigned data/mem for destructor (and maybe reloading if error occurs)
	 */
	void Unload();

	/**
	 * @brief Graphics related, how the spritesheet frame should stick to a 'mesh' (how to 'cut' the image from a spritesheet)
	 */
	vec2 GetUVScale() const;
	vec2 GetUVOffset() const;

	/**
	 * @brief Getters/ Setters for Rows and Cols
	 */
	int GetRows();
	void SetRows(int val);
	int GetCols();
	void SetCols(int val);

	/**
	 * @brief Getter for startFrame and amount of time per frame (frame in this context refers to spritesheet frames, not game fps)
	 */
	int GetCurrentStartFrame() const { return startFrame; }
	float GetTimePerFrame() const { return timePerFrame; }

	// function to loop anim if reach last frame
	void LoopSpriteSheetAnim(int startFrameIdx, int endFrameIdx, float time);

private:
	/**
	 * @brief Private members to control the spritesheet and which frames are being displayed!
	 */
	int rows, cols;			   // total rows/cols
	int currRow, currCol;	   // current row/col timer is on, WRT timePerFrame
	float uv_width, uv_height; // Width/ Height of UV (for texture)
	float timePerFrame;		   // Timer between swapping to next frame
	float currentFrameTime;	   // Current timer
	int startFrame, endFrame;  // Starting and Ending frame of specific animation

	const char *SpriteFile; // File path for spritesheet
	GLuint textureID;		// Personalised ID for the texture
	float alpha;			// Transparency
};
