#include <ncurses.h>
#include <vector>
#include <cstdlib>
#include <ctime>

const int HEIGHT = 21;
const int WIDTH = 21;
int map[HEIGHT][WIDTH];  // Map


void initMap() {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (i == 0 || i == HEIGHT-1 || j == 0 || j == WIDTH-1) {
                map[i][j] = 1; 
            } else {
                map[i][j] = 0;
            }
        }
    }

    initializeSnake(); 
}

void printMap() {
    clear(); 
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            char displayChar = ' ';
            switch(map[i][j]) {
                case 1: displayChar = '#'; break;
                case 3: displayChar = 'H'; break;
                case 4: displayChar = 'B'; break;
                case 5: displayChar = '+'; break;
                case 6: displayChar = '-'; break;
                case 7: displayChar = 'G'; break;
            }
            mvaddch(i, j, displayChar);
        }
    }
    refresh();  // Update the screen
}

int main() {
    initMap();
    printMap();
    int ch;
    while ((ch = getch()) != KEY_F(1)) { 
        printMap();
        refresh();
    }

    endwin();
    return 0;
}
