
#include "Animator.hpp"

Spritesheet::Spritesheet(GameObject* owner)
	: IComponent(owner),
	rows(1),
	cols(1),
	currRow(0),
	currCol(0),
	uv_width(1.0f),
	uv_height(1.0f),
	timePerFrame(0.0f),
	currentFrameTime(0.0f),
	startFrame(0),
	endFrame(0),
	textureID(0)
{
}
Spritesheet::Spritesheet(GameObject* owner, const char* textureFile, int numRows, int numCols)
	: IComponent(owner),
	rows(numRows),
	cols(numCols),
	currRow(0),
	currCol(0),
	uv_width(1.0f / (float)numCols), // Calculate 1/Cols
	uv_height(1.0f / (float)numRows), // Calculate 1/Rows
	timePerFrame(0.0f),
	currentFrameTime(0.0f),
	startFrame(0),
	endFrame(0),
	textureID(GraphicsSystem::LoadTexture(textureFile))
{
}
Spritesheet::Spritesheet(Spritesheet const& rhs, GameObject* NewClone)
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

std::unique_ptr<IComponent> Spritesheet::Clone(GameObject* NewClone) const
{
	return std::make_unique<Spritesheet>(*this, NewClone);
}

void Spritesheet::Update(float dt)
{
	if (startFrame > endFrame) return;

	currentFrameTime += dt;

	if (currentFrameTime >= timePerFrame) {
		currentFrameTime -= timePerFrame;

		int currentFrameIndex = currRow * cols + currCol;
		int nextFrameIndex = currentFrameIndex + 1;

		if (nextFrameIndex > endFrame) {
			nextFrameIndex = startFrame;
		}
		currRow = nextFrameIndex / cols;
		currCol = nextFrameIndex % cols;
	}
}

glm::vec2 Spritesheet::GetUVScale() const
{

	return glm::vec2(uv_width, uv_height);
}

glm::vec2 Spritesheet::GetUVOffset() const
{
	
	return glm::vec2(uv_width * currCol, uv_height * currRow);
}

void Spritesheet::LoadNewSpritesheet(const char* textureFile, int numRows, int numCols) {
	// 1. Unload the old texture to free memory (optional but good practice)
	if (textureID != 0) {
		
		Unload();
	}

	
	textureID = GraphicsSystem::LoadTexture(textureFile);

	
	rows = numRows;
	cols = numCols;
	uv_width = 1.0f / (float)numCols;
	uv_height = 1.0f / (float)numRows;

	// 4. Reset animation state
	currRow = 0;
	currCol = 0;
	startFrame = 0;
	endFrame = 0;


	if (auto* render = owner->GetComponent<RenderComponent>()) {
		render->SetTextureID(textureID); 
	}
}

void Spritesheet::SetAnimation(int startFrameIdx, int endFrameIdx, float fps) {
	startFrame = startFrameIdx;
	endFrame = endFrameIdx;
	timePerFrame = 1.0f / fps;
	currentFrameTime = 0.0f;

	// Set initial frame to startFrame
	currRow = startFrame / cols;
	currCol = startFrame % cols;
}

Spritesheet::~Spritesheet()
{

}

void Spritesheet::Unload()
{

}

int Spritesheet::GetRows()
{
	return rows;
}
void Spritesheet::SetRows(int val)
{
	if (val < 0 || val >= rows)
	{
		_CrtDbgBreak();										// Out of bounds, Breakpoints in visual studio
	}
	currRow = val;
}

int Spritesheet::GetCols()
{
	return cols;
}
void Spritesheet::SetCols(int val)
{
	if (val < 0 || val >= cols)
	{
		_CrtDbgBreak();										// Out of bounds, Breakpoints in visual studio
	}
	currCol = val;
}

// startFrameIdx - index of first frame of spritesheet
// endFrameIdx	 - index of last frame of spritesheet
// duration		 - length of entire anim (start frame to end frame)
//eg: ~player_sprite.Loop(0, 7, .8f);~ means go from frame 0 to 7 within 0.8s
void Spritesheet::LoopSpriteSheetAnim(int startFrameIdx, int endFrameIdx, float duration)
{
}



//// ----------------------------------------------------------------------------
//// ----------------------------------------------------------------------------
// example of how anim drawing will look like
// basically to draw anim, set row and col, and itll be good

// non looping
//{
//	switch (player.state)				// state of player, then just set row and set col
//	{
//	case MELEE:
//		if (Screen_To_World_x(Mouse.x) > player.pos.x)
//		{
//			player.direction_h = RIGHT;
//		}
//		else
//		{
//			player.direction_h = LEFT;
//		}
//		switch (melee_attack.combo)
//		{
//		case 1:
//
//			//new
//			player_sprite.Set_Row(6);
//			player_sprite.Set_Col(0, 9, melee_attack.active_time.current);
//			break;
//		case 2:
//
//			//new
//			player_sprite.Set_Row(7);
//			player_sprite.Set_Col(0, 9, melee_attack.active_time.current);
//			break;
//		case 3:
//
//			//new
//			player_sprite.Set_Row(8);
//			player_sprite.Set_Col(0, 19, melee_attack.active_time.current);
//			break;
//		default:
//			break;
//		}
//		break;	// end case MELEE
//	}
//}
//
//// looping
//	case MOVING:
//
//		player_sprite.Set_Row(1);
//		player_sprite.Loop(0, 7, .8f);
//		break;
//// ----------------------------------------------------------------------------
//
//switch (player.direction_h)
//{
//case LEFT:
//	player_sprite.Draw(World_To_Screen_x(player.pos.x), World_To_Screen_y(player.pos.y), PLAYER_DRAW_WIDTH, PLAYER_DRAW_HEIGHT, false);
//	break;
//case RIGHT:
//	player_sprite.Draw(World_To_Screen_x(player.pos.x), World_To_Screen_y(player.pos.y), PLAYER_DRAW_WIDTH, PLAYER_DRAW_HEIGHT, true);
//	break;
//default:
//	break;
//}
