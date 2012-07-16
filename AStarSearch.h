/*
* Class for searching optimal possible path between two points
*/
#pragma once

//#ifndef NULL
//#define NULL 0
//#endif

#include "Point.h"
#include "MineMap.h"
#include <list>
#include <stdlib.h>

using namespace std;

class Node
{
public:
	int x;
	int y;
	int cost;
	int heuristic;

	int father_x;
	int father_y;

	bool isDiscovered;
	bool isFrontier;

	Node()
	{
		this->x = 0;
		this->y = 0;
		this->cost = 0;
		this->heuristic = 0;

		isDiscovered = false;
		isFrontier = false;
	}

	Node(int x, int y)
	{
		this->x = x;
		this->y = y;
		this->cost = 0;
		this->heuristic = 0;
		this->father_x = -1;
		this->father_y = -1;
		isDiscovered = false;
		isFrontier = false;
	}

	Node(const Node& n)
	{
		this->x = n.x;
		this->y = n.y;
		this->cost = n.cost;
		this->heuristic = n.heuristic;
		this->father_x = n.father_x;
		this->father_y = n.father_y;
		isDiscovered = n.isDiscovered;
		isFrontier = n.isFrontier;
	}
};

class AStarSearch
{
public:
	AStarSearch()
	{
		lookupField = NULL;
	}

	~AStarSearch()
	{
		this->deleteLookupField();
	}

public:
	static const int STEP_COST = 1;

	void getRoute(MineMap* m, Point &start, Point &dest, list<Point> &route,
		TCheckFunction checkFunc, char* forbidCells);

private:
	int width;
	int height;
	Node** lookupField;

	void removeNodeFromFrontier(Node &n, list<Node> &nodes);

	void initLookupField(int width, int height);

	void eraseLookupField();

	void deleteLookupField();

	int getManhattenDistance(Node a, Node b);

	bool addPossibleNeighbors(MineMap* m, Node &n, Node &dest,
			list<Node> &frontier, TCheckFunction checkFunc, char* forbidCells);

	Node getOptimalNode(list<Node> &nodes);

	Node getNode(int x, int y, list<Node> &nodes);
};
