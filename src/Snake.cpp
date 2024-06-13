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
int currentMapIndex = 1;
int itemCount = 0;
int gateCount = 0;

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

// Function prototype for loadMap
void loadMap(int mapIndex);

class Snake
{
private:
    std::vector<Position> body;
    int direction;
    int prevDirection;

public:
    Snake(int mapWidth, int mapHeight)
    {
        direction = 2;
        prevDirection = 2;

        int startX = mapWidth / 2;
        int startY = mapHeight / 2;

        body.push_back({startX, startY});
        body.push_back({startX - 1, startY});
        body.push_back({startX - 2, startY});

        map[startY][startX] = 3;
        map[startY][startX - 1] = 4;
        map[startY][startX - 2] = 4;
    }

    void resetPosition()
    {
        Position newStart = {1,1}; // 새로운 시작 위치
        direction = prevDirection; // 아래 방향

        int length = body.size();
        body.clear();
        direction = prevDirection; // 아래 방향

        for (int i = 0; i < length; i++)
        {
            body.push_back({newStart.x, newStart.y + i});
            if (i == 0)
                map[newStart.y + i][newStart.x] = 3; // head
            else
                map[newStart.y + i][newStart.x] = 4; // body
        }
    }

    void changeDirection(int newDirection)
    {
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

            gateCount++;
        }

        if (checkCollision(newHead))
        {
            endwin();
            printf("Game Over! Score: %ld\n", body.size());
            exit(0);
        }

        bool grew = false;
        int growthUnits = 1;

        if (map[newHead.y][newHead.x] == 5 || map[newHead.y][newHead.x] == 6 || map[newHead.y][newHead.x] == 9)
        {
            itemCount++;
            if (map[newHead.y][newHead.x] == 5)
            {
                grew = true;
                map[newHead.y][newHead.x] = 0;
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
            }
            else if (map[newHead.y][newHead.x] == 9)
            {
                grew = true;
                growthUnits = 2;
                map[newHead.y][newHead.x] = 0;
            }

            if (checkMissionCompletion())
            {
                itemCount = 0;
                gateCount = 0;

                if(currentMapIndex != 5){
                    currentMapIndex = (currentMapIndex % 5) + 1;
                }else{
                    printf("Clear! your point is %ld\n", body.size());
                    exit(0);
                }
                loadMap(currentMapIndex);
                resetPosition();
            }
        }

        // Move the snake
        body.insert(body.begin(), newHead);

        // Update the map
        map[newHead.y][newHead.x] = 3;
        if (body.size() > 1)
        {
            map[body[1].y][body[1].x] = 4;
        }

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
        for (int i = 0; i < HEIGHT; ++i)
        {
            for (int j = 0; j < WIDTH; ++j)
            {
                char displayChar = ' ';
                switch (map[i][j])
                {
                    case 1:
                        displayChar = '#';
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
                    case 9:
                        displayChar = '*';
                        break;
                }
                mvaddch(i, j, displayChar);
            }
        }
        displayMission();
    }

    bool checkCollision(Position newHead)
    {
        if (newHead.x <= 0 || newHead.x >= WIDTH - 1 || newHead.y <= 0 || newHead.y >= HEIGHT - 1)
        {
            return true;
        }

        if (map[newHead.y][newHead.x] == 1)
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
                if (map[i][j] == 5 || map[i][j] == 6 || map[i][j] == 9)
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
                    map[y][x] = 9;
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

    void handleItems()
    {
        static auto lastItemTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastItemTime;

        if (elapsed.count() >= 5.0)
        {
            placeItems();
            lastItemTime = currentTime;
        }
    }

    bool checkMissionCompletion()
    {
        switch (currentMapIndex)
        {
            case 1:
                return itemCount >= 3 && gateCount >= 2;
            case 2:
                return itemCount >= 1 && gateCount >= 2;
            case 3:
                return itemCount >= 2 && gateCount >= 2;
            case 4:
                return gateCount >= 2;
            default:
                return false;
        }
    }

    void displayMission()
    {
        mvprintw(0, WIDTH + 2, "<MISSION>");

        switch (currentMapIndex)
        {
            case 1:
                mvprintw(1, WIDTH + 2, "Items: %d / 3", itemCount);
                mvprintw(2, WIDTH + 2, "Gates: %d / 2", gateCount);
                break;
            case 2:
                mvprintw(1, WIDTH + 2, "Items: %d / 1", itemCount);
                mvprintw(2, WIDTH + 2, "Gates: %d / 2", gateCount);
                break;
            case 3:
                mvprintw(1, WIDTH + 2, "Items: %d / 2", itemCount);
                mvprintw(2, WIDTH + 2, "Gates: %d / 2", gateCount);
                break;
            case 4:
                mvprintw(1, WIDTH + 2, "Gates: %d / 2", gateCount);
                break;
        }
    }
};

void loadMap(int mapIndex)
{
    std::ifstream mapFile("map" + std::to_string(mapIndex) + ".txt");
    if (mapFile.is_open())
    {
        std::string line;
        int row = 0;
        while (std::getline(mapFile, line))
        {
            std::stringstream ss(line);
            int col = 0;
            int value;
            while (ss >> value)
            {
                map[row][col] = value;
                col++;
            }
            row++;
        }
        mapFile.close();
    }
    else
    {
        endwin();
        printf("Failed to load map file: map%d.txt\n", mapIndex);
        exit(1);
    }
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
                case 9:
                    displayChar = '*';
                    break;
            }
            mvaddch(i, j, displayChar);
        }
    }
}

void gameLoop()
{
    Snake snake(WIDTH, HEIGHT);
    snake.placeItems();

    bool gameStarted = false;

    int ch;
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    timeout(180);

    mvprintw(HEIGHT + 1, 0, "Press any arrow to start the game");
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
            if (elapsed.count() >= 0.1)
            {
                snake.move();
                snake.draw();
                snake.handleItems();
                printMap();
                refresh();
                lastUpdateTime = currentTime;
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

    loadMap(currentMapIndex);
    printMap();

    gameLoop();

    endwin();
    return 0;
}
