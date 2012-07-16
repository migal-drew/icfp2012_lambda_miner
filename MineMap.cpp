#include "MineMap.h"


#include <string.h>
#include <queue>

#include <algorithm>

using namespace std;

void MineMap::openFileForMap(char *fileName)
{
    if (m_inputFile == stdin || m_inputFile == NULL )
    {
        m_inputFile = fopen(fileName, "r");
        if (m_inputFile == NULL)
        {
            m_inputFile = stdin;
        }
    }
}

int MineMap::getOneChar()
{
    if (m_inputFile != NULL)
    {
        int ans = getc( m_inputFile );
        if (ans == EOF)
        {
            fclose( m_inputFile );
        }
        return ans;
    }
    return 0;
}

int MineMap::META_DEFAULT[MineMap::META_SIZE] = {0, 0, 10, 25, 0};
const char* MineMap::META_NAME[MineMap::META_SIZE] = {"Water", "Flooding", "Waterproof", "Growth", "Razors"};

MineMap::MineMap() : m_width(m_Size.x), m_height(m_Size.y),
m_Water(m_metadata[0]), m_Flooding(m_metadata[1]), m_Waterproof(m_metadata[2]),
m_Growth(m_metadata[3]), m_RazorAmount(m_metadata[4])
{

    //ctor
	m_Score = 0;
	m_isChanged = false;
    m_Map = NULL;
    m_Size.x = 0;
    m_Size.y = 0;
    m_Lift = m_Size;
    m_Robot = m_Size;
    for (int i = 0; i < TRAMPOLAINES_AMOUNT; i++)
    {
        m_startTrampolines[i] = m_endTrampolines[i] = NULL;
        m_tramplinesAcces[i] = -1;
    }
    m_tramplanes = NULL;
    m_tramplanesCount = 0;
    for (int i = 0; i < META_SIZE; i++)
        m_metadata[i] = META_DEFAULT[i];
    m_inputFile = stdin;
}

MineMap::~MineMap()
{
    if (m_height > 0)
    {
        if (m_width > 0)
            for (int i = 0; i < m_height; i++)
                delete [] m_Map[i];
        delete m_Map;
    }

    for (int i = 0; i < TRAMPOLAINES_AMOUNT; i++)
    {
        if (m_startTrampolines[i])
            delete m_startTrampolines[i];
        if (m_endTrampolines[i])
            delete m_endTrampolines[i];
    }
    if (m_tramplanes)
        delete [] m_tramplanes;
    //dtor
}

/*
Заполняет информацию о трамплинах, лямбдах, лифте и роботе.
*/
void MineMap::fillMapCache(char ch, int x, int y)
{
    if (ch >= START_TRAMPLAINE_CHAR && ch < START_TRAMPLAINE_CHAR + TRAMPOLAINES_AMOUNT)
    {
        m_startTrampolines[ch - START_TRAMPLAINE_CHAR] = new Point(x, y);
    }
    else if (ch >= START_TRAGET_CHAR && ch < START_TRAGET_CHAR + TRAMPOLAINES_AMOUNT)
    {
        m_endTrampolines[ch - START_TRAGET_CHAR] = new Point(x, y);
    }
    else switch (ch)
    {
        case 'R' : m_Robot = Point(x, y); break;
        case 'L' : m_Lift = Point(x, y); break;
        case '\\' : m_Lambdas.push_back(Point(x, y)); break;
        case 'W' : m_Beards.push_back(Point(x, y)); break;
        case '!' : m_Razors.push_back(Point(x, y)); break;
        case '@' : m_HORock.push_back(Point(x, y)); break;
    }
}

/*
Чтение карты
*/
void MineMap::ReadMap()
{
    using namespace std;

    /* Читаем первую строчку в буфер-очередь. Собственно для того, что бы узнать размер строки*/
    queue<char> firstLine;
    char ch;
    int cnt = 0;
    while (((ch = getOneChar()) != EOF) && (ch != '\n'))
    {
        firstLine.push(ch);
        fillMapCache(ch, cnt, 0);
        cnt++;
    }
    /* Создаём буфер-очередь карты, состоящей из строчек*/
    int &width = m_width;
    int &height = m_height;
    width = firstLine.size();
    height = 1;
    queue<char*> map;
    /* Загоняем туда первую строчку*/
    char *tmp = new char[width];
    for (int i = 0; i < width; i++)
    {
        tmp[i] = firstLine.front();
        firstLine.pop();
    }
    map.push(tmp);
    /* Читаем до конца файла или пока не встретится строчка, начинающаяся с '\n' */
    ch = getOneChar();
    while ((ch != EOF) && (ch != '\n'))
    {
        /* Собираем всю строчку и записываем её в конечный буфер (для каждой строки свой массив)*/
        tmp = new char [width];
        int i;
        for (i = 0; i < width && ch != '\n'; i++, ch = getOneChar())
        {
            tmp[i] = ch;
            fillMapCache(ch, i, height);
        }
        /* Добиваем до конца пробелами, если строка короткая */
        while (i < width)
            tmp[i++] = empty;
        /* читаем следующий символ */
        ch = getOneChar();
        /* загоняем адрес созданной строки в буфер-карту */
        map.push(tmp);
        height++;
    }

    /* Переписываем из буфер-карты(очереди) в нормальный массив (создаём матрицу)*/
    m_Map = new char* [height];
    for (int i = 0; i < height; i++)
    {
        m_Map[i] = map.front();
        map.pop();
    }
    /* Подсчитываем, сколько было найдено трамплинов и создаём массив с найденными*/
    for (int i = 0; i < TRAMPOLAINES_AMOUNT; i++)
        if (m_startTrampolines[i])
            m_tramplanesCount++;
    if (m_tramplanesCount)
        m_tramplanes = new unsigned int [m_tramplanesCount];
    int trampolaineNum = 0;

    /* Далее будем читать дополнительную информацию о карте*/
    int result;
    char* buffer = new char[100];
    buffer[99] = 0;
    int value = 0;
    do
    {
        /* Читаем строчку до пробела для последующей расшифровки типа информации*/
        result = fscanf(m_inputFile, "%99s", buffer);
        if (result > 0)
        {
            /* Если начинается с "Trampoline", значит это трамплин и заносим информацию для него*/
            if (!strcmp("Trampoline", buffer))
            {
                char trmp[5] = "qwer";
                char *start = &trmp[0], *end = &trmp[2];
                /* Читаем по формату " A targets 1"*/
                /* Первый пробел игнорим, так же игнорим 9 символов " targets "*/
                result = fscanf(m_inputFile, "%*c%c%*9c%c", start, end);
                /* Записываем в список трамплинов порядковый номер в массиве всех трамплинов */
                int startTramp = m_tramplanes[trampolaineNum++] = *start - START_TRAMPLAINE_CHAR;
                /* Заносим информацию о доступе этого трамплина к цели, для этого в соответствующем */
                /* массиве в порядковом номере трамплина ставим порядковый номер цели */
                m_tramplinesAcces[startTramp] = *end - START_TRAGET_CHAR;
            }
            else
            /* Иначе это другая инфа, проверим на Water, Flooding, Waterproof*/
                for (int i = 0; i < META_SIZE; i++)
                {
                    if (!strcmp(META_NAME[i], buffer))
                    {
                        result = fscanf(m_inputFile, "%d", &value);
                        m_metadata[i] = value;
                        break;
                    }
                }
        }
    /* Всё это делаем, пока не достигнем конца ввода */
    } while (result > 0);
    delete [] buffer;
}

/*
Важная функция - вывод на экран карты =)
Для изменения вывода используются флаги (enum PrintStyle)
по умолчанию выводится всё (PSFull)
*/
void MineMap::PrintMap(int style)
{
    if (style & PSMap)
    {
        /* Выводим карту с рамкой */
        int water_start = m_height - m_Flooding;
        printf("Map %d*%d:\n", m_width, m_height);
        printf("  +");
        for (int i = 0; i < m_width; i++)
            printf("-");
        printf("+\n");
        for (int i = 0; i < m_height; i++)
        {
            printf("  ");
            /* Если уровень затоплен, то ставим значок "~" */
            if (i >= water_start)
                printf("~");
            else
                printf("|");
            /* Выводим само содержимое карты*/
            for (int j = 0; j < m_width; j++)
                printf("%c", m_Map[i][j]);
            if (i >= water_start)
                printf("~");
            else
                printf("|");
            printf("\n");
        }
        /* Нижняя граница рамки*/
        printf("  +");
        for (int i = 0; i < m_width; i++)
            printf("-");
        printf("+\n");
    }
    if (style & PSMeta)
    {
        /* Вывод метаданных*/
        printf("metadata:\n");
        for (int i = 0; i < META_SIZE; i++)
        {
            printf("  %s = %d\n", META_NAME[i], m_metadata[i]);
        }
    }

    if (style & (PSRobot | PSLift))
    {
        /* Вывод информации о положении робота и лифта */
        printf("another information:\n");
        if (style & PSRobot)
            printf("  Robot at [%d, %d]\n", m_Robot.x, m_Robot.y);
        if (style & PSLift)
            printf("  Lift at [%d, %d] is %s\n", m_Lift.x, m_Lift.y, (m_Map[m_Lift.y][m_Lift.x] == 'O')?"open":"close");
    }
    if (style & (PSLambda | PSTramplaines))
    {
        /* Вывод информации о расположении всех лямбд и трамплинов в два столбца */
        int i = 0;
        int lambdaCount = m_Lambdas.size() + m_HORock.size();

        if (style & PSLambda)
            printf(" +----------------+");
        else
            printf("                  +");
        if (style & PSTramplaines)
            printf("-------------------------------------------+\n");
        else
            printf("\n");

        if (style & PSLambda)
            printf(" |%3d Lambdas at: |", lambdaCount);
        else
            printf("                  |");
        if (style & PSTramplaines)
            printf("    %3d Tramplanes:                        |\n", m_tramplanesCount);
        else
            printf("\n");

        if (style & PSLambda)
            printf(" +----------------+");
        else
            printf("                  +");
        if (style & PSTramplaines)
            printf("-------------------------------------------+\n");
        else
            printf("\n");

        while (((style & PSLambda) && i < lambdaCount) || ((style & PSTramplaines) && i < m_tramplanesCount))
        {
            if (style & PSLambda)
                printf(" |  ");
            else
                printf("    ");
            if (i < lambdaCount && (style & PSLambda))
                if (i < m_Lambdas.size())
                    printf("  [%3d, %3d]  ", m_Lambdas.at(i).x, m_Lambdas.at(i).y);
                else
                    printf("* [%3d, %3d]  ", m_HORock.at(i - m_Lambdas.size()).x, m_HORock.at(i - m_Lambdas.size()).y);
            else if (!lambdaCount && !i && (style & PSLambda))
                printf("no lambdas.   ");
            else
                printf("              ");
            printf("|    ");

            if (i < m_tramplanesCount && (style & PSTramplaines))
            {
                int start = m_tramplanes[i];
                int end = m_tramplinesAcces[m_tramplanes[i]];
                printf("from '%c' [%3d, %3d] to '%c' [%3d, %3d]  |\n",
                    START_TRAMPLAINE_CHAR + start, m_startTrampolines[start]->x, m_startTrampolines[start]->y,
                    START_TRAGET_CHAR + end, m_endTrampolines[end]->x, m_endTrampolines[end]->y);
            }
            else if (!m_tramplanesCount && !i && (style & PSTramplaines))
                printf("no tramplaines.                        |\n");
            else
                printf("                                       %c\n", (style & PSTramplaines)?'|':' ');
            i++;
        }

        if (style & PSLambda)
            printf(" +----------------+");
        else
            printf("                  +");
        if (style & PSTramplaines)
            printf("-------------------------------------------+\n");
        else
            printf("\n");
    }
    if (style & (PSBeards | PSRazors))
    {
        printf("about growth:\n");
        int BeardCount = m_Beards.size();
        int RazorCount = m_Razors.size();
        int i = 0;

        if (style & PSBeards)
            printf(" +--------------+");
        else
            printf("                +");
        if (style & PSRazors)
            printf("----------------+\n");
        else
            printf("\n");

        if (style & PSBeards)
            printf(" | %3d Beards:  |", BeardCount);
        else
            printf("                |");
        if (style & PSRazors)
            printf("  %3d Razors:   |\n", RazorCount);
        else
            printf("\n");

        if (style & PSBeards)
            printf(" +--------------+");
        else
            printf("                +");
        if (style & PSRazors)
            printf("----------------+\n");
        else
            printf("\n");

        while (((style & PSBeards) && i < BeardCount) || ((style & PSRazors) && i < RazorCount))
        {
            if (style & PSBeards)
                printf(" |  ");
            else
                printf("    ");

            if (i < BeardCount && (style & PSBeards))
                printf("[%3d, %3d]  ", m_Beards.at(i).x, m_Beards.at(i).y);
            else if (!BeardCount && !i && PSBeards)
                printf("no beards.  ");
            else
                printf("            ");

            printf("|    ");

            if (i < RazorCount && (style & PSRazors))
                printf("[%3d, %3d]  |\n", m_Razors.at(i).x, m_Razors.at(i).y);
            else if (!RazorCount && !i && (style & PSRazors))
                printf("no razors   |\n");
            else
                printf("            %c\n", (style & PSRazors)?'|':' ');
            i++;
        }

        if (style & PSBeards)
            printf(" +--------------+");
        else
            printf("                +");
        if (style & PSRazors)
            printf("----------------+\n");
        else
            printf("\n");
    }
    if (style)
        printf("It is all\n");
}

bool charInString(char c, char* str)
{
    int i = 0;
    while (str[i])
        if (c == str[i])
            return true;
        else
            i++;
    return false;
}

/*
Получение списка точек около curPoint
добавляются только те, которые удовлетворяют функции func, если она не NULL
и те, которые входят в строчку mask, если include = true или не входят иначе
например
GetListOfPoint(list, Point(2,2), NULL, "*#", true);
вернёт все камни и стены около 2,2
GetListOfPoint(list, Point(2,2), NULL, "*#", false);
вернёт все не камни и не стены
по идеи можно и так:
GetListOfPoint(list, Point(2,2), NULL, (char*)(MapValue[]){rock, wall}, true);
*/
void MineMap::GetListOfPoint(list<Point>& outList, Point curPoint, TCheckFunction func, char * mask, bool include)
{
    Point lt = Point((curPoint.x == 0)?curPoint.x:curPoint.x-1, (curPoint.y == 0)?curPoint.y:curPoint.y-1);
    Point br = Point((curPoint.x == m_width - 1)?curPoint.x:curPoint.x+1, (curPoint.y == m_height - 1)?curPoint.y:curPoint.y+1);
    for (int i = lt.y; i <= br.y; i++)
        for (int j = lt.x; j < br.x; j++)
        {
            char ch = m_Map[lt.y][lt.x];
            bool inMask = charInString(ch, mask);
            bool MaskCheck = (include)?inMask:!inMask;
            if (MaskCheck && (func == NULL || func(m_Map, this->GetWidth(), this->GetHeight(), Point(j,i))))
            {
                outList.push_back(Point(j, i));
            }
        }
}

/* Возвращает список трамплинов в виде пар Источник - Приёмник */
list<pair<Point, Point> > MineMap::GetTramplainPairs()
{
    list<pair<Point, Point> > answer;
    for (int i = 0; i < m_tramplanesCount; i++)
    {
        Point start = *m_startTrampolines[m_tramplanes[i]];
        Point end = *m_endTrampolines[m_tramplinesAcces[m_tramplanes[i]]];
        answer.push_back(pair<Point, Point>(start, end));

    }
    return answer;
}

void MineMap::GetListOrthogonalPoints(list<Point>& outList, Point curPoint, TCheckFunction func, char * mask, bool include)
{
    Point top = Point(curPoint.x, (curPoint.y == 0) ? curPoint.y : curPoint.y - 1);
    Point bottom = Point(curPoint.x, (curPoint.y == m_height - 1) ? curPoint.y : curPoint.y + 1);
    Point left = Point((curPoint.x == 0) ? curPoint.x : curPoint.x - 1, curPoint.y);
    Point right = Point((curPoint.x == m_width - 1)?curPoint.x:curPoint.x + 1, curPoint.y);

    Point cells[] = {top, bottom, left, right};
    for (int i = 0; i < 4; i++)
    {
        char ch = m_Map[cells[i].y][cells[i].x];
        bool inMask = charInString(ch, mask);
        bool MaskCheck = (include)?inMask:!inMask;
        if (MaskCheck && (func == NULL
        		|| func(m_Map, this->GetWidth(), this->GetHeight(),
        				Point(cells[i].x, cells[i].y))))
            outList.push_back(Point(cells[i].x, cells[i].y));
    }
}

void MineMap::CheckHORock(Point point)
{
	if (point.y >= (m_height - 1))
	{
		m_Map[point.y][point.x] = '\\';
		m_Lambdas.push_back(point);
	}
	char c = m_Map[point.y][point.x];
	if (c != ' ')
	{
		m_Map[point.y][point.x] = '\\';
		m_Lambdas.push_back(point);
	}
}

bool MineMap::MoveRobot(Direction direction)
{
    bool result = false;

	Point nextPoint = m_Robot + direction;

	if ((direction == 5) && (m_RazorAmount > 0))
	{
		m_RazorAmount--;
		CutBeard(nextPoint + up + left);
		CutBeard(nextPoint + up);
		CutBeard(nextPoint + up + right);
		CutBeard(nextPoint + right);
		CutBeard(nextPoint + down + right);
		CutBeard(nextPoint + down);
		CutBeard(nextPoint + down + left);
		CutBeard(nextPoint + left);
		return false;
	}

	char cellType = m_Map[nextPoint.y][nextPoint.x];

	m_Score--;
	switch (cellType)
	{
	case ' ':
		{
			break;
		}
	case '.':
		{
			break;
		}
	case '@':
		{
			if ((direction != left) && (direction != right))
				break;
			Point nextNextPoint = nextPoint + direction;
			if (m_Map[nextNextPoint.y][nextNextPoint.x] == ' ')
			{
				MoveRobotAndUpdateMap(nextPoint);
				m_Map[nextNextPoint.y][nextNextPoint.x] = '@';
			}
			break;
		}
	case '*':
		{
			if ((direction != left) && (direction != right))
				break;
			Point nextNextPoint = nextPoint + direction;
			if (m_Map[nextNextPoint.y][nextNextPoint.x] == ' ')
			{
				MoveRobotAndUpdateMap(nextPoint);
				m_Map[nextNextPoint.y][nextNextPoint.x] = '*';
			}
			break;
		}
	case '#':
		{
			break;
		}
	case 'L':
		{
			break;
		}
	case 'O':
		{
			MoveRobotAndUpdateMap(nextPoint);
			return true;
		}
	case 'W':
		{
			break;
		}
	case '!':
		{
			m_Razors.erase(find(m_Razors.begin(), m_Razors.end(), nextPoint));
			m_RazorAmount++;
			MoveRobotAndUpdateMap(nextPoint);
			break;
		}
	case '\\':
		{
			MoveRobotAndUpdateMap(nextPoint);
			#include <algorithm>
			m_Lambdas.erase(find(m_Lambdas.begin(), m_Lambdas.end(), nextPoint));
			m_Score += 25;
			return false;
		}
	default: 
		{
			break;
		}
	}
	MoveRobotAndUpdateMap(nextPoint);
    return result;
}

int MineMap::UpdateMineMap()
{
	m_isChanged = false;
	char** newMap = new char*[m_height];
	for (int i = 0; i < m_height; i++)
	{
		newMap[i] = new char[m_width];
		for (int j = 0; j < m_width; j++)
		{
			newMap[i][j] = ' ';
		}
	}
	if (m_metadata[1] != 0)
	{
		m_Flooding--;
		if (m_Flooding <= 0)
		{
			m_Water++;
			if (m_Water > m_height)
			{
				m_Water = m_height;
			}
			m_Flooding = m_metadata[1];
		}
		if ((m_Robot.y) <= (m_height - m_Water))
		{
			m_Waterproof--;
		}
		else
		{
			m_Waterproof = m_metadata[2];
		}
		if (m_Waterproof <= 0)
		{
			return 1;
		}
	}

	if (m_metadata[3] != 0)
	{
		m_Growth--;
		if (m_Growth <= 0)
		{
			for (int i = 0; i < m_height; i++)
			{
				for (int j = 0; j < m_width; j++)
				{
					if (m_Map[i][j] == 'W')
					{
						m_isChanged = true;
						SetBeard(newMap, Point(j, i) + up + left);
						SetBeard(newMap, Point(j, i) + up);
						SetBeard(newMap, Point(j, i) + up + right);
						SetBeard(newMap, Point(j, i) + right);
						SetBeard(newMap, Point(j, i) + down + right);
						SetBeard(newMap, Point(j, i) + down);
						SetBeard(newMap, Point(j, i) + down + left);
						SetBeard(newMap, Point(j, i) + left);
					}
				}
			}
			m_Growth = m_metadata[3];
		}
	}

	bool openLift = true;
	for (int i = 0; i < m_height; i++)
	{
		for (int j = 0; j < m_width; j++)
		{
			switch(m_Map[i][j])
			{
				case '@':
				{
					if (i < (m_height - 1))
					{
						//1 условие
						if (m_Map[i + 1][j] == ' ')
						{
							m_isChanged = true;
							newMap[i][j] = ' ';
							newMap[i + 1][j] = '@';
							CheckHORock(Point(j, i + 1));
							continue;
						}


						//2..4 условия
						if ((j < (m_width - 1)))
						{
							if ((m_Map[i][j + 1] == ' ') && (m_Map[i + 1][j + 1] == ' '))
							{
								//2 и 4 условия
								if ((m_Map[i + 1][j] == '@') || (m_Map[i + 1][j] == '*') || (m_Map[i + 1][j] == '\\'))
								{
									m_isChanged = true;
									newMap[i][j] = ' ';
									newMap[i + 1][j + 1] = '@';
									CheckHORock(Point(j + 1, i + 1));
									continue;
								}
							}
							else
							{
								//3 условие
								if ((j > 0) && (((m_Map[i + 1][j] == '*') || (m_Map[i + 1][j] == '*')))
									&& (m_Map[i][j - 1] == ' ') && (m_Map[i + 1][j - 1] == ' '))
								{
									m_isChanged = true;
									newMap[i][j] = ' ';
									newMap[i + 1][j - 1] = '@';
									CheckHORock(Point(j - 1, i + 1));
									continue;
								}
							}
						}
					}
					//камень остается на месте
					newMap[i][j] = '@';
					CheckHORock(Point(j, i));
					break;
				}

			case '*':
				{
					if (i < (m_height - 1))
					{
						//1 условие
						if (m_Map[i + 1][j] == ' ')
						{
							m_isChanged = true;
							newMap[i][j] = ' ';
							newMap[i + 1][j] = '*';
							continue;
						}


						//2..4 условия
						if ((j < (m_width - 1)))
						{
							if ((m_Map[i][j + 1] == ' ') && (m_Map[i + 1][j + 1] == ' '))
							{
								//2 и 4 условия
								if ((m_Map[i + 1][j] == '@') ||(m_Map[i + 1][j] == '*') || (m_Map[i + 1][j] == '\\'))
								{
									m_isChanged = true;
									newMap[i][j] = ' ';
									newMap[i + 1][j + 1] = '*';
									continue;
								}
							}
							else
							{
								//3 условие
								if ((j > 0) && ((m_Map[i + 1][j] == '*') || (m_Map[i + 1][j] == '@'))
									&& (m_Map[i][j - 1] == ' ') && (m_Map[i + 1][j - 1] == ' '))
								{
									m_isChanged = true;
									newMap[i][j] = ' ';
									newMap[i + 1][j - 1] = '*';
									continue;
								}
							}
						}
					}
					//камень остается на месте
					newMap[i][j] = '*';
					break;
				}
			case '\\':
				{
					openLift = false;
					break;
				}
			default:
				{
					if (newMap[i][j] == ' ')
					{
						newMap[i][j] = m_Map[i][j];
						break;
					}
				}
				break;
			}
		}
	}
	if (openLift)
	{
		newMap[m_Lift.y][m_Lift.x] = 'O'; // ...[y, x] - Oo
	}

	for (int i = 0; i < m_height; i++)
	{
		delete[] m_Map[i];
	}
	delete[] m_Map;
	m_Map = newMap;	
	return 0;	
}

void MineMap::MoveRobotAndUpdateMap(Point nextRobotPos)
{
	m_Map[m_Robot.y][m_Robot.x] = ' ';
	m_Robot = nextRobotPos;
	m_Map[m_Robot.y][m_Robot.x] = 'R';
}

void MineMap::SetBeard(char** newMap, Point point)
{
	if ((point.x < 0) || (point.x >= m_width) 
		|| (point.y < 0) || (point.y >= m_height))
	{
		return;
	}
	if (' ' == m_Map[point.y][point.x])
	{
		newMap[point.y][point.x] = 'W';
		m_Beards.push_back(point);
	}
}

void MineMap::CutBeard(Point point)
{
	if ((point.x < 0) || (point.x >= m_width) 
		|| (point.y < 0) || (point.y >= m_height))
	{
		return;
	}
	if ('W' == m_Map[point.y][point.x])
	{
		m_Map[point.y][point.x] = ' ';
		m_Beards.erase(find(m_Beards.begin(), m_Beards.end(), point));
	}
}

bool MineMap::isChanged()
{
	return m_isChanged;
}

int MineMap::Waterproof()
{
	return m_Waterproof;
}
