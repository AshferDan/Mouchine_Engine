#include "Animator.hpp"

Spritesheet::Spritesheet() : rows{}, cols{}, currRow{}, currCol{}, uv_width{}, uv_height{}, loop{ 0.f }, textureID{}, alpha{ 1 }
{

}

Spritesheet::Spritesheet(const char* textureFile, int numRows, int numCols): 
	rows {numRows}, cols{numCols}, currRow{0}, currCol{0}, uv_width{1.f/rows}, uv_height{1.f/cols}, loop{ 0.f }, textureID{GraphicsSystem::LoadTexture(textureFile)}, alpha{1}
{
	// Load texture
	// Create mesh based off uv size

	// from AE
	//texture = AEGfxTextureLoad(texture_file);

	//AEGfxMeshStart();										// Make the mesh based on UV size

	//AEGfxTriAdd(
	//	-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, uv_height,				// bottom left
	//	0.5f, -0.5f, 0xFFFFFFFF, uv_width, uv_height,			// bottom right 
	//	-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);					// top left

	//AEGfxTriAdd(
	//	0.5f, -0.5f, 0xFFFFFFFF, uv_width, uv_height,			// bottom right 
	//	0.5f, 0.5f, 0xFFFFFFFF, uv_width, 0.0f,					// top right
	//	-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);					// bottom left

	//mesh = AEGfxMeshEnd();									// End mesh
}

Spritesheet::Spritesheet(Spritesheet const& rhs) : 
	rows{rhs.rows}, cols{rhs.cols}, uv_width{rhs.uv_width}, uv_height{rhs.uv_height}, textureID{rhs.textureID}, alpha{rhs.alpha}
{
	currRow = rhs.currRow;
	currCol = rhs.currCol;
	loop = rhs.loop;
}

Spritesheet::~Spritesheet() 
{

}

void Spritesheet::Unload()
{
	// if (texture) free texture
	//if (textureID) 
	//{
	//	
	//}

	// if (mesh) free mesh
}

int Spritesheet::GetRows() 
{
	return currRow;
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
	return currCol;
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
	if (startFrameIdx < 0 || endFrameIdx >= rows)					// Outside spritesheet range!
	{
		_CrtDbgBreak();
	}

	int rowIdx = endFrameIdx + 1 - startFrameIdx;
	// get dt here
	loop += static_cast<float>(CoreEngine::GetDeltaTime()) / duration;
	if (loop > 1.f)
	{
		loop = 0;
	}
	// Sets col to be of correct frame WRT loop values
	SetCols(static_cast<int>(rows * loop) + startFrameIdx);
}

void Spritesheet::DrawSpritesheetAnim(GameObject* GO, float width, float height, bool flip)
{
	if (flip)
	{
		width = -width;
	}
	// set render mode
	// set alpha value
	// draw image w specific row n col
	// draw image at gameobject position
	
	// either
	// (harder?) we attach texture to a unit square mesh, so we can distort it for extra visual effects, and just attach the mesh to GO position (dynamic!)
	// or 
	// (easier) just draw texture only -> no scaling/ distorting of mesh separate from game object
	
	// draw texture!


	//AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

	//AEGfxSetColorToMultiply(1.f, 1.f, 1.f, 1.f);
	//AEGfxSetColorToAdd(0.f, 0.f, 0.f, 0.f);
	//AEGfxSetTransparency(1.0f);
	//AEGfxSetTransform(Transform.m);

	//AEGfxTextureSet(texture, uv_width * curr_col, uv_height * curr_row);

	//AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
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