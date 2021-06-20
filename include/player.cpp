#include "player.h"
#include <math.h>
#include <vector>

Player::Player(double x, double y, int width, int height)
{
	this->x = x;
	this->y = y;
    this->width = width;
	this->height = height;
}

double Player::getX()
{
	return x;
}

double Player::getY()
{
	return y;
}

void Player::setX(int x)
{
    this->x = x;
}

void Player::setY(int y)
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
double Player::getRelativeX(){
    return getX() + getWidth() / 3;
}
double Player::getRelativeY(){
    return getY() + getHeight() / 3;
}
