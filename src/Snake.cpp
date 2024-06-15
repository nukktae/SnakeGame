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
int map[HEIGHT][WIDTH];

struct Position
{
    int x, y;
};
struct Gate
{
    Position pos1;
    Position pos2;
};
std::pair<Position, Position> gates;

enum MissionType { GROWTH_ITEMS, BONUS_ITEMS, GATE_USAGE, POISON_ITEMS };
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
    {GATE_USAGE, 2, 0}
};

int currentMapIndex = 0;
const char* mapFiles[] = {"map1.txt", "map2.txt", "map3.txt", "map4.txt", "map5.txt"};
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

public:
    Snake(int mapWidth, int mapHeight)
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

        map[startY][startX] = 3;
        map[startY][startX - 1] = 4;
        map[startY][startX - 2] = 4;
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

        if (map[newHead.y][newHead.x] == 1)
        {
            endwin();
            printf("Game Over! Score: %ld\n", body.size());
            exit(0);
        }

        if (map[newHead.y][newHead.x] == 7 || map[newHead.y][newHead.x] == 8)
        {
            if (map[newHead.y][newHead.x] == 7)
            {
                newHead = gates.second;
            }
            else
            {
                newHead = gates.first;
            }

            if (newHead.x <= 0)
                newHead.x = 1;
            if (newHead.x >= WIDTH - 1)
                newHead.x = WIDTH - 2;
            if (newHead.y <= 0)
                newHead.y = 1;
            if (newHead.y >= HEIGHT - 1)
                newHead.y = HEIGHT - 2;

            if (newHead.y == gates.first.y && newHead.x == gates.first.x)
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
            else if (newHead.y == gates.second.y && newHead.x == gates.second.x)
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

            map[gates.first.y][gates.first.x] = 0;
            map[gates.second.y][gates.second.x] = 0;

            if (currentMapIndex == 2)
            {
                missions[currentMapIndex + 2].currentCount++;  
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

        if (map[newHead.y][newHead.x] == 5)
        {
            grew = true;
            map[newHead.y][newHead.x] = 0;

            if (currentMapIndex == 0)
            {
                missions[0].currentCount++;  
            }
            growthItems++;
        }
        else if (map[newHead.y][newHead.x] == 6)
        {
            if (body.size() <= 3)
            {
                endwin();
                printf("Game Over! Score: %ld\n", body.size());
                exit(0);
            }
            Position tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = 0;
            map[newHead.y][newHead.x] = 0;

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
        else if (map[newHead.y][newHead.x] == 2)
        {
            grew = true;
            growthUnits = 2;
            map[newHead.y][newHead.x] = 0;

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
        map[newHead.y][newHead.x] = 3; 

        if (!grew)
        {
            Position tail = body.back();
            body.pop_back();
            map[tail.y][tail.x] = 0;
            if (body.size() < 3)
            {
                endwin();
                printf("Game Over! Score: %ld\n", body.size());
                exit(0);
            }
        }

        // header problem solving
        if (body.size() > 1)
        {
            map[body[1].y][body[1].x] = 4;
        }

        prevDirection = direction;

        for (int i = 0; i < growthUnits - 1; i++)
        {
            Position newSegment = body.back();
            body.push_back(newSegment);
            map[newSegment.y][newSegment.x] = 4;
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
        mvaddch(gates.first.y, gates.first.x, 'G');
        mvaddch(gates.second.y, gates.second.x, 'G');
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

    void clearOldItems()
    {
        for (int i = 1; i < HEIGHT - 1; i++)
        {
            for (int j = 1; j < WIDTH - 1; j++)
            {
                if (map[i][j] == 5 || map[i][j] == 6 || map[i][j] == 2)
                {
                    map[i][j] = 0;
                }
                else if (map[i][j] == 7 || map[i][j] == 8)
                {
                    map[gates.first.y][gates.first.x] = 0;
                    map[gates.second.y][gates.second.x] = 0;
                }
            }
        }
    }

    void placeItems()
    {
        clearOldItems();

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
        placeGates();
    }

    void placeGates()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(1, WIDTH - 2);

        Position gate1, gate2;
        do
        {
            gate1 = {dis(gen), dis(gen)};
        } while (map[gate1.y][gate1.x] != 0);

        do
        {
            gate2 = {dis(gen), dis(gen)};
        } while (map[gate2.y][gate2.x] != 0 || (gate1.x == gate2.x && gate1.y == gate2.y));

        gates = {gate1, gate2};

        map[gate1.y][gate1.x] = 7;
        map[gate2.y][gate2.x] = 8;
    }

    void handleItems(float itemChangeSpeed)
    {
        static auto lastItemTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastItemTime;

        if (elapsed.count() >= itemChangeSpeed)
        {
            placeItems();
            lastItemTime = currentTime;
        }
    }

    int getLength() { return body.size(); }
    int getGrowthItems() { return growthItems; }
    int getPoisonItems() { return poisonItems; }
    int getGateUsage() { return gateUsage; }
};

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
            ss >> map[row][col];
        }
        row++;
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

void printMap()
{
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            char displayChar = ' ';
            switch (map[i][j])
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
                    displayChar = 'G';
                    break;
                case 8:
                    displayChar = 'G';
                    break;
            }
            mvaddch(i, j, displayChar);
        }
    }
}

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

void checkMissionCompletion()
{
    if (currentMapIndex == 0 &&
        missions[0].currentCount >= missions[0].targetCount &&
        missions[1].currentCount >= missions[1].targetCount &&
        missions[2].currentCount >= missions[2].targetCount)
    {
        currentMapIndex++;
        missions[3].currentCount = 0;  
        missions[4].currentCount = 0; 
        loadMap(mapFiles[currentMapIndex]);
        clear();
        printMap();
        printMission();
    }
    else if (currentMapIndex == 1 &&
             missions[3].currentCount >= missions[3].targetCount &&
             missions[4].currentCount >= missions[4].targetCount)
    {
        currentMapIndex++;
        missions[5].currentCount = 0;  
        loadMap(mapFiles[currentMapIndex]);
        clear();
        printMap();
        printMission();
    }
    else if (currentMapIndex == 2 && missions[5].currentCount >= missions[5].targetCount)
    {
        currentMapIndex++;
        loadMap(mapFiles[currentMapIndex]);
        clear();
        printMap();
        printMission();
    }
}

void gameLoop()
{
    Snake snake(WIDTH, HEIGHT);
    snake.placeItems();

    bool gameStarted = false;

    int ch;
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    auto startTime = std::chrono::high_resolution_clock::now();
    timeout(snakeSpeed[currentMapIndex]);

    mvprintw(HEIGHT + 2, 0, "Press any arrow to start the game");
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
                    startTime = std::chrono::high_resolution_clock::now(); 
                    break;
                case KEY_RIGHT:
                    snake.changeDirection(2);
                    gameStarted = true;
                    startTime = std::chrono::high_resolution_clock::now();  
                    break;
                case KEY_DOWN:
                    snake.changeDirection(3);
                    gameStarted = true;
                    startTime = std::chrono::high_resolution_clock::now();  
                    break;
                case KEY_LEFT:
                    snake.changeDirection(4);
                    gameStarted = true;
                    startTime = std::chrono::high_resolution_clock::now();  
                    break;
            }
            clear();
            printMap();
            printMission();
            printScoreBoard(snake, startTime);
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
                snake.handleItems(itemChangeSpeed[currentMapIndex]);
                printMap();
                printMission();
                printScoreBoard(snake, startTime);
                refresh();
                lastUpdateTime = currentTime;
                checkMissionCompletion();
            }
        }
    }
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
    loadMap(mapFiles[currentMapIndex]);
    printMap();

    gameLoop();

    endwin();
    return 0;
}
