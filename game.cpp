#include <Windows.h>
#include <cstdlib>
#include <list>

#include "basic_defs.h"
#include "game.h"
#include "platform.h"


static color FoodColor = {200, 0, 0};
static color BorderColor = {0, 0, 0};
static color GameoverColor = {120, 0, 0};
static color SnakeColor = {0, 240, 0};


direction GetOppositeDirection(direction Dir)
{
	switch(Dir)
	{
        case Up:
            return Down;
            break;
        case Down:
            return Up;
            break;
        case Left:
            return Right;
            break;
        case Right:
            return Left;
            break;
        default:
            Assert(0);
            break;
	}
}

snake_game NewGame(i32 Width, i32 Height)
{
    snake_game Result = {};
    Result.Snake = NewSnake(2, 2);
    Result.WaitingTime = 0.0f;
    Result.FoodExists = true;
    Result.FoodX = 6;
    Result.FoodY = 4;
    Result.Width = Width;
    Result.Height = Height;
    Result.GameOver = false;
    return Result;
}

void KeyPressHandler(snake_game *Game, key Key)
{
    if(Game->GameOver)
    {
        return;
    }

    direction NextDirection = NoChange;
    switch(Key)
    {
        case KeyUp:
            NextDirection = Up;
            break;
        case KeyDown:
            NextDirection = Down;
            break;
        case KeyLeft:
            NextDirection = Left;
            break;
        case KeyRight:
            NextDirection = Right;
            break;
        default:
            break;
    }

    direction SnakeHeadCurrentDirection = GetSnakeHeadDirection(&Game->Snake);
    if(NextDirection == GetOppositeDirection(SnakeHeadCurrentDirection))
    {
        return;
    }
    else
    {
        UpdateSnake(Game, NextDirection);
    }
}

void UpdateSnake(snake_game *Game, direction Direction)
{
    if(CheckIfSnakeAlive(Game, Direction))
    {
        MoveSnakeForward(&Game->Snake, Direction);
        CheckIfSnakeEating(Game);
    }
    else
    {
        Game->GameOver = true;
    }
    Game->WaitingTime = 0.0f;
}

void DrawGame(snake_game *Game)
{
    DrawSnake(&Game->Snake);

    if(Game->FoodExists)
    {
        DrawBlock(FoodColor, Game->FoodX, Game->FoodY);
    }

    DrawRectangle(BorderColor, 0, 0, Game->Width, 1);
    DrawRectangle(BorderColor, 0, 0, 1, Game->Height);
    DrawRectangle(BorderColor, 0, Game->Height - 1, Game->Width, 1);
    DrawRectangle(BorderColor, Game->Width - 1, 0, 1, Game->Height);

    if(Game->GameOver)
    {
        DrawRectangle(GameoverColor, 0, 0, Game->Width, Game->Height);
    }
}

void UpdateGame(snake_game *Game, f64 TimeDelta)
{
    Game->WaitingTime += TimeDelta;
    if(Game->GameOver)
    {
        if(Game->WaitingTime > RESTART_TIME)
        {
            RestartGame(Game);
        }
        return;
    }

    if(!Game->FoodExists)
    {
        AddFood(Game);
    }

    if(Game->WaitingTime > MOVING_PERIOD)
    {
        UpdateSnake(Game, NoChange);
    }
}

void CheckIfSnakeEating(snake_game *Game)
{
    i32 HeadX, HeadY;
    GetSnakeHeadPosition(&Game->Snake, &HeadX, &HeadY);
    if(Game->FoodExists && (Game->FoodX == HeadX) && (Game->FoodY == HeadY))
    {
        Game->FoodExists = false;
        RestoreSnakeTail(&Game->Snake);
    }
}

bool CheckIfSnakeAlive(snake_game *Game, direction Direction)
{
    i32 NextX, NextY;
    GetNextSnakeHeadPosition(&Game->Snake, Direction, &NextX, &NextY);
    if(IsTailOverlapping(&Game->Snake, NextX, NextY))
    {
        return false;
    }
    return (NextX > 0 && NextY > 0 && NextX < (Game->Width - 1) && NextY < (Game->Height - 1));
}

void AddFood(snake_game *Game)
{
    i32 NewX = (rand() % (Game->Width - 1)) + 1;
    i32 NewY = (rand() % (Game->Height - 1)) + 1;

    while(IsTailOverlapping(&Game->Snake, NewX, NewY))
    {
        NewX = (rand() % (Game->Width - 1)) + 1;
        NewY = (rand() % (Game->Height - 1)) + 1;
    }

    Game->FoodX = NewX;
    Game->FoodY = NewY;
    Game->FoodExists = true;
}

void RestartGame(snake_game *Game)
{
    Game->Snake = NewSnake(2, 2);
    Game->WaitingTime = 0.0f;
    Game->FoodExists = true;
    Game->FoodX = 6;
    Game->FoodY = 4;
    Game->GameOver = false;
}

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

snake NewSnake(i32 X, i32 Y)
{
    snake Result;
    Result.Tail = {};
    Result.CurrentDirection = Right;
    Result.Body = {};
    Result.Body.push_back({X + 2, Y});
    Result.Body.push_back({X + 1, Y});
    Result.Body.push_back({X, Y});
    return Result;
}

void DrawSnake(snake *Snake)
{
    for(block BLock : Snake->Body)
    {
        DrawBlock(SnakeColor, BLock.X, BLock.Y);
    }
}

void GetSnakeHeadPosition(snake *Snake, i32 *X, i32 *Y)
{
    block HeadBlock = Snake->Body.front();
    *X = HeadBlock.X;
    *Y = HeadBlock.Y;
}

direction GetSnakeHeadDirection(snake *Snake)
{
    return Snake->CurrentDirection;
}

void MoveSnakeForward(snake *Snake, direction Direction)
{
    if(Direction != NoChange)
    {
        Snake->CurrentDirection = Direction;
    }
    i32 LastX, LastY;
    GetSnakeHeadPosition(Snake, &LastX, &LastY);

    block NewBlock = {};
    switch(Snake->CurrentDirection)
    {
        case Up:
        {
            NewBlock = {LastX, LastY - 1};
        } break;
        case Down:
        {
            NewBlock = {LastX, LastY + 1};
        } break;
        case Left:
        {
            NewBlock = {LastX - 1, LastY};
        } break;
        case Right:
        {
            NewBlock = {LastX + 1, LastY};
        } break;
        default:
        {
            Assert(0);
        } break;
    }

    Snake->Body.push_front(NewBlock);
    block RemovedBlock = Snake->Body.back();
    Snake->Body.pop_back();
    Snake->Tail = RemovedBlock;
}

void GetNextSnakeHeadPosition(snake *Snake, direction Direction, i32 *X, i32 *Y)
{
    i32 HeadX, HeadY;
    GetSnakeHeadPosition(Snake, &HeadX, &HeadY);
    direction MovingDirection = Snake->CurrentDirection;
    if(Direction != NoChange)
    {
        MovingDirection = Direction;
    }

    switch(MovingDirection)
    {
        case Up:
        {
            *X = HeadX;
            *Y = HeadY - 1;
        } break;
        case Down:
        {
            *X = HeadX;
            *Y = HeadY + 1;
        } break;
        case Left:
        {
            *X = HeadX - 1;
            *Y = HeadY;
        } break;
        case Right:
        {
            *X = HeadX + 1;
            *Y = HeadY;
        } break;
        default:
            Assert(0);
            break;
    }
}

void RestoreSnakeTail(snake *Snake)
{
    Snake->Body.push_back(Snake->Tail);
}

bool IsTailOverlapping(snake *Snake, i32 X, i32 Y)
{
    u64 ch = 0;
    for(block Block : Snake->Body)
    {
        if(X == Block.X && Y == Block.Y)
        {
            return true;
        }

        ch += 1;
        if(ch == Snake->Body.size() - 1)
        {
            break;
        }
    }
    return false;
}