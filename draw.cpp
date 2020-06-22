#include "basic_defs.h"
#include "draw.h"

f64 GameCoordsToPixelCoords(i32 GameCoords)
{
    f64 Result = (f64) GameCoords * BLOCK_SIZE_PIXELS;
    return Result;
}

u32 GameCoordsToPixelCoordsU32(i32 GameCoords)
{
    return (u32) GameCoordsToPixelCoords(GameCoords);
}

void DrawBlock(color Color, i32 X, i32 Y)
{
    f64 PixelX = GameCoordsToPixelCoords(X);
    f64 PixelY = GameCoordsToPixelCoords(Y);

    RenderRectangle(Color, PixelX, PixelY, BLOCK_SIZE_PIXELS, BLOCK_SIZE_PIXELS);
}

void DrawRectangle(color Color, i32 X, i32 Y, i32 Width, i32 Height)
{
    f64 PixelX = GameCoordsToPixelCoords(X);
    f64 PixelY = GameCoordsToPixelCoords(Y);

    RenderRectangle(Color, PixelX, PixelY, 
                    (BLOCK_SIZE_PIXELS * (f64) Width), 
                    (BLOCK_SIZE_PIXELS * (f64) Height));
}