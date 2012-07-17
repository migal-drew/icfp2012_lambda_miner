#include <iostream>
#include "MineMap.h"
#include "Point.h"
#include "Claster.h"
#include "AStarSearch.h"
#include "transtochar.h"

#include <math.h>
#include <queue>
#include <fstream>
#include <algorithm>


using namespace std;

void MoveRobot(vector<char> *res, MineMap &map, list<Point>* route)
{
	res->push_back(MoveToChar(map.GetRobot(), *route->begin()));					//write step to answer
	//printvect(*res);
	if (map.MoveRobot(MoveToEnum(map.GetRobot(), *route->begin())) 
		|| map.UpdateMineMap())
	{
		//printf("Game over\n");
//		map.PrintMap();
		//return 0;
	}
	//bool m = map.MoveRobot(MoveToEnum(map.GetRobot(), *route->begin()));					//step to first point of route
	map.UpdateMineMap();
	//map.PrintMap(PSShort);
	//cout<<m<<endl;
	route->erase(route->begin());	
}

int main()
{
    MineMap map;
	//map.openFileForMap("C:\\Users\\Vlad\\Projects\\C++\\Lex200icfp\\Debug\\test.map");
    map.ReadMap();
    //map.ShortPrint();
    vector<Point> lam = map.GetLambdas();
    Point start = map.GetRobot();
    vector< list<Point> > paths;

    vector<char> res;

    vector< vector<Point> >  clstrs = GetCluster(lam);
    Point minp;
	//cout<<"We get index and route to nearest cluster "<<endl;
    int clastertomove = GetNearestAA(map, clstrs, start, &minp, 1);

    AStarSearch as;

	list<Point>* route = new list<Point>();
	bool claster_achieved = false;
    while (clastertomove != -1)
    {
		if(map.Waterproof() == 1)
		{
			res.push_back('A');
			printvect(res);
			cout<<'\n';
			return 0;
		}
		if(!claster_achieved)
		{
			route = new list<Point>();
			//MOVEROBOT here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			Point ppp = map.GetRobot();
			as.getRoute(&map, ppp, minp, *route, isSafeFromRocks, (char*)"*#W@");
			if(route->size() == 0)
			{
				clastertomove = GetNearestAA(map, clstrs, map.GetRobot(), &minp, 1);
				continue;
			}
		//	cout<<"//we get a route to the nearest point of the nearest claster"<<endl;
			do
			{
				MoveRobot(&res, map, route);
			}
			while((!map.isChanged()) && (route->size() != 0));
			if(map.isChanged())								//if map changed we get route to claster again
			{
				if(route->size() == 0)
					claster_achieved = true;
				continue;
			}
		}
		//cout<<"//when we achieved claster we achieved it's lambda"<<endl;
		if(find(clstrs[clastertomove].begin(), clstrs[clastertomove].end(), map.GetRobot()) != clstrs[clastertomove].end())
			clstrs[clastertomove].erase(find(clstrs[clastertomove].begin(), clstrs[clastertomove].end(), map.GetRobot()));
		list<Point> *route_to_nrp = new list<Point>();												//route to nearest point in claster
		//we running from lambda to lambda in claster
		Point p = GetNextLam(map, as, map.GetRobot(), clstrs[clastertomove], route_to_nrp);
		route_to_nrp->clear();												//fucking kostyl
		PathSize(map, map.GetRobot(), p, route_to_nrp, as);
		while(route_to_nrp->size())//!(map.GetRobot() == p))
		{
			do
			{
				//if point to move is lambda then remove it from cluster
				if(find(clstrs[clastertomove].begin(), clstrs[clastertomove].end(), *(route_to_nrp->begin())) != clstrs[clastertomove].end())
					clstrs[clastertomove].erase(find(clstrs[clastertomove].begin(), clstrs[clastertomove].end(), *route_to_nrp->begin()));
				MoveRobot(&res, map, route_to_nrp);
			}
			while((route_to_nrp->size() != 0) && (!map.isChanged()));
			//don't have any thoughts about action when map changes. Now robot continues gather lambdas in claster
			route_to_nrp->clear();
			Point p = GetNextLam(map, as, map.GetRobot(), clstrs[clastertomove], route_to_nrp);
			route_to_nrp->clear();
			PathSize(map, map.GetRobot(), p, route_to_nrp, as);
		}
		clstrs.erase(clstrs.begin() + clastertomove);
		clastertomove = GetNearestAA(map, clstrs, map.GetRobot(), &minp, 1);				//we get new claster to move
		claster_achieved = false;
        //cout<<"//and repeat our routine"<<endl;
    }
//	cout<<"//when all what we can do is done, we find way to exit"<<endl;
    minp = map.GetLift();
    route = new list<Point>();
	while(!(map.GetRobot() == map.GetLift()))
	{
		route = new list<Point>();
		Point ppp = map.GetRobot();
		as.getRoute(&map, ppp, minp, *route, isSafeFromRocks, (char*)"*#W@");
		
		if(route->size() == 0)
		{
	//		cout<<"//if lift is unaccessible we send A"<<endl;
			res.push_back('A');
			printvect( res );
			cout <<"\n";
			return 0;
		}
		//cout<<"//we are going to lift"<<endl;
		do
		{
			MoveRobot(&res, map, route);
			if(route->size() == 0)
			{
				printvect( res );
				cout <<"\n";
				return 0;
			}
		}
		while(!map.isChanged());
	}
    
    return 0;
}
