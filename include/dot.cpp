#include "raylib.h"
#include "dot.h"
#include <math.h>
#include <cstdio>
const int screenWidth = 800;
const int screenHeight = 450;
Platform::Platform(int index)
{
    width = 64;
    height = 64;
    dot_x = rand()% 660 + 20;
    dot_y = 0 - height - (index * 100);
    int coinInt = rand()% 4;
    if (coinInt == 0 || index == 0)
    {
        hasCoin = false;
    } else {
        hasCoin = true;
    }
    coinX = dot_x + width/2 - 24/2;
    coinY = dot_y - 24 - 5;
    
}

double Platform::getX()
{
    return dot_x;
}

double Platform::getY()
{
    return dot_y;
}

int Platform::getWidth()
{
    return width;
}

int Platform::getHeight()
{
    return height;
}

bool Platform::getHasCoin()
{
    return hasCoin;
}
void Platform::setHasCoin(bool value)
{
    hasCoin = value;
}
int Platform::getCoinX()
{
    return coinX;
}
int Platform::getCoinY()
{
    return coinY;
}

void Platform::updatePosition()
{
    int old_dot_x = dot_x;
    int old_dot_y = dot_y;

    while (true){
        dot_x = rand() % 700;
        dot_y = rand() % 350;

        int sum_x = old_dot_x - dot_x;
        int sum_y = old_dot_y - dot_y;
        if (sum_x < 0) sum_x*=-1;
        if (sum_y < 0) sum_y*=-1;
        if (not(sum_x <= 75 or sum_y <= 75)) break;
    }
}

void Platform::remove()
{
    dot_y = -50;
    dot_x = -50;
}