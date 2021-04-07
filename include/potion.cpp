#include "raylib.h"
#include "potion.h"
#include <math.h>
#include <cstdio>
Potion::Potion(int type)
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

void Potion::update()
{
    if (available){
        x = rand() % 700;
        y = rand() % 350;
        type = 1;
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
