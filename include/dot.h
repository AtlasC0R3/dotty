#ifndef DOT_H
#define DOT_H
class Dot
{
  private:
	double dot_x;
  double dot_y;
  int width;
  int height;
	
  public:
	Dot();
	double getX();
	double getY();
  int    getWidth();
  int    getHeight();
  void   updatePosition(const int screenWidth, const int screenHeight);
  void   remove();

};
#endif