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
	Dot(int index);
	double getX();
	double getY();
  int    getWidth();
  int    getHeight();
  void   updatePosition();
  void   remove();

};
#endif