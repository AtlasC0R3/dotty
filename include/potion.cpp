#include "raylib.h"
#include "potion.h"
#include <math.h>
#include <cstdio>
#include <stdlib.h>
#include <stdio.h>
Potion::Potion()
{
    width = 64;
    height = 64;
    x = rand()% 660 + 20;
    y = 0 - height;
    available = true;
}

double Potion::getX()
{
    return x;
}

double Potion::getY()
{
    return y;
}

int Potion::getWidth()
{
    return width;
}

int Potion::getHeight()
{
    return height;
}

void Potion::randomize_position(const int screenWidth, const int screenHeight)
{
    while (true){
        x = rand() % (screenWidth - 96);
        y = rand() % (screenHeight - 96);
        if (x >= 96 and y >= 96) break;
    }
}

void Potion::update(const int screenWidth, const int screenHeight)
{
    if (available and x < 0){
        randomize_position(screenWidth, screenHeight);
    } else{
        remove();
    }
}

void Potion::remove()
{
    y = -500;
    x = -500;
}

void Potion::setAvailable(bool availability)
{
    available = availability;
}
