#include "crtdbg.h"		// For debugging
#include "core.h"		// For DT
#include "GraphicsSystem.h"
#include "RenderComponent.h"
#include "stb_image.h"
#include "gameObject.h"
#define STB_IMAGE_IMPLEMENTATION

// Class that handles spritesheet Animations
class Spritesheet {
public:
	Spritesheet();
	Spritesheet(const char* textureFile, int numRows, int numCols);
	Spritesheet(Spritesheet const& rhs);
	~Spritesheet();

	// function to free textures and meshes (might not use dtor)
	void Unload();

	// Setter Getters for row n cols
	int GetRows();
	void SetRows(int val);

	int GetCols();
	void SetCols(int val);

	// function to loop anim if reach last frame
	void LoopSpriteSheetAnim(int startFrameIdx, int endFrameIdx, float time);

	// function to draw anim
	void DrawSpritesheetAnim(GameObject* GO , float width, float height, bool flip);
	
private:
	const int rows, cols;					// Amount of rows and calls, ideally anims will be separated by rows
	int currRow, currCol;					// Current row and col (specific sprite)
	const float uv_width, uv_height;				// Width and Height for textures
	//float timeBtwnFrames;
	float loop;								// Value to check against for loop timer
	float alpha;							// Transparency!

	// incl texture here
	const GLuint textureID;

	// include unit square mesh to apply transforms here
};