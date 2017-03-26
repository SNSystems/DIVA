/* example_04.cpp */
#include <iostream>
using namespace std;

class Point {
  int x, y;
public:
  Point(int i, int j) { x = i; y = j; }
  int getX() { return x; }
  int getY() { return y; }
};

void foo()
{
  Point p1(10, 20);
  Point p2 = p1;
  cout << "x = " << p2.getX() << " y = " << p2.getY();
}
