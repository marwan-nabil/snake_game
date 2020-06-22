#pragma once
#include "basic_defs.h"
#include <list>

enum direction {
    Up, Down, Left, Right, NoChange
};

struct block {
    i32 X, Y;
};

struct snake {
    direction CurrentDirection;
    std::list<block> Body;
    block Tail;
};

snake NewSnake(i32 X, i32 Y);

void DrawSnake(snake *Snake);

void GetSnakeHeadPosition(snake *Snake, i32 *X, i32 *Y);

direction GetSnakeHeadDirection(snake *Snake);

void MoveSnakeForward(snake *Snake, direction Direction);

void GetNextSnakeHeadPosition(snake *Snake, direction Direction, i32 *X, i32 *Y);

void RestoreSnakeTail(snake *Snake);

bool IsTailOverlapping(snake *Snake, i32 X, i32 Y);
