#pragma once

#include <iostream>
#include "MineMap.h"
#include "Point.h"

#include <math.h>
#include <queue>
#include <list>


#include "AStarSearch.h"

#include "MineMap.h"

const int threshold = 2;

float getdistMan(Point a, Point b)
{
	return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
}

bool isTrampolineOrTarget(char c)
{
	bool isTrampoline = false;
	bool isTarget = false;

	if (c > 'A' && c < 'z')
		isTrampoline = true;
	if (c > '1' && c < '9')
		isTarget = true;

	return isTarget || isTrampoline;
}

bool isRockFallingHere(char** m, int w, int h, Point curPoint)
{
	int width = w;
	int height = h;

	Point up = Point(curPoint.x, (curPoint.y - 1 >= 0) ? curPoint.y : -1);
	Point upper = Point(curPoint.x, (curPoint.y - 2 >= 0) ? curPoint.y : -1);
	Point upLeft = Point((curPoint.x - 1 >= 0) ? curPoint.x - 1 : -1,
			(curPoint.y - 1 >= 0) ? curPoint.y - 1 : -1);
	Point upperLeft = Point((curPoint.x - 1 >= 0) ? curPoint.x - 1 : -1,
			(curPoint.y - 2 >= 0) ? curPoint.y - 2 : -1);
	Point upRight = Point((curPoint.x + 1 < width) ? curPoint.x + 1 : -1,
			(curPoint.y - 1 >= 0) ? curPoint.y - 1 : -1);
	Point upperRight = Point((curPoint.x + 1 < width) ? curPoint.x + 1 : -1,
			(curPoint.y - 2 >= 0) ? curPoint.y - 2 : -1);

	if (upper.y != -1)
	{
		if ((m[upper.y][upper.x] == '*' || m[upper.y][upper.x] == '@')
			&& m[up.y][up.x] == ' ')
			return false;

		if ((m[upper.y][upper.x] == '*' || m[upper.y][upper.x] == '@')
			&& m[up.y][up.x] == 'R')
			return false;

		if ((m[upperLeft.y][upperLeft.x] == '*' || m[upperLeft.y][upperLeft.x] == '@')
				&& (m[upLeft.y][upLeft.x] == '*' || m[upLeft.y][upLeft.x] == '#'
						|| m[upLeft.y][upLeft.x] == 'W' || m[upLeft.y][upLeft.x] == '@'
								|| m[upLeft.y][upLeft.x] == '\\'
										|| isTrampolineOrTarget(m[upLeft.y][upLeft.x])))
			return false;

		if ((m[upperRight.y][upperRight.x] == '*' || m[upperRight.y][upperRight.x] == '@')
				&& (m[upLeft.y][upLeft.x] == '*' || m[upLeft.y][upLeft.x] == '#'
						|| m[upLeft.y][upLeft.x] == 'W' || m[upLeft.y][upLeft.x] == '@'
								|| m[upLeft.y][upLeft.x] == '\\'
										|| isTrampolineOrTarget(m[upRight.y][upRight.x])))
			return false;
	}

	return true;
}

vector< vector<Point> > GetCluster(vector<Point> lam)
{
	vector<int> clstrs;
	vector< vector<Point> > res;

	for (unsigned int i = 0; i < lam.size(); i++)
	{
		clstrs.push_back(-1);
	}


	for (unsigned int i = 0; i < lam.size(); i++)
	{
		if (clstrs[i] == -1)
		{
			vector<Point> buf;
			queue<int> tmp;
			tmp.push(i);
			while (!tmp.empty())
			{
				int pnt = tmp.front();
				buf.push_back(lam[pnt]);
				tmp.pop();
				//cout<<"!"<<pnt;
				clstrs[pnt] = 1;
				//queue<int> tmp2 = GetNBH(lam, clstrs, pnt);

                for (unsigned int i2 = 0; i2 < lam.size(); i2++)
                {
                    if ((i2 != pnt) && (clstrs[i2] == -1) && (getdistMan(lam[i2], lam[pnt]) <= threshold))
                    {
                        tmp.push(i2);
                        clstrs[i2] = 1;
                    }
                }

			}
			res.push_back(buf);

		}
	}

	return res;
}

int GetNearest(vector< vector<Point> > clasters, Point startpos, Point *minp, int minsize)
{

    if (clasters.size() == 0)
    {
        *minp = startpos;
        return -1;
    }

    *minp = clasters[0][0];
    int cnum = 0;
    float mindist = getdistMan(startpos, clasters[0][0]);

    for (unsigned int i = 0; i < clasters.size(); i++)
    {
        if (clasters[i].size()>=minsize)
        {

            for (unsigned int j = 0; j < clasters[i].size(); j++)
            {
                float dist = getdistMan(startpos, clasters[i][j]);
                if (dist < mindist)
                {
                    mindist = dist;
                    *minp = clasters[i][j];
                    cnum = i;
                }
            }
        }
    }

    return cnum;
}

int PathSize(MineMap &map, Point start, Point dest, list<Point>* route, AStarSearch &as)
{
        as.getRoute(&map, start, dest, *route, isRockFallingHere, "*#W@");

        if (route->size() > 0)
        {
            return route->size();
        }
        return -1;
}

vector<Point> Approachability(MineMap &map, vector<Point>& points, Point start, vector< list<Point> >* paths)
{
    AStarSearch as;

	vector<Point> res;


    for (int i = 0; i < points.size(); i++)
    {

        Point dest = points[i];//map.GetLift();
        //cout<<'h';
        list<Point>* route = new list<Point>();

        //as.getRoute(&map, start, dest, *route);

        paths->push_back(*route);

	    int size = PathSize(map, start, dest, route, as);//route->size();
	    if (size > 0)
	    {
	        res.push_back(points[i]);
        }

        delete route;
    }


	return res;
}

int GetNearestAA(MineMap& map, vector< vector<Point> > &clasters, Point startpos, Point *minp, int minsize)
{

    if (clasters.size() == 0)
    {
        *minp = startpos;
        return -1;
    }

    AStarSearch as;
	
    //*minp = clasters[0][0];
    int cnum = -1;
    list<Point>* route = new list<Point>();
    float mindist = 2147483647;
    //float mindist = PathSize(map, startpos, clasters[0][0], route, as);//getdistMan(startpos, clasters[0][0]);

    for (unsigned int i = 0; i < clasters.size(); i++)
    {
        if (clasters[i].size()>=minsize)
        {

            for (unsigned int j = 0; j < clasters[i].size(); j++)
            {
                list<Point>* route = new list<Point>();
                float dist = PathSize(map, startpos, clasters[i][j], route, as);//route->size();//getdistMan(startpos, clasters[i][j]);
                if ( (dist > 0) && (dist < mindist))
                {
                    mindist = dist;
                    *minp = clasters[i][j];
                    cnum = i;
                }
                delete route;
            }
        }
    }

    delete route;

    return cnum;
}

Point GetNextLam(MineMap &map, AStarSearch &as, Point start, vector<Point> &clstr, list<Point> *proute)
{

    Point res = start;

    if (clstr.size() == 0)
    {
        return start;
    }

    proute = new list<Point>();
    float mindist = 2147483647;//PathSize(map, start, clstr[0], proute, as);//getdistMan(startpos, clasters[0][0]);

        for (unsigned int j = 0; j < clstr.size(); j++)
        {
            list<Point>* route = new list<Point>();
            float dist = PathSize(map, start, clstr[j], route, as);//route->size();//getdistMan(startpos, clasters[i][j]);
            if ( (dist > 0) && (dist < mindist))
            {
                mindist = dist;
                res= clstr[j];
//                cnum = i;
                delete proute;
                proute = route;
            }
            else
                delete route;
        }


    return res;
}
