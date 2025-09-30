#pragma once
#include "crtdbg.h"		// For debugging
#include "core.h"		// For DT
#include "GraphicsSystem.h"
#include "RenderComponent.h"
#include "gameObject.h"

// Class that handles spritesheet Animations
class Spritesheet : public IComponent{
public:
	Spritesheet(GameObject* owner);
	Spritesheet(GameObject* owner ,const char* textureFile, int numRows, int numCols);
	Spritesheet(Spritesheet const& rhs,GameObject* newClone);
	~Spritesheet();

	std::string GetName() const override { return "Spritesheet"; }
	void Initialize() override {}
	void Update(float dt) override;
	void Destroy() override {}
	std::unique_ptr<IComponent> Clone(GameObject* NewClone) const override;
	
	void SetAnimation(int startFrameIdx, int endFrameIdx, float fps);
	void Unload();

	glm::vec2 GetUVScale() const;
	glm::vec2 GetUVOffset() const;

	// Setter Getters for row n cols
	int GetRows();
	void SetRows(int val);

	int GetCols();
	void SetCols(int val);

	// function to loop anim if reach last frame
	void LoopSpriteSheetAnim(int startFrameIdx, int endFrameIdx, float time);


private:
	const int rows, cols;					// Amount of rows and calls, ideally anims will be separated by rows
	int currRow, currCol;					// Current row and col (specific sprite)
	const float uv_width, uv_height;				// Width and Height for textures
	float timePerFrame;         // Time for one frame
	float currentFrameTime;     // Time accumulated since last frame change
	int startFrame, endFrame;   // The frame range for the current animation
	// incl texture here
	const char* SpriteFile;
	const GLuint textureID;
	float alpha;

	// include unit square mesh to apply transforms here
};
