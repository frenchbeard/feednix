#include <iostream>
#include <curses.h>
#include <menu.h>
#include <panel.h>

#ifndef _CURSES_H
#define _CURSES_H

#include "FeedlyProvider.h"

class CursesProvider{
        public:
                CursesProvider(bool verbose, bool change);
                void init();
                void control();
        private:
                FeedlyProvider feedly;
                WINDOW *ctgWin, *postsWin;
                PANEL  *panels[2], *top;
                ITEM **ctgItems, **postsItems;
                MENU *ctgMenu, *postsMenu;
                std::string lastEntryRead;
                int totalPosts = 0, numRead = 0, numUnread = 0;
                bool currentCategoryRead;
                void createCategoriesMenu(); 
                void createPostsMenu(); 
                void ctgMenuCallback(char* label);
                void postsMenuCallback(ITEM* item, bool preview);
                void win_show(WINDOW *win, char *label, int label_color, bool highlight);
                void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color);
                void print_in_center(WINDOW *win, int starty, int startx, int height, int width, char *string, chtype color);
                void clear_updateline();
                void update_counter();
                void cleanup();
};

#endif
