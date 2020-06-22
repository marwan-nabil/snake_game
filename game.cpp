#include <cstdlib>
#include "basic_defs.h"
#include "renderer.h"
#include "game.h"
#include "draw.h"

color FoodColor = {200, 0, 0};
color BorderColor = {0, 0, 0};
color GameoverColor = {120, 0, 0};


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