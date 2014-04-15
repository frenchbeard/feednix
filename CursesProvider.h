#include <iostream>
#include <curses.h>
#include <menu.h>
#include <panel.h>

using namespace std;

#ifndef _CURSES_H
#define _CURSES_H

class CursesProvider{
        public:
                CursesProvider(const map<string, string> *categories);
                void init();
        private:
                const map<string, string> *labels;
                WINDOW *main_wins[2];
                PANEL  *main_pans[2];
                PANEL  *top;
                ITEM **my_items;
                MENU *my_menu;
                void create_menu(); 
                void win_show(WINDOW *win, char *label, int label_color);
                void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
                void cleanup();
};

#endif
