#ifndef POINT_H
#define POINT_H

enum Direction {up = 0, down = 1, left = 2, right = 3, wait = 4, s = 5};

struct Point 
{
   int x, y;
   Point()
   {
       x = 0;
       y = 0;
   }
   Point (int X, int Y)
   {
       x=X;
       y=Y;
   }

   Point operator+(const Direction &direction)
   {
	   Point result(x, y);
	   switch(direction)
	   {
	   case up:
		   {
			   result.y -= 1;
			   return result;
		   }
	   case down:
		   {
			   result.y += 1;
			   return result;
		   }
	   case left:
		   {
			   result.x -= 1;
			   return result;
		   }
	   case right:
		   {
			   result.x += 1;
			   return result;
		   }
	   }
	   return result;
   }

   bool operator== (const Point &point)
   {
	   return ((point.x == x) && (point.y == y));
   }
};

#endif // POINT_H
