#include <ncurses.h>
#include <random>
#include <unistd.h>
// #include "Map.cpp"
using namespace std;

class growthItem
{
public:
    growthItem() = default;
    growthItem(int x = 0, int y = 0) : x{x}, y{y} {};
    void move();
    void bodyIncrease();
    void print();

private:
    int x{};
    int y{};
};
class poisonItem
{
public:
    poisonItem() = default;
    poisonItem(int x = 0, int y = 0) : x{x}, y{y} {};
    void move();
    void bodyDecrease();
    void print();

private:
    int x{};
    int y{};
};
void growthItem::move()
{
    mvdelch(x, y);
    refresh();
    // Assume that there is no snake body part
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 20);
    this->x = dis(gen);
    this->y = dis(gen);
}
void growthItem::bodyIncrease()
{
    // snake body increase
}
void growthItem::print()
{
    init_pair(1, COLOR_GREEN, COLOR_GREEN);
    attron(COLOR_PAIR(1));
    mvprintw(x, y, "*");
}
void poisonItem::move()
{
    mvdelch(x, y);
    refresh();
    // Assume that there is no snake body part
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dis(0, 20);
    this->x = dis(gen);
    this->y = dis(gen);
}
void poisonItem::bodyDecrease()
{
    // snake body decrease
}
void poisonItem::print()
{
    init_pair(2, COLOR_RED, COLOR_RED);
    attron(COLOR_PAIR(2));
    mvprintw(x, y, "*");
}
void game()
{
    initscr();
    growthItem g(1, 2);
    poisonItem p(1, 2);
    keypad(stdscr, TRUE);
    noecho();
    start_color();
    int ch{};
    while (ch = getch() != KEY_UP)
    {

        g.move();
        g.print();
        p.move();
        p.print();
        refresh();
        sleep(5);
    }
    refresh();
    getch();
    // to change
    endwin();
}

int main()
{
    game();
    return 0;
}