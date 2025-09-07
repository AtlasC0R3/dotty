#include "player.h"
#include <math.h>
#include <vector>

Player::Player(float x, float y, int width, int height)
{
	this->x = x;
	this->y = y;
    this->width = width;
	this->height = height;
}

float Player::getX()
{
	return x;
}

float Player::getY()
{
	return y;
}

void Player::setX(float x)
{
    this->x = x;
}

void Player::setY(float y)
{
    this->y = y;
}

int Player::getWidth()
{
    return width;
}

int Player::getHeight()
{
    return height;
}
float Player::getRelativeX(){
    return getX() + getWidth() / 3;
}
float Player::getRelativeY(){
    return getY() + getHeight() / 3;
}
