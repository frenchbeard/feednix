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

CursesProvider::CursesProvider(const map<string, string> *categories){
        initscr();
        start_color();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        labels = categories;
}

void CursesProvider::init(){
        int ch;

        /* Initialize all the colors */
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);

        //init_wins(main_wins, 3);
        create_menu();
        main_wins[1] = newwin(LINES-2, 0, 0, 40);
        win_show(main_wins[1], "Categories", 1);

        /* Attach a panel to each window */     /* Order is bottom up */
        main_pans[0] = new_panel(main_wins[0]);   /* Push 0, order: stdscr-0 */
        main_pans[1] = new_panel(main_wins[1]);   /* Push 1, order: stdscr-0-1 */
        //main_pans[2] = new_panel(main_wins[2]);   /* Push 2, order: stdscr-0-1-2 */
        //main_pans[3] = new_panel(main_wins[3]);

        /* Set up the user pointers to the next panel */
        set_panel_userptr(main_pans[0], main_pans[1]);
        set_panel_userptr(main_pans[1], main_pans[0]);
        //set_panel_userptr(main_pans[2], main_pans[3]);
        //set_panel_userptr(main_pans[3], main_pans[0]);

        /* Update the stacking order. 2nd panel will be on top */
        update_panels();

        /* Show it on the screen */
        attron(COLOR_PAIR(4));
        mvprintw(LINES - 2, 0, "Use tab to browse through the windows (F1 to Exit)");
        attroff(COLOR_PAIR(4));
        doupdate();

        top = main_pans[0];
        while((ch = getch()) != KEY_F(1))
        {       switch(ch)
                {       case 9:
                        top = (PANEL *)panel_userptr(top);
                        top_panel(top);
                        break;
                }
                switch(ch){
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
                update_panels();
                doupdate();
        }
        cleanup();
}
void CursesProvider::create_menu(){
        int n_choices, c, i = 0;

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

        main_wins[0] = newwin(LINES-2, 40, 0, 0);
        keypad(main_wins[0], TRUE);

        /* Set main window and sub window */
        set_menu_win(my_menu, main_wins[0]);
        set_menu_sub(my_menu, derwin(main_wins[0], 6, 38, 3, 1));

        /* Set menu mark to the string " * " */
        set_menu_mark(my_menu, " * ");

        /* Print a border around the main window and print a title */
        box(main_wins[0], 0, 0);
        print_in_middle(main_wins[0], 1, 0, 40, "My Menu", COLOR_PAIR(1));
        mvwaddch(main_wins[0], 2, 0, ACS_LTEE);
        mvwhline(main_wins[0], 2, 1, ACS_HLINE, 38);
        mvwaddch(main_wins[0], 2, 39, ACS_RTEE);
        mvprintw(LINES - 2, 0, "F1 to exit");
        refresh();

        post_menu(my_menu);
}
void CursesProvider::win_show(WINDOW *win, char *label, int label_color){
        int startx, starty, height, width;

        getbegyx(win, starty, startx);
        getmaxyx(win, height, width);

        box(win, 0, 0);
        mvwaddch(win, 2, 0, ACS_LTEE); 
        mvwhline(win, 2, 1, ACS_HLINE, width - 2); 
        mvwaddch(win, 2, width - 1, ACS_RTEE); 

        print_in_middle(win, 1, 0, width, label, COLOR_PAIR(label_color));
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
void CursesProvider::cleanup(){
        unpost_menu(my_menu);
        free_menu(my_menu);
        for(int i = 0; i < sizeof(my_items); ++i)
                free_item(my_items[i]);
        endwin();
}
