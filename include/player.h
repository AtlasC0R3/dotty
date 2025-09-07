#ifndef PLAYER_H
#define PLAYER_H
#include <vector>
class Player
{
  private:
	  float x;
    float y;
    int width;
    int height;
	
  public:
	Player(float x, float y, int width, int height);
	float getX();
	float getY();
  float getRelativeX();
	float getRelativeY();
    void setX(float x);
    void setY(float y);
    int getWidth();
    int getHeight();

};
#endif