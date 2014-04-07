#include <curses.h>
#include <map>
#include <panel.h>
#include <menu.h> 
#include <string.h>
#include <stdio.h>
#include <iostream>

#include "CursesProvider.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD   4

CursesProvider::CursesProvider(){
}

void CursesProvider::create_menu(const map<string, string> *labels){
        ITEM **my_items;
        int c;                          
        MENU *my_menu;
        WINDOW *my_menu_win;
        int n_choices, i = 0;

        initscr();
        start_color();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        init_pair(1, COLOR_RED, COLOR_BLACK);

        /* Create items */
        n_choices = labels->size();
        my_items = (ITEM **)calloc(sizeof(std::map<string, string>::value_type)*n_choices, sizeof(ITEM *));

        for(std::map<string, string>::const_iterator it = labels->begin(); it != labels->end(); ++it){
                my_items[i] = new_item((it->first).c_str(), "");
                i++;
        }

        my_menu = new_menu((ITEM **)my_items);

        my_menu_win = newwin(10, 40, 0, 0);
        keypad(my_menu_win, TRUE);

        /* Set main window and sub window */
        set_menu_win(my_menu, my_menu_win);
        set_menu_sub(my_menu, derwin(my_menu_win, 6, 38, 3, 1));

        /* Set menu mark to the string " * " */
        set_menu_mark(my_menu, " * ");

        /* Print a border around the main window and print a title */
        box(my_menu_win, 0, 0);
        print_in_middle(my_menu_win, 1, 0, 40, "My Menu", COLOR_PAIR(1));
        mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
        mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
        mvwaddch(my_menu_win, 2, 39, ACS_RTEE);
        mvprintw(LINES - 2, 0, "F1 to exit");
        refresh();

        post_menu(my_menu);
        wrefresh(my_menu_win);

        while((c = wgetch(my_menu_win)) != KEY_F(1)){
                switch(c){
                        case KEY_DOWN:
                                menu_driver(my_menu, REQ_DOWN_ITEM);
                                break;
                        case KEY_UP:
                                menu_driver(my_menu, REQ_UP_ITEM);
                                break;
                        case 'j':
                                menu_driver(my_menu, REQ_DOWN_ITEM);
                                break;
                        case 'k':
                                menu_driver(my_menu, REQ_UP_ITEM);
                                break;
                }
                wrefresh(my_menu_win);
        }       

        unpost_menu(my_menu);
        free_menu(my_menu);
        for(i = 0; i < n_choices; ++i)
                free_item(my_items[i]);
        endwin();
}
void CursesProvider::print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color){   

        int length, x, y;
        float temp;

        if(win == NULL)
                win = stdscr;
        getyx(win, y, x);
        if(startx != 0)
                x = startx;
        if(starty != 0)
                y = starty;
        if(width == 0)
                width = 80;

        length = strlen(string);
        temp = (width - length)/ 2;
        x = startx + (int)temp;
        wattron(win, color);
        mvwprintw(win, y, x, "%s", string);
        wattroff(win, color);
        refresh();
}
