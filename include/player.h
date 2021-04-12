#ifndef PLAYER_H
#define PLAYER_H
#include <vector>
class Player
{
  private:
	double x;
    double y;
    int width;
    int height;
	
  public:
	Player(double x, double y, int width, int height);
	double getX();
	double getY();
  double getRelativeX();
	double getRelativeY();
    void setX(int x);
    void setY(int y);
    int getWidth();
    int getHeight();

};
#endif