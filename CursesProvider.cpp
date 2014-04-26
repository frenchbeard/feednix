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
        feedly.authenticateUser("jorgemartinezhernandez", "trueforerunner117");

        initscr();
        start_color();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);

        labels = feedly.getLabels();
}
void CursesProvider::init(){
        int ch;

        /* Initialize all the colors */
        init_pair(1, COLOR_RED, 0);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);

        create_categories_menu();
        create_posts_menu();

        /* Attach a panel to each window */     /* Order is bottom up */
        main_pans[0] = new_panel(ctg_win);   /* Push 0, order: stdscr-0 */
        main_pans[1] = new_panel(posts_win);   /* Push 1, order: stdscr-0-1 */

        /* Set up the user pointers to the next panel */
        set_panel_userptr(main_pans[0], main_pans[1]);
        set_panel_userptr(main_pans[1], main_pans[0]);

        update_panels();

        attron(COLOR_PAIR(4));
        mvprintw(LINES - 2, 0, "Use tab to browse through the windows (F1 to Exit)");
        attroff(COLOR_PAIR(4));
        doupdate();

        top = main_pans[1];
        top_panel(top);
        MENU* cur_menu = posts_menu;

        while((ch = getch()) != KEY_F(1)){
                switch(ch){
                        case 9:
                                if(cur_menu == ctg_menu)
                                        cur_menu = posts_menu;
                                else
                                        cur_menu = ctg_menu;
                                top = (PANEL *)panel_userptr(top);
                                top_panel(top);
                                break;
                        case KEY_DOWN:
                                menu_driver(cur_menu, REQ_DOWN_ITEM);
                                break;
                        case KEY_UP:
                                menu_driver(cur_menu, REQ_UP_ITEM);
                                break;
                        case 'j':
                                menu_driver(cur_menu, REQ_DOWN_ITEM);
                                break;
                        case 'k':
                                menu_driver(cur_menu, REQ_UP_ITEM);
                                break;
                        case 10: /* Enter */
                                if(cur_menu == ctg_menu){
                                        wclear(posts_win);
                                        ctg_menu_callback(strdup(item_name(current_item(cur_menu))));

                                        top = (PANEL *)panel_userptr(top);
                                        top_panel(top);
                                        cur_menu = posts_menu;

                                        win_show(posts_win, strdup("Posts"), 1);
                                }
                                else if(panel_window(top) == posts_win){
                                        ITEM* curItem = current_item(cur_menu);
                                        postsMenuCallback(curItem, item_description(curItem));
                                }
                                break;
                }
                update_panels();
                doupdate();
        }
        cleanup();
}
void CursesProvider::create_categories_menu(){
        int n_choices, c, i = 2;

        /* Create items */
        n_choices = labels->size() + 2;
        ctg_items = (ITEM **)calloc(sizeof(std::map<string, string>::value_type)*n_choices, sizeof(ITEM *));

        ctg_items[0] = new_item("All", "");
        ctg_items[1] = new_item("Saved", "");

        for(std::map<string, string>::const_iterator it = labels->begin(); it != labels->end(); ++it){
                ctg_items[i] = new_item((it->first).c_str(), (it->second).c_str());
                i++;
        }

        ctg_menu = new_menu((ITEM **)ctg_items);

        ctg_win = newwin(LINES-2, 40, 0, 0);
        keypad(ctg_win, TRUE);

        set_menu_win(ctg_menu, ctg_win);
        set_menu_sub(ctg_menu, derwin(ctg_win, 0, 38, 3, 1));

        set_menu_mark(ctg_menu, "  ");

        /* Print a border around the main window and print a title */
        win_show(ctg_win, strdup("Categories"),  1);

        menu_opts_off(ctg_menu, O_SHOWDESC);
        menu_opts_on(posts_menu, O_NONCYCLIC);

        post_menu(ctg_menu);
}
void CursesProvider::create_posts_menu(){
        int height, width;

        int n_choices, c, i = 0;

        const map<string, PostData>* posts = feedly.giveStreamPosts("All");

        /* Create items */
        n_choices = posts->size();
        posts_items = (ITEM **)calloc(sizeof(std::map<string, PostData>::value_type)*n_choices, sizeof(ITEM *));

        for(std::map<string, PostData>::const_iterator it = posts->begin(); it != posts->end(); ++it){
                posts_items[i] = new_item((it->second.title).c_str(), (it->first).c_str()); 
                i++;
        }
        posts_menu = new_menu((ITEM **)posts_items);

        posts_win = newwin(LINES-2, 0, 0, 40);
        keypad(posts_win, TRUE);

        getmaxyx(posts_win, height, width);

        set_menu_win(posts_menu, posts_win);
        set_menu_sub(posts_menu, derwin(posts_win, height-4, width-2, 3, 1));
        set_menu_format(posts_menu, height-4, 0);

        set_menu_fore(posts_menu, COLOR_PAIR(1) | A_REVERSE);
        set_menu_back(posts_menu, COLOR_PAIR(2));
        set_menu_grey(posts_menu, COLOR_PAIR(3));

        set_menu_mark(posts_menu, "  ");

        /* Print a border around the main window and print a title */
        win_show(posts_win, strdup("Posts"),  1);

        menu_opts_off(posts_menu, O_SHOWDESC);

        post_menu(posts_menu);
}
void CursesProvider::ctg_menu_callback(char* label){
        int height, width;

        getmaxyx(posts_win, height, width);

        int n_choices, c, i = 0;
        const map<string, PostData>* posts;

        posts = feedly.giveStreamPosts(label);

        /* Create items */
        n_choices = posts->size() + 1;
        ITEM** items = (ITEM **)calloc(sizeof(std::map<string, PostData>::value_type)*n_choices, sizeof(ITEM *));

        for(std::map<string, PostData>::const_iterator it = posts->begin(); it != posts->end(); ++it){
                items[i] = new_item((it->second.title).c_str(), (it->first).c_str()); 
                i++;
        }
        items[i] = NULL;


        unpost_menu(posts_menu);

        set_menu_items(posts_menu, items);

        post_menu(posts_menu);
        set_menu_format(posts_menu, height, 0);
}
void CursesProvider::postsMenuCallback(ITEM* item, const char* desc){
        item_opts_off(item, O_SELECTABLE);

        PostData* container = feedly.getSinglePostData(desc);

        ofstream myfile ("example.html");

        if (myfile.is_open())
                myfile << container->content;

        myfile.close();

        def_prog_mode();
        endwin();

        system("w3m example.html");

        reset_prog_mode();

        update_panels();
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
}
void CursesProvider::cleanup(){
        unpost_menu(ctg_menu);
        free_menu(ctg_menu);
        for(int i = 0; i < ARRAY_SIZE(ctg_items); ++i)
                free_item(ctg_items[i]);

        unpost_menu(posts_menu);
        free_menu(posts_menu);
        for(int i = 0; i < ARRAY_SIZE(posts_items); ++i)
                free_item(posts_items[i]);

        endwin();
        feedly.curl_cleanup();
}
