#include "raylib.h"
#include "dot.h"
#include <math.h>
#include <cstdio>
#include <stdlib.h>
#include <stdio.h>
Dot::Dot(int index)
{
    width = 64;
    height = 64;
    dot_x = rand()% 660 + 20;
    dot_y = 0 - height - (index * 100);
    
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

void Dot::updatePosition()
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

void Dot::remove()
{
    dot_y = -50;
    dot_x = -50;
}