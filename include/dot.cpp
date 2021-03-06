#include "dot.h"
#include <math.h>
#include <cstdio>
#include <stdlib.h>
#include <stdio.h>
Dot::Dot()
{
    width = 64;
    height = 64;
    dot_x = rand()% 660 + 20;
    dot_y = 0 - height;
    
}

double Dot::getX()
{
    return dot_x;
}

double Dot::getY()
{
    return dot_y;
}

int Dot::getWidth()
{
    return width;
}

int Dot::getHeight()
{
    return height;
}

void Dot::updatePosition(const int screenWidth, const int screenHeight)
{
    int old_dot_x = dot_x;
    int old_dot_y = dot_y;

    while (true){
        dot_x = rand() % (screenWidth - 96);
        dot_y = rand() % (screenHeight - 96);

        int sum_x = old_dot_x - dot_x;
        int sum_y = old_dot_y - dot_y;
        if (sum_x < 0) sum_x*=-1;
        if (sum_y < 0) sum_y*=-1;
        if (not(sum_x <= 75 or sum_y <= 75) and (dot_x >= 96 and dot_y >= 96)) break;
    }
}

void Dot::remove()
{
    dot_y = -50;
    dot_x = -50;
}