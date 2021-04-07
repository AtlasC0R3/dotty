#ifndef POTION_H
#define POTION_H
class Potion
{
  private:
	  double x;
    double y;
    int width;
    int height;
    int type;
    bool available;
	
  public:
	Potion(int type);
	double getX();
	double getY();
  int    getWidth();
  int    getHeight();
  int    getType();
  void   update();
  void   remove();
  void   setAvailable(bool availability);

};
#endif