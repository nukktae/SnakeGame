#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <random>
#include <unistd.h>
#include <vector>
#include <chrono>
#include <utility>
#include <fstream>
#include <sstream>

const int HEIGHT = 21;
const int WIDTH = 21;

class Position
{
public:
    int x, y;
};

class Gate
{
private:
    Position pos1;
    Position pos2;

public:
    void clearOldGate(int map[HEIGHT][WIDTH])
    {
        for (int i = 1; i < HEIGHT - 1; i++)
        {
            for (int j = 1; j < WIDTH - 1; j++)
            {
                if (map[i][j] == 7 || map[i][j] == 8)
                {
                    map[i][j] = 0;
                }
            }
        }
    }

    void placeGate(int map[HEIGHT][WIDTH])
    {
        clearOldGate(map);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(1, WIDTH - 1);

        do
        {
            pos1 = {dis(gen), dis(gen)};
        } while (map[pos1.y][pos1.x] != 0 && map[pos1.y][pos1.x] != 1);

        do
        {
            pos2 = {dis(gen), dis(gen)};
        } while ((map[pos2.y][pos2.x] != 0 && map[pos1.y][pos1.x] != 1) || (pos1.x == pos2.x && pos1.y == pos2.y));

        map[pos1.y][pos1.x] = 7;
        map[pos2.y][pos2.x] = 8;
    }

    Position getFirst() const { return pos1; }
    Position getSecond() const { return pos2; }
};

class Item
{
public:
    void clearOldItems(int map[HEIGHT][WIDTH])
    {
        for (int i = 1; i < HEIGHT - 1; i++)
        {
            for (int j = 1; j < WIDTH - 1; j++)
            {
                if (map[i][j] == 5 || map[i][j] == 6 || map[i][j] == 2)
                {
                    map[i][j] = 0;
                }
            }
        }
    }

    void placeItems(int map[HEIGHT][WIDTH])
    {
        clearOldItems(map);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(1, WIDTH - 2);

        int growthItemsPlaced = 0;
        int poisonItemsPlaced = 0;
        int bonusItemsPlaced = 0;

        while (growthItemsPlaced < 3 || poisonItemsPlaced < 2 || bonusItemsPlaced < 1)
        {
            int x = dis(gen);
            int y = dis(gen);
            if (map[y][x] == 0)
            {
                if (growthItemsPlaced < 3)
                {
                    map[y][x] = 5;
                    growthItemsPlaced++;
                }
                else if (poisonItemsPlaced < 2)
                {
                    map[y][x] = 6;
                    poisonItemsPlaced++;
                }
                else if (bonusItemsPlaced < 1)
                {
                    map[y][x] = 2;
                    bonusItemsPlaced++;
                }
            }
        }
    }
};

class Map
{
public:
    int grid[HEIGHT][WIDTH];

    void loadMap(const char *filename)
    {
        std::ifstream file(filename);
        std::string line;
        int row = 0;
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            for (int col = 0; col < WIDTH; col++)
            {
                ss >> grid[row][col];
            }
            row++;
        }
    }

    void initMap()
    {
        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                if (i == 0 || i == HEIGHT - 1 || j == 0 || j == WIDTH - 1)
                {
                    grid[i][j] = 1;
                }
                else
                {
                    grid[i][j] = 0;
                }
            }
        }
    }

    void printMap()
    {
        for (int i = 0; i < HEIGHT; i++)
        {
            for (int j = 0; j < WIDTH; j++)
            {
                char displayChar = ' ';
                switch (grid[i][j])
                {
                case 1:
                    displayChar = '#';
                    break;
                case 2:
                    displayChar = '*';
                    break;
                case 3:
                    displayChar = 'H';
                    break;
                case 4:
                    displayChar = 'o';
                    break;
                case 5:
                    displayChar = '+';
                    break;
                case 6:
                    displayChar = '-';
                    break;
                case 7:
                case 8:
                    displayChar = 'G';
                    break;
                }
                mvaddch(i, j, displayChar);
            }
        }
    }
};

enum MissionType
{
    GROWTH_ITEMS,
    BONUS_ITEMS,
    GATE_USAGE,
    POISON_ITEMS
};
struct Mission
{
    MissionType type;
    int targetCount;
    int currentCount;
};

std::vector<Mission> missions = {
    {GROWTH_ITEMS, 3, 0},
    {POISON_ITEMS, 1, 0},
    {BONUS_ITEMS, 1, 0},
    {BONUS_ITEMS, 2, 0},
    {POISON_ITEMS, 2, 0},
    {GATE_USAGE, 2, 0}};

int currentMapIndex = 0;
const char *mapFiles[] = {"map1.txt", "map2.txt", "map3.txt", "map4.txt", "map5.txt"};
int max_length = 0;
int snakeSpeed[] = {100, 130, 150, 180};
float itemChangeSpeed[] = {5.0, 4.0, 3.0, 2.0};

class Snake
{
private:
    std::vector<Position> body;
    int direction;
    int prevDirection;
    int growthItems;
    int poisonItems;
    int gateUsage;
    Map &map;
    Gate &gate;

public:
    Snake(int mapWidth, int mapHeight, Map &m, Gate &g) : map(m), gate(g)
    {
        direction = 2;
        prevDirection = 2;
        growthItems = 0;
        poisonItems = 0;
        gateUsage = 0;

        int startX = mapWidth / 2;
        int startY = mapHeight / 2;

        body.push_back({startX, startY});
        body.push_back({startX - 1, startY});
        body.push_back({startX - 2, startY});

        map.grid[startY][startX] = 3;
        map.grid[startY][startX - 1] = 4;
        map.grid[startY][startX - 2] = 4;
    }

    void changeDirection(int newDirection)
    {
        if (abs(newDirection - direction) == 2)
        {
            endwin();
            printf("Game Over! You tried to move in the opposite direction. Score: %ld\n", body.size());
            exit(0);
        }
        if (newDirection != prevDirection && abs(newDirection - prevDirection) != 2)
        {
            direction = newDirection;
        }
    }

    void move()
    {
        Position newHead = body[0];

        switch (direction)
        {
        case 1:
            newHead.y--;
            break;
        case 2:
            newHead.x++;
            break;
        case 3:
            newHead.y++;
            break;
        case 4:
            newHead.x--;
            break;
        }
        if (map.grid[newHead.y][newHead.x] == 1)
        {
            endwin();
            printf("Game Over! Score: %ld\n", body.size());
            exit(0);
        }

        if (map.grid[newHead.y][newHead.x] == 7 || map.grid[newHead.y][newHead.x] == 8)
        {
            if (map.grid[newHead.y][newHead.x] == 7)
            {
                newHead = gate.getSecond();
            }
            else
            {
                newHead = gate.getFirst();
            }

            if (newHead.x <= 0)
                newHead.x = 1;
            if (newHead.x >= WIDTH - 1)
                newHead.x = WIDTH - 2;
            if (newHead.y <= 0)
                newHead.y = 1;
            if (newHead.y >= HEIGHT - 1)
                newHead.y = HEIGHT - 2;

            if (newHead.y == gate.getFirst().y && newHead.x == gate.getFirst().x)
            {
                if (newHead.x == 1)
                    direction = 2;
                else if (newHead.x == WIDTH - 2)
                    direction = 4;
                else if (newHead.y == 1)
                    direction = 3;
                else if (newHead.y == HEIGHT - 2)
                    direction = 1;
            }
            else if (newHead.y == gate.getSecond().y && newHead.x == gate.getSecond().x)
            {
                if (newHead.x == 1)
                    direction = 2;
                else if (newHead.x == WIDTH - 2)
                    direction = 4;
                else if (newHead.y == 1)
                    direction = 3;
                else if (newHead.y == HEIGHT - 2)
                    direction = 1;
            }

            map.grid[gate.getFirst().y][gate.getFirst().x] = 0;
            map.grid[gate.getSecond().y][gate.getSecond().x] = 0;

            if (currentMapIndex == 2)
            {
                missions[5].currentCount++;
            }

            gateUsage++;
        }

        if (checkCollision(newHead))
        {
            endwin();
            printf("Game Over! Score: %ld\n", body.size());
            exit(0);
        }

        bool grew = false;
        int growthUnits = 1;

        if (map.grid[newHead.y][newHead.x] == 5)
        {
            grew = true;
            map.grid[newHead.y][newHead.x] = 0;

            if (currentMapIndex == 0)
            {
                missions[0].currentCount++;
            }
            growthItems++;
        }
        else if (map.grid[newHead.y][newHead.x] == 6)
        {
            if (body.size() <= 3)
            {
                endwin();
                printf("Game Over! Score: %ld\n", body.size());
                exit(0);
            }
            Position tail = body.back();
            body.pop_back();
            map.grid[tail.y][tail.x] = 0;
            map.grid[newHead.y][newHead.x] = 0;

            if (currentMapIndex == 0)
            {
                missions[1].currentCount++;
            }
            else if (currentMapIndex == 1)
            {
                missions[4].currentCount++;
            }

            poisonItems++;
        }
        else if (map.grid[newHead.y][newHead.x] == 2)
        {
            grew = true;
            growthUnits = 2;
            map.grid[newHead.y][newHead.x] = 0;

            if (currentMapIndex == 0)
            {
                missions[2].currentCount++;
            }
            else if (currentMapIndex == 1)
            {
                missions[3].currentCount++;
            }
        }

        body.insert(body.begin(), newHead);
        map.grid[newHead.y][newHead.x] = 3;

        if (!grew)
        {
            Position tail = body.back();
            body.pop_back();
            map.grid[tail.y][tail.x] = 0;
            if (body.size() < 3)
            {
                endwin();
                printf("Game Over! Score: %ld\n", body.size());
                exit(0);
            }
        }

        if (body.size() > 1)
        {
            map.grid[body[1].y][body[1].x] = 4;
        }

        prevDirection = direction;

        for (int i = 0; i < growthUnits - 1; i++)
        {
            Position newSegment = body.back();
            body.push_back(newSegment);
            map.grid[newSegment.y][newSegment.x] = 4;
        }
    }

    void draw()
    {
        for (size_t i = 0; i < body.size(); ++i)
        {
            if (i == 0)
            {
                mvaddch(body[i].y, body[i].x, 'H');
            }
            else
            {
                mvaddch(body[i].y, body[i].x, 'o');
            }
        }
        mvaddch(gate.getFirst().y, gate.getFirst().x, 'G');
        mvaddch(gate.getSecond().y, gate.getSecond().x, 'G');
    }

    bool checkCollision(Position newHead)
    {
        if (newHead.x <= 0 || newHead.x >= WIDTH - 1 || newHead.y <= 0 || newHead.y >= HEIGHT - 1)
        {
            return true;
        }

        for (size_t i = 0; i < body.size(); i++)
        {
            if (newHead.x == body[i].x && newHead.y == body[i].y)
            {
                return true;
            }
        }

        return false;
    }

    void handleItemsandGate(float itemChangeSpeed, Item &item, Gate &gate)
    {
        static auto lastItemTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastItemTime;

        if (elapsed.count() >= 5.0)
        {
            item.placeItems(map.grid);
            gate.placeGate(map.grid);
            lastItemTime = currentTime;
        }
    }

    int getLength() { return body.size(); }
    int getGrowthItems() { return growthItems; }
    int getPoisonItems() { return poisonItems; }
    int getGateUsage() { return gateUsage; }
};

void printMission()
{
    mvprintw(0, WIDTH + 2, "Current Mission:");
    switch (currentMapIndex)
    {
    case 0:
        mvprintw(1, WIDTH + 2, "Growth Items: %d / 3", missions[0].currentCount);
        mvprintw(2, WIDTH + 2, "Poison Items: %d / 1", missions[1].currentCount);
        mvprintw(3, WIDTH + 2, "Bonus Items: %d / 1", missions[2].currentCount);
        break;
    case 1:
        mvprintw(1, WIDTH + 2, "Bonus Items: %d / 2", missions[3].currentCount);
        mvprintw(2, WIDTH + 2, "Poison Items: %d / 2", missions[4].currentCount);
        break;
    case 2:
        mvprintw(1, WIDTH + 2, "Gate Usage: %d / 2", missions[5].currentCount);
        break;
    case 3:
        mvprintw(1, WIDTH + 2, "Survive as long as you can!");
        break;
    }
}

void printScoreBoard(Snake &snake, const std::chrono::time_point<std::chrono::high_resolution_clock> &startTime)
{
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - startTime;
    int elapsedSeconds = static_cast<int>(elapsed.count());

    int minutes = elapsedSeconds / 60;
    int seconds = elapsedSeconds % 60;

    mvprintw(6, WIDTH + 2, "Score Board");
    mvprintw(7, WIDTH + 2, "B: %d / %d", snake.getLength(), max_length);
    mvprintw(8, WIDTH + 2, "+: %d", snake.getGrowthItems());
    mvprintw(9, WIDTH + 2, "-: %d", snake.getPoisonItems());
    mvprintw(10, WIDTH + 2, "G: %d", snake.getGateUsage());
    mvprintw(11, WIDTH + 2, "Time: %02d:%02d", minutes, seconds);
}
void checkMissionCompletion(Map &map)
{
    if (currentMapIndex == 0 &&
        missions[0].currentCount >= missions[0].targetCount &&
        missions[1].currentCount >= missions[1].targetCount &&
        missions[2].currentCount >= missions[2].targetCount)
    {
        currentMapIndex++;
        missions[3].currentCount = 0;
        missions[4].currentCount = 0;
        map.loadMap(mapFiles[currentMapIndex]);
        clear();
        map.printMap();
        printMission();
    }
    else if (currentMapIndex == 1 &&
             missions[3].currentCount >= missions[3].targetCount &&
             missions[4].currentCount >= missions[4].targetCount)
    {
        currentMapIndex++;
        missions[5].currentCount = 0;
        map.loadMap(mapFiles[currentMapIndex]);
        clear();
        map.printMap();
        printMission();
    }
    else if (currentMapIndex == 2 && missions[5].currentCount >= missions[5].targetCount)
    {
        currentMapIndex++;
        map.loadMap(mapFiles[currentMapIndex]);
        clear();
        map.printMap();
        printMission();
    }
}
void gameLoop(Map &map, Snake &snake, Item &item, Gate &gate)
{
    bool gameStarted = false;
    item.placeItems(map.grid);
    gate.placeGate(map.grid);

    int ch;
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    auto startTime = std::chrono::high_resolution_clock::now();
    timeout(snakeSpeed[currentMapIndex]);

    mvprintw(HEIGHT + 2, 0, "Press any arrow to start the game");
    // item.placeItems(map.grid);  // 시작할 때 아이템 배치
    // map.printMap();
    // snake.draw();
    printMission();
    printScoreBoard(snake, startTime);
    refresh();

    while ((ch = getch()) != 'q')
    {
        if (!gameStarted && ch != ERR)
        {
            switch (ch)
            {
            case KEY_UP:
                snake.changeDirection(1);
                gameStarted = true;
                break;
            case KEY_RIGHT:
                snake.changeDirection(2);
                gameStarted = true;
                break;
            case KEY_DOWN:
                snake.changeDirection(3);
                gameStarted = true;
                break;
            case KEY_LEFT:
                snake.changeDirection(4);
                gameStarted = true;
                break;
            }
            clear();
            map.printMap();
            // snake.draw();
            printMission();
            printScoreBoard(snake, startTime);
            // refresh();
        }

        if (gameStarted)
        {
            if (ch != ERR)
            {
                switch (ch)
                {
                case KEY_UP:
                    snake.changeDirection(1);
                    break;
                case KEY_RIGHT:
                    snake.changeDirection(2);
                    break;
                case KEY_DOWN:
                    snake.changeDirection(3);
                    break;
                case KEY_LEFT:
                    snake.changeDirection(4);
                    break;
                }
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = currentTime - lastUpdateTime;
            if (elapsed.count() >= snakeSpeed[currentMapIndex] / 1000.0)
            {
                snake.move();
                snake.draw();
                snake.handleItemsandGate(itemChangeSpeed[currentMapIndex], item, gate);
                map.printMap();
                printMission();
                printScoreBoard(snake, startTime);
                refresh();
                lastUpdateTime = currentTime;
                checkMissionCompletion(map);
            }
        }
    }
}

void calculateMaxLength()
{
    std::ifstream file(mapFiles[4]);
    std::string line;
    int zeroCount = 0;
    while (std::getline(file, line))
    {
        for (char c : line)
        {
            if (c == '0')
            {
                zeroCount++;
            }
        }
    }
    max_length = zeroCount;
}

int main()
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
    start_color();

    calculateMaxLength();
    Map map;
    // map.loadMap(mapFiles[currentMapIndex]);
    map.initMap();
    map.printMap();

    Gate gate;
    Snake snake(WIDTH, HEIGHT, map, gate);
    Item item;

    gameLoop(map, snake, item, gate);

    endwin();
    return 0;
}
