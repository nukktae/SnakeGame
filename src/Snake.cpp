#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include "Map.cpp"
#include <iostream>
#include <unistd.h>

int kbhit(void) {
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);

    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
}

class Snake {
private:
    struct Segment {
        int x, y;
    };

    Segment *body;
    int length;
    int maxLength;
    int direction;
    int prevDirection;
    int width = WIDTH;
    int height = HEIGHT;
    
public:
    Snake(int mapWidth, int mapHeight) {
        direction = 2;
        prevDirection = 2;
        maxLength = WIDTH * HEIGHT;

        body = new Segment[maxLength];

        int startX = WIDTH / 2 - 5;
        int startY = HEIGHT / 2 - 5;

        length = 3;
        for (int i = 0; i < length; ++i) {
            body[i].x = startX - i;
            body[i].y = startY;
        }

    }

    ~Snake() {
        delete[] body;
    }

    void changeDirection(int newDirection) {
        if (newDirection != prevDirection && abs(newDirection - prevDirection) != 2) {
            direction = newDirection;
        }
    }

    void move() {
        int headX = body[0].x;
        int headY = body[0].y;
        if (headX <= 0 || headX >= WIDTH-2 || headY <= 0 || headY >= HEIGHT-2) {
            endwin();
            std::cout << "Game Over!" << std::endl;
            exit(0);
        }
        switch (direction) {
            case 1: headY--; break;
            case 2: headX++; break;
            case 3: headY++; break;
            case 4: headX--; break;
        }

        for (int i = length - 1; i > 0; --i) {
            body[i] = body[i - 1];
        }

        body[0].x = headX;
        body[0].y = headY;

        prevDirection = direction;
    }

    void draw() {
        mvaddch(body[0].y, body[0].x, 'H');
        for (int i = 1; i < length; ++i) {
            mvaddch(body[i].y, body[i].x, 'o');
        }
    }

    bool checkCollision() {
        int headX = body[0].x;
        int headY = body[0].y;

        if (headX <= 0 || headX >= WIDTH - 1 || headY <= 0 || headY >= HEIGHT - 1 || mvwinch(stdscr, headY, headX) == '#') {
            return true;
        }

        for (int i = 1; i < length; i++) {
            if (headX == body[i].x && headY == body[i].y) {
                return true;
            }
        }

        return false;
    }

    void runGame() {
        while (true) {
            if (kbhit()) {
                handleInput();
            }

            move();
            printMap();
            draw();
            refresh();

            usleep(100000);

            if (checkCollision()) {
                endwin();
                std::cout << "Game Over!" << std::endl;
                break;
            }
        }
    }

    void handleInput() {
        int c = getch();
        switch (c) {
            case KEY_UP:    changeDirection(1); break;
            case KEY_RIGHT: changeDirection(2); break;
            case KEY_DOWN:  changeDirection(3); break;
            case KEY_LEFT:  changeDirection(4); break;
            case 'q': endwin(); exit(0); break;
        }
    }

    void initGame() {
        initscr();
        curs_set(0);
        noecho();
        keypad(stdscr, TRUE);

        getmaxyx(stdscr, height, width);

        initMap();
        printMap();
        draw();
        refresh();
        usleep(2000000);
    }
};

int main() {
    Snake snake(40, 20);
    snake.initGame();
    snake.runGame();

    return 0;
}
