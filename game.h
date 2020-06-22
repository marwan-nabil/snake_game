#pragma once
#include "basic_defs.h"
#include "snake.h"

#define MOVING_PERIOD 0.2
#define RESTART_TIME 1.0


struct snake_game {
    snake Snake;
    bool FoodExists;
    i32 FoodX, FoodY;
    i32 Width, Height;
    bool GameOver;
    f64 WaitingTime;
};

enum key {
    KeyUp, KeyDown, KeyLeft, KeyRight, KeyEscape
};

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
