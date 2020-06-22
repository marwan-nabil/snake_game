#pragma once
#include "basic_defs.h"
#include "renderer.h"

#define BLOCK_SIZE_PIXELS 25.0f

f64 GameCoordsToPixelCoords(i32 GameCoords);

u32 GameCoordsToPixelCoordsU32(i32 GameCoords);

void DrawBlock(color Color, i32 X, i32 Y);

void DrawRectangle(color Color, i32 X, i32 Y, i32 Width, i32 Height);
