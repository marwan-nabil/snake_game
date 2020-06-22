#include "basic_defs.h"
#include <cstdlib>
#include "snake.h"
#include "renderer.h"
#include "draw.h"

static color SnakeColor = {0, 240, 0};


snake NewSnake(i32 X, i32 Y)
{
    snake Result;
    Result.Tail = {};
    Result.CurrentDirection = Right;
    Result.Body = {};
    Result.Body.push_back({X + 2, Y});
    Result.Body.push_back({X + 1, Y});
    Result.Body.push_back({X , Y});
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