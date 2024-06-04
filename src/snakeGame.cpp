```cpp
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <ncurses.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <fstream>
#include <map>
#include <sstream>
#include <chrono>

void initializeSound();
void playBackgroundMusic();
void playSoundEffect(const char* filepath);

const int HEIGHT = 21;
const int WIDTH = 21;
int DELAY = 300000;
int itemDisappearTime = 9;
int maxItems = 3;
int map[HEIGHT][WIDTH];
int score = 0;
int stage = 1;
int targetScore = 10;
char snakeBodyChar = 'B';

bool speedBoost = false;
bool invincible = false;
bool reverseControls = false;

std::chrono::time_point<std::chrono::high_resolution_clock> speedBoostEndTime;
std::chrono::time_point<std::chrono::high_resolution_clock> invincibleEndTime;
std::chrono::time_point<std::chrono::high_resolution_clock> reverseControlsEndTime;

struct Position {
    int x, y;
};
struct Particle {
    Position pos;
    char symbol;
    int life;
    int color;
};

std::vector<Particle> particles;
std::vector<Position> snake;

int growthItemsCollected = 0;
int poisonItemsCollected = 0;
int gateUsageCount = 0;

Position gateA, gateB;

int currentDirection = KEY_RIGHT;
bool gameStarted = false;
bool gamePaused = false;

std::map<std::string, int> config;

void loadConfig();
void initializeColors();
void initializeSnake();
void placeItems();
void placeBonusItem();
void placePowerUps();
void placeGates();
void drawMainMenu();
void drawButton(int y, int x, const char* text, bool selected);
void handleMainMenu();
void initMap();
void printMap();
void moveSnake();
void updateMapWithSnake();
void printScore();
void gameOver();
void restartGame();
void handleGateEntry(Position& newHead);
void handleItems();
void handlePowerUps();
void nextStage();
void waitForStart();
void changeSnakeBodyAppearance();
void showCongratulations();
void printTitle();
void drawGameOverMenu();
void printDynamicMessage();
void smoothTransition(int);
void drawGlowingBorders();
void drawCustomAsciiTitle();
void drawCustomAsciiScoreboard();
bool checkMissionCompletion();
void handleCollision(Position& newHead);
void handleItemCollection(Position& newHead);
void handlePowerUpEffects(Position& newHead);
void updateSnakePosition(Position& newHead);
bool checkCollision(const Position& pos);
void gameLoop();
void togglePause();

void loadConfig() {
    std::ifstream configFile("config.txt");
    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        int value;
        if (std::getline(iss, key, '=') && iss >> value) {
            config[key] = value;
        }
    }
    DELAY = config["DELAY"];
    itemDisappearTime = config["ITEM_DISAPPEAR_TIME"];
    maxItems = config["MAX_ITEMS"];
}

void createParticle(int x, int y, char symbol, int life, int color) {
    particles.push_back({{x, y}, symbol, life, color});
}

void updateParticles() {
    for (auto it = particles.begin(); it != particles.end();) {
        it->life--;
        if (it->life <= 0) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}
void drawMainMenu() {
    clear();
    drawCustomAsciiTitle();
    int startX = (WIDTH + 5) / 2 - 6;
    drawButton(10, startX, "START GAME", true);
    drawButton(12, startX, "EXIT", false);
    refresh();
}
void handleMainMenu() {
    int selected = 0;
    int ch;
    drawMainMenu();
    while ((ch = getch()) != '\n') {
        switch (ch) {
            case KEY_UP:
                selected = (selected + 1) % 2;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % 2;
                break;
            default:
                break;
        }
        drawButton(10, (WIDTH + 5) / 2 - 6, "START GAME", selected == 0);
        drawButton(12, (WIDTH + 5) / 2 - 6, "EXIT", selected == 1);
        refresh();
    }
    if (selected == 0) {
        restartGame();
    } else {
        endwin();
        exit(0);
    }
}

void drawParticles() {
    for (const auto& particle : particles) {
        attron(COLOR_PAIR(particle.color));
        mvaddch(particle.pos.y, particle.pos.x, particle.symbol);
        attroff(COLOR_PAIR(particle.color));
    }
}

void drawCuteAsciiArt() {
    const char* asciiArt[] = {
            " /\\_/\\  ",
            "( o.o ) ",
            " > ^ <  ",
            "Bunny"
    };

    int artStartY = 20;
    int artStartX = WIDTH + 4;

    attron(COLOR_PAIR(5) | A_BOLD);
    for (int i = 0; i < 4; i++) {
        mvprintw(artStartY + i, artStartX, asciiArt[i]);
    }
    attroff(COLOR_PAIR(5) | A_BOLD);
}

void initializeSnake() {
    int startX = 10;
    int startY = 10;

    snake.clear();
    for (int i = 0; i < 4; i++) {
        snake.push_back({startX, startY + i});
        map[startY + i][startX] = (i == 0 ? 3 : 4);
    }
}

void placeItems() {
    int itemsPlaced = 0;
    while (itemsPlaced < maxItems) {
        int x = rand() % WIDTH;
        int y = rand() % HEIGHT;
        if (map[y][x] == 0) {
            map[y][x] = (rand() % 2 == 0) ? 5 : 6;
            itemsPlaced++;
        }
    }
}

void placeBonusItem() {
    while (true) {
        int x = rand() % WIDTH;
        int y = rand() % HEIGHT;
        if (map[y][x] == 0) {
            map[y][x] = 9;
            break;
        }
    }
}

void placePowerUps() {
    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;
    if (map[y][x] == 0) {
        map[y][x] = 10;
    }

    x = rand() % WIDTH;
    y = rand() % HEIGHT;
    if (map[y][x] == 0) {
        map[y][x] = 11;
    }
}

void placeGates() {
    gateA = {0, HEIGHT / 2};
    gateB = {WIDTH - 1, HEIGHT / 2};
    map[gateA.y][gateA.x] = 7;
    map[gateB.y][gateB.x] = 8;
}

void initMap() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (i == 0 || i == HEIGHT - 1 || j == 0 || j == WIDTH - 1) {
                map[i][j] = 1;
            } else {
                map[i][j] = (rand() % 10 == 0) ? 2 : 0;
            }
        }
    }

    initializeSnake();
    placeItems();
    placeBonusItem();
    placeGates();
    placePowerUps();
}
void printTitle() {
    const char* title = "SNAKE GAME";
    int titleLength = strlen(title);
    int centerX = WIDTH + (40 - titleLength) / 2;

    drawCustomAsciiTitle();

    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(3, centerX, title);
    mvprintw(4, centerX, "============");
    attroff(COLOR_PAIR(5) | A_BOLD);
}

void drawCustomAsciiTitle() {
    const char* asciiTitle[] = {
            "  ____              _        ",
            " / ___| _ __   __ _| | _____ ",
            " \\___ \\| '_ \\ / _` | |/ / _ \\",
            "  ___) | | | | (_| |   <  __/",
            " |____/|_| |_|\\__,_|_|\\_\\___|"
    };

    int titleStartY = 1;
    int titleStartX = WIDTH + 5;

    attron(COLOR_PAIR(5) | A_BOLD);
    for (int i = 0; i < 5; i++) {
        mvprintw(titleStartY + i, titleStartX, asciiTitle[i]);
        mvprintw(titleStartY + i + 1, titleStartX + 1, asciiTitle[i]);
    }
    attroff(COLOR_PAIR(5) | A_BOLD);
}

void drawButton(int y, int x, const char* text, bool selected) {
    if (selected) {
        attron(C

        OLOR_PAIR(4) | A_BOLD);
    } else {
        attron(COLOR_PAIR(2));
    }
    mvprintw(y, x, "[ %s ]", text);
    if (selected) {
        attroff(COLOR_PAIR(4) | A_BOLD);
    } else {
        attroff(COLOR_PAIR(2));
    }
}

void initializeColors() {
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_BLACK);
        init_pair(2, COLOR_YELLOW, COLOR_BLACK);
        init_pair(3, COLOR_RED, COLOR_BLACK);
        init_pair(4, COLOR_GREEN, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_WHITE, COLOR_BLACK);
        init_pair(7, COLOR_RED, COLOR_BLACK);
        init_pair(8, COLOR_BLUE, COLOR_BLACK);
        init_pair(9, COLOR_WHITE, COLOR_BLUE);
        init_pair(10, COLOR_WHITE, COLOR_GREEN);
        init_pair(11, COLOR_WHITE, COLOR_RED);
        init_pair(12, COLOR_YELLOW, COLOR_CYAN);
        init_pair(13, COLOR_WHITE, COLOR_MAGENTA);
    }
}

void drawCustomAsciiScoreboard() {
    const char* asciiScoreboard[] = {
            "  _____                     _ ",
            " / ____|                   | |",
            "| (___   ___  __ _ _ __ ___| |",
            " \\___ \\ / _ \\/ _` | '__/ _ \\ |",
            " ____) |  __/ (_| | | |  __/ |",
            "|_____/ \\___|\\__,_|_|  \\___|_|"
    };

    int scoreStartY = 2;
    int scoreStartX = WIDTH + 4;

    attron(COLOR_PAIR(1) | A_BOLD);
    for (int i = 0; i < 6; i++) {
        mvprintw(scoreStartY + i, scoreStartX, asciiScoreboard[i]);
        mvprintw(scoreStartY + i + 1, scoreStartX + 1, asciiScoreboard[i]);
    }
    attroff(COLOR_PAIR(1) | A_BOLD);
}

void printDynamicMessage() {
    if (score >= 100 && stage == 1) {
        attron(COLOR_PAIR(1) | A_BLINK);
        mvprintw(HEIGHT + 11, 0, "Awesome! You reached 100 points!");
        attroff(COLOR_PAIR(1) | A_BLINK);
    } else if (score >= 150 && stage == 2) {
        attron(COLOR_PAIR(2) | A_BLINK);
        mvprintw(HEIGHT + 11, 0, "Great job! You reached 150 points!");
        attroff(COLOR_PAIR(2) | A_BLINK);
    } else if (score >= 200 && stage == 3) {
        attron(COLOR_PAIR(3) | A_BLINK);
        mvprintw(HEIGHT + 11, 0, "You are a true Snake Master!");
        attroff(COLOR_PAIR(3) | A_BLINK);
    }
    refresh();
}
void drawGlowingBorders() {
    static bool glow = false;
    static int colorIndex = 0;
    int borderColors[] = {1, 2, 3, 4, 5, 6};

    for (int i = 0; i < WIDTH; i++) {
        attron(COLOR_PAIR(borderColors[colorIndex % 6]));
        mvaddch(0, i, '#');
        mvaddch(HEIGHT - 1, i, '#');
        attroff(COLOR_PAIR(borderColors[colorIndex % 6]));
        colorIndex++;
    }

    for (int i = 0; i < HEIGHT; i++) {
        attron(COLOR_PAIR(borderColors[colorIndex % 6]));
        mvaddch(i, 0, '#');
        mvaddch(i, WIDTH - 1, '#');
        attroff(COLOR_PAIR(borderColors[colorIndex % 6]));
        colorIndex++;
    }

    attron(COLOR_PAIR(1));
    mvaddch(gateA.y, gateA.x, 'G');
    mvaddch(gateB.y, gateB.x, 'G');
    attroff(COLOR_PAIR(1));

    glow = !glow;
}

bool checkMissionCompletion() {
    if (stage == 1) {
        return gateUsageCount >= 1 && growthItemsCollected >= 3 && poisonItemsCollected <= 2;
    } else if (stage == 2) {
        return gateUsageCount >= 1 && growthItemsCollected >= 5 && poisonItemsCollected <= 2;
    } else if (stage == 3) {
        return gateUsageCount >= 1 && growthItemsCollected >= 7 && poisonItemsCollected <= 2;
    }
    return false;
}
bool toggleAnimationState() {
    static bool toggle = false;
    toggle = !toggle;
    return toggle;
}

void drawDecorations() {
    attron(COLOR_PAIR(5));
    mvprintw(0, 0, "*");
    mvprintw(0, WIDTH - 1, "*");
    mvprintw(HEIGHT - 1, 0, "*");
    mvprintw(HEIGHT - 1, WIDTH - 1, "*");
    attroff(COLOR_PAIR(5));

    attron(COLOR_PAIR(6));
    mvprintw(HEIGHT / 2, 0, "<3");
    mvprintw(HEIGHT / 2, WIDTH - 2, "<3");
    attroff(COLOR_PAIR(6));
}
void printMap() {
    clear();
    printTitle();
    drawGlowingBorders();
    drawDecorations();

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            char displayChar = ' ';
            switch(map[i][j]) {
                case 1:
                    attron(COLOR_PAIR(1));
                    displayChar = '#';
                    break;
                case 3:
                    attron(COLOR_PAIR(12));
                    displayChar = 'H';
                    break;
                case 4:
                    displayChar = snakeBodyChar;
                    break;
                case 5:
                    attron(COLOR_PAIR(2));
                    displayChar = '+';
                    break;
                case 6:
                    attron(COLOR_PAIR(3));
                    displayChar = '-';
                    break;
                case 7: displayChar = 'G'; break;
                case 8: displayChar = 'G'; break;
                case 9:
                    attron(COLOR_PAIR(4));
                    displayChar = '*';
                    break;
                case 10:
                    attron(COLOR_PAIR(9));
                    displayChar = 'S';
                    break;
                case 11:
                    attron(COLOR_PAIR(10));
                    displayChar = 'M';
                    break;
            }
            mvaddch(i, j, displayChar);
            attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(4) | COLOR_PAIR(9) | COLOR_PAIR(10) | COLOR_PAIR(11) | COLOR_PAIR(12) | COLOR_PAIR(13));
        }
    }

    drawParticles();

    printScore();

    int scoreStartX = WIDTH + 4;
    int scoreStartY = 2 + 8;

    int maxLength = (stage == 1) ? 7 : (stage == 2) ? 12 : 17;
    bool missionCompleted = checkMissionCompletion();
    mvprintw(scoreStartY + 1, scoreStartX, "Mission - Length: %d (%s)", maxLength, missionCompleted ? "Completed" : "Active");

    if (stage == 1) {
        mvprintw(scoreStartY + 2, scoreStartX, "1. Use Gate at least 1 time");
        mvprintw(scoreStartY + 3, scoreStartX, "2. Eat growth item 3 times");
        mvprintw(scoreStartY + 4, scoreStartX, "3. Limit poison item 3 times");
    } else if (stage == 2) {
        mvprintw(scoreStartY + 2, scoreStartX, "1. Use Gate at least 1 time");
        mvprintw(scoreStartY + 3, scoreStartX, "2. Eat growth item 5 times");
        mvprintw(scoreStartY + 4, scoreStartX, "3. Limit poison item 2 times");
    } else if (stage == 3) {
        mvprintw(scoreStartY + 2, scoreStartX, "1. Use Gate at least 1 time");
        mvprintw(scoreStartY + 3, scoreStartX, "2. Eat growth item 7 times");
        mvprintw(scoreStartY + 4, scoreStartX, "3. Limit poison item 2 times");
    }

    printDynamicMessage();

    refresh();
}

void moveSnake() {
    Position newHead = snake.front();
    switch (currentDirection) {
        case KEY_LEFT:
            newHead.x--;
            break;
        case KEY_RIGHT:
            newHead.x++;
            break;
        case KEY_UP:
            newHead.y--;
            break;
        case KEY_DOWN:
            newHead.y++;
            break;
    }

    handleCollision(newHead);
    handleItemCollection(newHead);
    handlePowerUpEffects(newHead);
    updateSnakePosition(newHead);

    createParticle(newHead.x, newHead.y, '*',

                   5, 8);

    updateMapWithSnake();
    printMap();

    if (score < 0 || snake.size() < 3) {
        gameOver();
    }

    if (checkMissionCompletion()) {
        nextStage();
    }
}
void handleGateEntry(Position& newHead) {
    Position currentGate = newHead;
    Position targetGate = (map[newHead.y][newHead.x] == 7) ? gateB : gateA;

    if (targetGate.y == 0) {
        newHead = {targetGate.x, targetGate.y + 1};
    } else if (targetGate.y == HEIGHT - 1) {
        newHead = {targetGate.x, targetGate.y - 1};
    } else if (targetGate.x == 0) {
        newHead = {targetGate.x + 1, targetGate.y};
    } else if (targetGate.x == WIDTH - 1) {
        newHead = {targetGate.x - 1, targetGate.y};
    } else {
        newHead = targetGate;
    }

    createParticle(currentGate.x, currentGate.y, 'G', 10, 1);
}

void handleCollision(Position& newHead) {
    if (checkCollision(newHead) && !invincible) {
        gameOver();
        return;
    }
    if (map[newHead.y][newHead.x] == 7 || map[newHead.y][newHead.x] == 8) {
        handleGateEntry(newHead);
        gateUsageCount++;
        playSoundEffect("use_gate.wav");
    }
}

bool checkCollision(const Position& pos) {
    if (pos.x < 0 || pos.x >= WIDTH || pos.y < 0 || pos.y >= HEIGHT || map[pos.y][pos.x] == 1) {
        return true;
    }
    for (const auto& segment : snake) {
        if (segment.x == pos.x && segment.y == pos.y) {
            return true;
        }
    }
    return false;
}
void handleItemCollection(Position& newHead) {
    if (map[newHead.y][newHead.x] == 5) {
        growthItemsCollected++;
        score += 10;
        playSoundEffect("eat_item.wav");
        placeItems();
        createParticle(newHead.x, newHead.y, '+', 10, 2);
    } else if (map[newHead.y][newHead.x] == 6) {
        if (snake.size() > 3) {
            poisonItemsCollected++;
            score -= 10;
            map[newHead.y][newHead.x] = 0;
            map[snake.back().y][snake.back().x] = 0;
            snake.pop_back();
            playSoundEffect("eat_poison.wav");
            placeItems();
            createParticle(newHead.x, newHead.y, '-', 10, 3);
        } else {
            gameOver();
            return;
        }
    } else if (map[newHead.y][newHead.x] == 9) {
        score += 20;
        playSoundEffect("bonus_item.wav");
        placeBonusItem();
        changeSnakeBodyAppearance();
        createParticle(newHead.x, newHead.y, '*', 10, 4);
    } else if (map[newHead.y][newHead.x] == 10) {
        speedBoost = true;
        speedBoostEndTime = std::chrono::high_resolution_clock::now() + std::chrono::seconds(5);
        DELAY = 200000;
        playSoundEffect("power_up.wav");
        createParticle(newHead.x, newHead.y, 'S', 10, 9);
    } else if (map[newHead.y][newHead.x] == 11) {
        score += 50;
        playSoundEffect("power_up.wav");
        placePowerUps();
        createParticle(newHead.x, newHead.y, 'M', 10, 10);
    }
}

void handlePowerUpEffects(Position& newHead) {
    if (map[newHead.y][newHead.x] == 10) {
        speedBoost = true;
        speedBoostEndTime = std::chrono::high_resolution_clock::now() + std::chrono::seconds(5);
        DELAY /= 2;
        playSoundEffect("power_up.wav");
    } else if (map[newHead.y][newHead.x] == 11) {
        invincible = true;
        invincibleEndTime = std::chrono::high_resolution_clock::now() + std::chrono::seconds(5);
        playSoundEffect("power_up.wav");
    } else if (map[newHead.y][newHead.x] == 12) {
        reverseControls = true;
        reverseControlsEndTime = std::chrono::high_resolution_clock::now() + std::chrono::seconds(5);
        playSoundEffect("power_up.wav");
    }
}

void updateSnakePosition(Position& newHead) {
    snake.insert(snake.begin(), newHead);
    if (map[newHead.y][newHead.x] != 5 && map[newHead.y][newHead.x] != 9) {
        map[snake.back().y][snake.back().x] = 0;
        snake.pop_back();
    }
}

void updateMapWithSnake() {
    bool animationState = toggleAnimationState();
    for (size_t i = 0; i < snake.size(); i++) {
        int colorPair = (i == 0) ? 12 : (animationState ? 8 : 13);
        attron(COLOR_PAIR(colorPair));
        map[snake[i].y][snake[i].x] = (i == 0) ? 3 : 4;
        attroff(COLOR_PAIR(colorPair));
    }
}

void printScore() {
    drawCustomAsciiScoreboard();

    int scoreStartX = WIDTH + 4;
    int scoreStartY = 10;

    attron(COLOR_PAIR(1));
    mvprintw(scoreStartY, scoreStartX, "=======================");
    mvprintw(scoreStartY + 1, scoreStartX, "||   SCOREBOARD     ||");
    mvprintw(scoreStartY + 2, scoreStartX, "=======================");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(2));
    mvprintw(scoreStartY + 3, scoreStartX, "Score: %d", score);
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(3));
    mvprintw(scoreStartY + 4, scoreStartX, "Length: %lu", snake.size());
    attroff(COLOR_PAIR(3));

    attron(COLOR_PAIR(4));
    mvprintw(scoreStartY + 5, scoreStartX, "Max Length: %d", 10);
    mvprintw(scoreStartY + 6, scoreStartX, "+: %d, -: %d, G: %d", growthItemsCollected, poisonItemsCollected, gateUsageCount);
    attroff(COLOR_PAIR(4));

    attron(COLOR_PAIR(1));
    mvprintw(scoreStartY + 7, scoreStartX, "=======================");
    attroff(COLOR_PAIR(1));

    drawCuteAsciiArt();

    refresh();
}

void drawGameOverMenu(int selected) {
    clear();
    mvprintw(HEIGHT / 2 - 2, (WIDTH - 9) / 2, "GAME OVER");

    drawButton(HEIGHT / 2, (WIDTH - 9) / 2, "RESTART", selected == 0);
    drawButton(HEIGHT / 2 + 2, (WIDTH - 9) / 2, "EXIT", selected == 1);

    refresh();
}

void gameOver() {
    playSoundEffect("game_over.wav");

    int selected = 0;
    int ch;
    drawGameOverMenu(selected);

    while (true) {
        ch = getch();
        switch (ch) {
            case KEY_UP:
                selected = (selected + 1) % 2;
                break;
            case KEY_DOWN:
                selected = (selected + 1) % 2;
                break;
            case '\n':
                if (selected == 0) {
                    restartGame();
                    return;
                } else if (selected == 1) {
                    endwin();
                    exit(0);
                }
                break;
            default:
                break;
        }
        drawGameOverMenu(selected);
    }
}

void restartGame() {
    clear();

    score = 0;
    growthItemsCollected = 0;
    poisonItemsCollected = 0;
    gateUsageCount = 0;
    currentDirection = KEY_RIGHT;
    gameStarted = false;
    gamePaused = false;
    stage = 1;
    targetScore = 10;
    DELAY = 300000;
    itemDisappearTime = 7;
    maxItems = 3;
    snakeBodyChar = 'B';
    speedBoost = false;
    invincible = false;
    reverseControls = false;

    particles.clear();

    initMap();
    initializeSnake();
    placeItems();
    placeBonusItem();
    placeGates();
    placePowerUps();
    printMap();

    waitForStart();

    nodelay(stdscr, TRUE);
}

void handleItems() {
    static int itemCounter = 0;
    itemCounter++;
    if (itemCounter >= itemDisappearTime * 2) {
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (map[i][j] == 5 || map[i][j] == 6 || map[i][

                                                                j] == 9) {
                    map[i][j] = 0;
                }
            }
        }
        placeItems();
        placeBonusItem();
        itemCounter = 0;
    }
}

void handlePowerUps() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    if (speedBoost && currentTime > speedBoostEndTime) {
        speedBoost = false;
        DELAY *= 2;
    }
    if (invincible && currentTime > invincibleEndTime) {
        invincible = false;
    }
    if (reverseControls && currentTime > reverseControlsEndTime) {
        reverseControls = false;
    }
}
void drawCongratulatoryAnimation() {
    const char* cuteCharacter[] = {
            "  \\  |  / ",
            "   .-'-.   ",
            " --|   |-- ",
            "   `-O-'   ",
            "  / _|_ \\  "
    };

    const char* confetti[] = {
            " *  .  *  .",
            " .  *  .  *",
            " *  .  *  .",
            " .  *  .  *"
    };

    int startY = HEIGHT / 2 - 2;
    int startX = WIDTH / 2 - 5;

    for (int i = 0; i < 5; ++i) {
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(startY + i, startX, cuteCharacter[i]);
        mvprintw(startY + i + 1, startX + 1, cuteCharacter[i]);
        attroff(COLOR_PAIR(5) | A_BOLD);
    }

    for (int i = 0; i < 4; ++i) {
        attron(COLOR_PAIR(4) | A_BOLD | A_BLINK);
        mvprintw(startY - 4 + i, startX + 10, confetti[i]);
        mvprintw(startY + 6 + i, startX - 10, confetti[i]);
        attroff(COLOR_PAIR(4) | A_BOLD | A_BLINK);
    }

    refresh();
    usleep(500000);
}

void nextStage() {
    if (stage >= 3) {
        showCongratulations();
    } else {
        mvprintw(HEIGHT + 4, 0, "Stage %d Complete! Press enter to continue.", stage);
        playSoundEffect("stage_complete.wav");
        refresh();
        nodelay(stdscr, FALSE);
        while (getch() != '\n');

        stage++;
        if (stage == 2) {
            targetScore = 100;
            DELAY = 200000;
            itemDisappearTime = 5;
        } else if (stage == 3) {
            targetScore = 150;
            DELAY = 200000;
            itemDisappearTime = 5;
            maxItems = 5;
        }
        score = 0;
        growthItemsCollected = 0;
        poisonItemsCollected = 0;
        gateUsageCount = 0;
        gameStarted = false;
        initMap();
        smoothTransition(3);
        printMap();
        waitForStart();
    }
}

void showCongratulations() {
    clear();
    const char* messages[] = {
            "GOOD JOB! YOU ARE A SNAKE MASTER NOW",
            "Game by: Anu",
            "Press 'r' to restart the game"
    };
    int numMessages = sizeof(messages) / sizeof(messages[0]);

    initializeColors();

    for (int j = 0; j < numMessages; ++j) {
        attron(COLOR_PAIR(j + 1) | A_BOLD);
        mvprintw((HEIGHT / 2) + j, WIDTH + 2, messages[j]);
        attroff(COLOR_PAIR(j + 1) | A_BOLD);
    }
    refresh();

    playSoundEffect("congratulations.wav");
    drawCongratulatoryAnimation();

    nodelay(stdscr, FALSE);
    int ch = getch();
    if (ch == 'r') {
        restartGame();
    } else {
        endwin();
        exit(0);
    }
}

void waitForStart() {
    mvprintw(HEIGHT + 5, 0, "Press any arrow key to start");
    refresh();
    nodelay(stdscr, TRUE);
    int ch;
    while ((ch = getch()) != KEY_F(1)) {
        if (ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_DOWN) {
            currentDirection = ch;
            gameStarted = true;
            playBackgroundMusic();
            break;
        }
    }
}

void changeSnakeBodyAppearance() {
    char bodyChars[] = {'@', 'x', 'O', '+', '#'};
    snakeBodyChar = bodyChars[rand() % 5];
}

void smoothTransition(int steps) {
    for (int i = 0; i < steps; ++i) {
        clear();
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(HEIGHT / 2, (WIDTH / 2) - 5, "Stage %d", stage);
        attroff(COLOR_PAIR(1) | A_BOLD);
        refresh();
        usleep(500000);
    }
}

void togglePause() {
    gamePaused = !gamePaused;
    if (gamePaused) {
        mvprintw(HEIGHT + 5, 0, "Game Paused. Press 'p' to resume.");
    } else {
        mvprintw(HEIGHT + 5, 0, "Press 'p' to pause the game.");
    }
    refresh();
}

void initializeSound() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    } else {
        printf("SDL initialized successfully.\n");
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "Failed to initialize SDL_mixer: %s\n", Mix_GetError());
        SDL_Quit();
        exit(1);
    } else {
        printf("SDL_mixer initialized successfully.\n");
    }
}

void playBackgroundMusic() {
    printf("Attempting to play background music.\n");
    Mix_Music* music = Mix_LoadMUS("C:/Users/anubn/CLionProjects/SnakeGame/src/background_music.mp3");
    if (!music) {
        fprintf(stderr, "Failed to load background music: %s\n", Mix_GetError());
        return;
    }
    printf("Background music loaded successfully.\n");
    if (Mix_PlayMusic(music, -1) == -1) {
        fprintf(stderr, "Failed to play background music: %s\n", Mix_GetError());
        Mix_FreeMusic(music);
        return;
    }
    printf("Background music playing.\n");
}

void playSoundEffect(const char* filename) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "C:/Users/anubn/CLionProjects/SnakeGame/src/%s", filename);
    printf("Attempting to play sound effect: %s\n", filepath);
    Mix_Chunk* sound = Mix_LoadWAV(filepath);
    if (!sound) {
        fprintf(stderr, "Failed to load sound effect %s: %s\n", filepath, Mix_GetError());
        return;
    }
    printf("Sound effect %s loaded successfully.\n", filepath);
    if (Mix_PlayChannel(-1, sound, 0) == -1) {
        fprintf(stderr, "Failed to play sound effect %s: %s\n", filepath, Mix_GetError());
        Mix_FreeChunk(sound);
        return;
    }
    printf("Sound effect %s playing.\n", filepath);
}

void gameLoop() {
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    int ch;
    while ((ch = getch()) != KEY_F(1)) {
        if (!gameStarted) {
            if (ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_DOWN) {
                gameStarted = true;
                currentDirection = ch;
                playBackgroundMusic();
            }
        } else if (gamePaused) {
            if (ch == 'p') {
                togglePause();
            }
        } else {
            switch (ch) {
                case KEY_LEFT:
                    if (reverseControls) {
                        if (currentDirection != KEY_RIGHT) currentDirection = KEY_RIGHT;
                    } else {
                        if (currentDirection != KEY_RIGHT) currentDirection = ch;
                    }
                    break;
                case KEY_RIGHT:
                    if (reverseControls) {
                        if (currentDirection != KEY_LEFT) currentDirection = KEY_LEFT;
                    } else {
                        if (currentDirection != KEY_LEFT) currentDirection = ch;
                    }
                    break;
                case KEY_UP:
                    if (reverseControls) {
                        if (currentDirection != KEY_DOWN) currentDirection = KEY_DOWN;
                    } else {
                        if (currentDirection != KEY_DOWN) currentDirection = ch;
                    }
                    break;
                case KEY_DOWN:
                    if (reverseControls) {
                        if (currentDirection != KEY_UP) currentDirection = KEY_UP;
                    } else {
                        if (currentDirection != KEY_UP) currentDirection = ch;
                    }
                    break;
                case 'p':
                    togglePause();
                    break;
                default:
                    break;
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            std

            ::chrono::duration<float> elapsed = currentTime - lastUpdateTime;
            if (elapsed.count() >= DELAY / 1e6) {
                moveSnake();
                handleItems();
                handlePowerUps();
                updateParticles();
                printScore();
                drawGlowingBorders();
                lastUpdateTime = currentTime;
            }
            refresh();
        }
    }
}

int main() {
    srand(time(NULL));
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    loadConfig();
    initializeColors();
    initializeSound();

    handleMainMenu();

    initMap();
    printMap();
    waitForStart();

    nodelay(stdscr, TRUE);

    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    int ch;
    while ((ch = getch()) != KEY_F(1)) {
        if (!gameStarted) {
            if (ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_DOWN) {
                gameStarted = true;
                currentDirection = ch;
                playBackgroundMusic();
            }
        } else if (gamePaused) {
            if (ch == 'p') {
                togglePause();
            }
        } else {
            switch (ch) {
                case KEY_LEFT:
                    if (reverseControls) {
                        if (currentDirection != KEY_RIGHT) currentDirection = KEY_RIGHT;
                    } else {
                        if (currentDirection != KEY_RIGHT) currentDirection = ch;
                    }
                    break;
                case KEY_RIGHT:
                    if (reverseControls) {
                        if (currentDirection != KEY_LEFT) currentDirection = KEY_LEFT;
                    } else {
                        if (currentDirection != KEY_LEFT) currentDirection = ch;
                    }
                    break;
                case KEY_UP:
                    if (reverseControls) {
                        if (currentDirection != KEY_DOWN) currentDirection = KEY_DOWN;
                    } else {
                        if (currentDirection != KEY_DOWN) currentDirection = ch;
                    }
                    break;
                case KEY_DOWN:
                    if (reverseControls) {
                        if (currentDirection != KEY_UP) currentDirection = KEY_UP;
                    } else {
                        if (currentDirection != KEY_UP) currentDirection = ch;
                    }
                    break;
                case 'p':
                    togglePause();
                    break;
                default:
                    break;
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = currentTime - lastUpdateTime;
            if (elapsed.count() >= DELAY / 1e6) {
                moveSnake();
                handleItems();
                handlePowerUps();
                updateParticles();
                printScore();
                drawGlowingBorders();
                lastUpdateTime = currentTime;
            }
            refresh();
        }
    }

    endwin();
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}
```cpp
void gameLoop() {
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    int ch;
    while ((ch = getch()) != KEY_F(1)) {
        if (!gameStarted) {
            if (ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_DOWN) {
                gameStarted = true;
                currentDirection = ch;
                playBackgroundMusic();
            }
        } else if (gamePaused) {
            if (ch == 'p') {
                togglePause();
            }
        } else {
            switch (ch) {
                case KEY_LEFT:
                    if (reverseControls) {
                        if (currentDirection != KEY_RIGHT) currentDirection = KEY_RIGHT;
                    } else {
                        if (currentDirection != KEY_RIGHT) currentDirection = ch;
                    }
                    break;
                case KEY_RIGHT:
                    if (reverseControls) {
                        if (currentDirection != KEY_LEFT) currentDirection = KEY_LEFT;
                    } else {
                        if (currentDirection != KEY_LEFT) currentDirection = ch;
                    }
                    break;
                case KEY_UP:
                    if (reverseControls) {
                        if (currentDirection != KEY_DOWN) currentDirection = KEY_DOWN;
                    } else {
                        if (currentDirection != KEY_DOWN) currentDirection = ch;
                    }
                    break;
                case KEY_DOWN:
                    if (reverseControls) {
                        if (currentDirection != KEY_UP) currentDirection = KEY_UP;
                    } else {
                        if (currentDirection != KEY_UP) currentDirection = ch;
                    }
                    break;
                case 'p':
                    togglePause();
                    break;
                default:
                    break;
            }

            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<float> elapsed = currentTime - lastUpdateTime;
            if (elapsed.count() >= DELAY / 1e6) {
                moveSnake();
                handleItems();
                handlePowerUps();
                updateParticles();
                printScore();
                drawGlowingBorders();
                lastUpdateTime = currentTime;
            }
            refresh();
        }
    }
}

int main() {
    srand(time(NULL));
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    loadConfig();
    initializeColors();
    initializeSound();

    handleMainMenu();

    initMap();
    printMap();
    waitForStart();

    nodelay(stdscr, TRUE);

    gameLoop();

    endwin();
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}
