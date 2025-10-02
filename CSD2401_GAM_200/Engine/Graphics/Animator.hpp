#pragma once
#include "crtdbg.h"	
#include "core.h"		
#include "GraphicsSystem.h"
#include "RenderComponent.h"
#include "gameObject.h"

// Class that handles spritesheet Animations
class Spritesheet : public IComponent{
public:
	Spritesheet(GameObject* owner ,const char* textureFile, int numRows, int numCols);
	Spritesheet(Spritesheet const& rhs,GameObject* newClone);
	~Spritesheet();

	std::string GetName() const override { return "Spritesheet"; }
	void Initialize() override {}
	void Update(float dt) override;
	void Destroy() override {}
	std::unique_ptr<IComponent> Clone(GameObject* NewClone) const override;
	
	GLuint GetTextureID() const { return textureID; }
	void LoadNewSpritesheet(const char* textureFile, int numRows, int numCols);
	void SetAnimation(int startFrameIdx, int endFrameIdx, float fps);
	void Unload();

	glm::vec2 GetUVScale() const;
	glm::vec2 GetUVOffset() const;


	int GetRows();
	void SetRows(int val);
	int GetCurrentStartFrame() const { return startFrame; }
	float GetTimePerFrame() const { return timePerFrame; }


	int GetCols();
	void SetCols(int val);

	// function to loop anim if reach last frame
	void LoopSpriteSheetAnim(int startFrameIdx, int endFrameIdx, float time);


private:
	int rows, cols;					
	int currRow, currCol;					
	float uv_width, uv_height;				
	float timePerFrame;        
	float currentFrameTime;     
	int startFrame, endFrame;  

	const char* SpriteFile;
	 GLuint textureID;
	float alpha;

};
