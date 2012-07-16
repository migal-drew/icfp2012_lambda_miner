#include "AStarSearch.h"

/*
* Heuristic
*/
int AStarSearch::getManhattenDistance(Node a, Node b)
{
	return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
}

/*
* Add into frontier surrounding undiscovered cells which are not walls or rocks
* and are undiscovered
*/
bool AStarSearch::addPossibleNeighbors(MineMap* m, Node &n, Node &dest,
	list<Node> &frontier, TCheckFunction checkFunc, char* forbidCells)
{
	bool isAdded = false;
	list<Point>* neighbours = new list<Point>();
	//Get points from MineMap
	m->GetListOrthogonalPoints(*neighbours, Point(n.x, n.y), checkFunc, forbidCells, false);

	list<Point>::const_iterator ip;
	list<Node>::const_iterator in;
	for (ip = neighbours->begin(); ip != neighbours->end(); ip++)
	{
		Node tmp = lookupField[(*ip).y][(*ip).x];
		if (!tmp.isDiscovered && !tmp.isFrontier)
		{
			Node newNode((*ip).x, (*ip).y);
			newNode.cost = n.cost + STEP_COST;
			newNode.heuristic = this->getManhattenDistance(newNode, dest);
			newNode.father_x = n.x;
			newNode.father_y = n.y;
			newNode.isFrontier = true;
			frontier.push_back(newNode);
			lookupField[newNode.y][newNode.x] = newNode;
			isAdded = true;
		}
	}

	delete neighbours;

	return isAdded;
}

/*
* returns node with the minimal estimation (cost + heuristic)
*/
Node AStarSearch::getOptimalNode(list<Node> &nodes)
{
	list<Node>::const_iterator i;
	int best = 2147483647;
	Node candidate;

	for (i = nodes.begin(); i != nodes.end(); i++) 
	{
		if ((*i).cost + (*i).heuristic < best)
		{
			candidate = (*i);
			best = candidate.cost + candidate.heuristic;
		}
	}

	return candidate;
}

bool isDestination(Node &p, Node &dest)
{
	return p.x == dest.x && p.y == dest.y;
}

bool isStart(Node &p, Node &start)
{
	return p.x == start.x && p.y == start.y;
}

void AStarSearch::removeNodeFromFrontier(Node &n, list<Node> &nodes)
{
	list<Node>::iterator i;
	for (i = nodes.begin(); i != nodes.end(); i++)
		if ((*i).x == n.x && (*i).y == n.y)
		{
			lookupField[(*i).y][(*i).x].isDiscovered = true;
			lookupField[(*i).y][(*i).x].isFrontier = false;
			nodes.erase(i);

			return;
		}
}

Node AStarSearch::getNode(int x, int y, list<Node> &nodes)
{
	list<Node>::iterator i;
	for (i = nodes.begin(); i != nodes.end(); i++)
		if ((*i).x == x && (*i).y == y)
			return (*i);

	return Node(-1, -1);
}

void AStarSearch::deleteLookupField()
{
	for (int i = 0; i < height; i++)
		delete[] lookupField[i];

	delete[] lookupField;

	width = 0;
	height = 0;
}

void AStarSearch::initLookupField(int width, int height)
{
	this->width = width;
	this->height = height;

	lookupField = new Node*[height];
	for (int i = 0; i < height; i++)
	{
		lookupField[i] = new Node[width];
	}
}

void AStarSearch::eraseLookupField()
{
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
		{
			lookupField[i][j].isDiscovered = false;
			lookupField[i][j].isFrontier = false;
			lookupField[i][j].father_x = -1;
			lookupField[i][j].father_y = -1;
		}
}

void AStarSearch::getRoute(MineMap* m, Point &start, Point &dest, list<Point> &route,
	TCheckFunction checkFunc, char* forbidCells)
{
	list<Node>* frontier = new list<Node>();

	if (lookupField == NULL)
		this->initLookupField(m->GetWidth(), m->GetHeight());
	else
		this->eraseLookupField();

	//Destination
	Node d = Node(dest.x, dest.y);
	//Start position
	Node s = Node(start.x, start.y);
	//Initialization
	s.heuristic = this->getManhattenDistance(s, d);
	d.heuristic = 0;
	//Add neighbors for start node
 	this->addPossibleNeighbors(m, s, d, *frontier, checkFunc, forbidCells);
	lookupField[s.y][s.x] = s;
	lookupField[s.y][s.x].isDiscovered = true;

	bool frontierChanged = true;
	bool success = false;
	int prevSize = -1;

	int i = 0;
	while (frontier->size() && (frontierChanged))
	{
		Node n = getOptimalNode(*frontier);
		if (!isDestination(n, d))
		{
			prevSize = frontier->size();
			//into frontier add possible neighbours
			bool isAdded = addPossibleNeighbors(m, n, d, *frontier, checkFunc, forbidCells);
			this->removeNodeFromFrontier(n, *frontier);
			bool isSizeChanged = prevSize != frontier->size();
			frontierChanged = isAdded || isSizeChanged;
		}
		else
		{
			//We found our goal!
			d.cost = n.cost;
			d.heuristic = n.heuristic;
			d.father_x = n.father_x;
			d.father_y = n.father_y;
			success = true;

			break;
		}
	}

	if (success)
	{
		Node tmp = d;
		//Reconstructing path
		while(!isStart(tmp, s))
		{
			route.push_front(Point(tmp.x, tmp.y));
			tmp = lookupField[tmp.father_y][tmp.father_x];
		}
	}

	frontier->clear();
	delete frontier;
}
