#pragma once
#include <list>
#include "basic_defs.h"

#define BLOCK_SIZE_PIXELS 25.0f
#define MOVING_PERIOD 0.2
#define RESTART_TIME 1.0

struct color {
    u8 Red, Green, Blue;
};

struct block {
    i32 X, Y;
};

enum key {
    KeyUp, KeyDown, KeyLeft, KeyRight, KeyEscape
};

enum direction {
    Up, Down, Left, Right, NoChange
};

struct snake {
    direction CurrentDirection;
    std::list<block> Body;
    block Tail;
};

struct snake_game {
    snake Snake;
    bool FoodExists;
    i32 FoodX, FoodY;
    i32 Width, Height;
    bool GameOver;
    f64 WaitingTime;
};


snake NewSnake(i32 X, i32 Y);

void DrawSnake(snake *Snake);

void GetSnakeHeadPosition(snake *Snake, i32 *X, i32 *Y);

direction GetSnakeHeadDirection(snake *Snake);

void MoveSnakeForward(snake *Snake, direction Direction);

void GetNextSnakeHeadPosition(snake *Snake, direction Direction, i32 *X, i32 *Y);

void RestoreSnakeTail(snake *Snake);

bool IsTailOverlapping(snake *Snake, i32 X, i32 Y);
direction GetOppositeDirection(direction Dir);

snake_game NewGame(i32 Width, i32 Height);

void KeyPressHandler(snake_game *Game, key Key);

void UpdateSnake(snake_game *Game, direction Direction);

void DrawGame(snake_game *Game);

void UpdateGame(snake_game *Game, f64 TimeDelta);

void CheckIfSnakeEating(snake_game *Game);

bool CheckIfSnakeAlive(snake_game *Game, direction Direction);

void AddFood(snake_game *Game);

void RestartGame(snake_game *Game);

f64 GameCoordsToPixelCoords(i32 GameCoords);

u32 GameCoordsToPixelCoordsU32(i32 GameCoords);

void DrawBlock(color Color, i32 X, i32 Y);

void DrawRectangle(color Color, i32 X, i32 Y, i32 Width, i32 Height);