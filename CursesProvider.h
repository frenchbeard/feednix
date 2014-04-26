#include <iostream>
#include <curses.h>
#include <menu.h>
#include <panel.h>

using namespace std;

#ifndef _CURSES_H
#define _CURSES_H

#include "FeedlyProvider.h"

class CursesProvider{
        public:
                CursesProvider();
                void init();
        private:
                FeedlyProvider feedly;
                const map<string, string> *labels;
                WINDOW *ctg_win, *posts_win, *single_win, *wrapper;
                PANEL  *main_pans[4], *top;
                ITEM **ctg_items, **posts_items;
                MENU *ctg_menu, *posts_menu;
                void create_categories_menu(); 
                void create_posts_menu(); 
                void ctg_menu_callback(char* label);
                void postsMenuCallback(ITEM* item, const char* desc);
                void win_show(WINDOW *win, char *label, int label_color);
                void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
                void cleanup();
};

#endif
