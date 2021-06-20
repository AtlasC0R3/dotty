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
	Potion();
	double getX();
	double getY();
  int    getWidth();
  int    getHeight();
  int    getType();
  void   update(const int screenWidth, const int screenHeight);
  void   remove();
  void   setAvailable(bool availability);
  void   randomize_position(const int screenWidth, const int screenHeight);

};
#endif