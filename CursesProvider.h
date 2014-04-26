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
                WINDOW *ctgWin, *postsWin;
                PANEL  *panels[2], *top;
                ITEM **ctgItems, **postsItems;
                MENU *ctgMenu, *postsMenu;
                void createCategoriesMenu(); 
                void createPostsMenu(); 
                void ctgMenuCallback(char* label);
                void postsMenuCallback(ITEM* item, const char* desc);
                void win_show(WINDOW *win, char *label, int label_color);
                void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
                void cleanup();
};

#endif
