#include <list>
#include <vector>
#include "MineMap.h"
#include "Point.h"

char MoveToChar(Point pt1, Point pt2)
{
    Point diff;
    diff.x = pt1.x - pt2.x;
    diff.y = pt1.y - pt2.y;
    if ( (diff.x == 0) && (diff.y == 0) )
    {return 'W';}

    if (diff.y == 1)
    {
        return 'U';
    }

    if (diff.y == -1)
    {
        return 'D';
    }

    if (diff.x == 1)
    {
        return 'L';
    }

    if (diff.x == -1)
    {
        return 'R';
    }

    return 'A';//Error!!!!
}

Direction MoveToEnum(Point pt1, Point pt2)
{
	Point diff;
    diff.x = pt1.x - pt2.x;
    diff.y = pt1.y - pt2.y;
    if ( (diff.x == 0) && (diff.y == 0) )
    {
		return wait;
	}

    if (diff.y == 1)
    {
		return up;
    }

    if (diff.y == -1)
    {
        return down;
    }

    if (diff.x == 1)
    {
		return (Direction)2;
    }

    if (diff.x == -1)
    {
        return (Direction)3;
    }

    //return 'A';//Error!!!!
}

vector<char> PathToChar(Point start, list<Point>* lst)
{
    vector<char> res;

    list<Point>::const_iterator iter;

    Point first = start;
    Point second;

    //res.push_back( MoveToChar(first, second) );

    for ( iter = lst->begin(); iter != lst->end(); iter++)
    {
        second = (*iter);
        res.push_back( MoveToChar(first, second) );
        first = second;
    }

    return res;
}

void printvect(vector<char> vect)
{
        cout<<'\n';
        for (int i = 0; i< vect.size(); i++)
        {
            cout<<vect[i];
        }
        cout<<'\n';

}
