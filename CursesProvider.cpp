#include <curses.h>
#include <map>
#include <panel.h>
#include <menu.h> 
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "CursesProvider.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD   4


CursesProvider::CursesProvider(){
        string email, pswd;
        cout << "Enter email: ";
        cin >> email;

        termios oldt;
        tcgetattr(STDIN_FILENO, &oldt);
        termios newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        cout << "Enter password: ";
        cin >> pswd;
        feedly.authenticateUser(email, pswd);

        initscr();
        start_color();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);

        labels = feedly.getLabels();
        lastEntryRead = "";
        currentCategoryRead = false;
}
void CursesProvider::init(){
        init_pair(1, COLOR_RED, 0);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);

        createCategoriesMenu();
        createPostsMenu();

        panels[0] = new_panel(ctgWin);
        panels[1] = new_panel(postsWin);

        set_panel_userptr(panels[0], panels[1]);
        set_panel_userptr(panels[1], panels[0]);

        update_panels();

        attron(COLOR_PAIR(4));
        mvprintw(LINES - 1, 0, "A: mark all read  u: mark unread  r: mark read  R: refresh  F1: exit");
        attroff(COLOR_PAIR(4));
        doupdate();

        top = panels[1];
        top_panel(top);
}
void CursesProvider::control(){
        int ch;
        MENU* curMenu = postsMenu;
        ITEM* curItem = current_item(curMenu);

        while((ch = getch()) != KEY_F(1)){
                curItem = current_item(curMenu);
                switch(ch){
                        case 9:
                                if(curMenu == ctgMenu)
                                        curMenu = postsMenu;
                                else
                                        curMenu = ctgMenu;

                                top = (PANEL *)panel_userptr(top);
                                top_panel(top);
                                break;
                        case KEY_DOWN:
                                menu_driver(curMenu, REQ_DOWN_ITEM);
                                break;
                        case KEY_UP:
                                menu_driver(curMenu, REQ_UP_ITEM);
                                break;
                        case 'j':
                                menu_driver(curMenu, REQ_DOWN_ITEM);
                                break;
                        case 'k':
                                menu_driver(curMenu, REQ_UP_ITEM);
                                break;
                        case 'u':{
                                         vector<string> *temp = new vector<string>;
                                         temp->push_back(item_description(curItem));

                                         feedly.markPostsUnread(temp);
                                         item_opts_on(curItem, O_SELECTABLE);

                                         break;
                                 }
                        case 'r':{
                                         vector<string> *temp = new vector<string>;
                                         temp->push_back(item_description(curItem));

                                         feedly.markPostsRead(temp);
                                         item_opts_on(curItem, O_SELECTABLE);

                                         break;
                                 }
                        case 'R':
                                 ctgMenuCallback(strdup(item_name(current_item(ctgMenu))));
                                 break;
                        case 'A':
                                 feedly.markCategoriesRead(item_description(current_item(ctgMenu)), lastEntryRead);
                                 ctgMenuCallback(strdup(item_name(curItem)));
                                 currentCategoryRead = true;
                                 break;
                        case 10:
                                 if(curMenu == ctgMenu){
                                         top = (PANEL *)panel_userptr(top);
                                         top_panel(top);

                                         wclear(postsWin);
                                         ctgMenuCallback(strdup(item_name(current_item(curMenu))));

                                         win_show(postsWin, strdup("Posts"), 1);
                                         if(currentCategoryRead)
                                                 curMenu = ctgMenu;
                                         else
                                                 curMenu = postsMenu;
                                 }
                                 else if(panel_window(top) == postsWin){
                                         postsMenuCallback(curItem);
                                 }
                                 break;
                }
                update_panels();
                doupdate();
        }
        cleanup();
}
void CursesProvider::createCategoriesMenu(){
        int n_choices, c, i = 3;

        n_choices = labels->size();
        ctgItems = (ITEM **)calloc(sizeof(std::string::value_type)*n_choices, sizeof(ITEM *));

        ctgItems[0] = new_item("All", labels->at("All").c_str());
        ctgItems[1] = new_item("Saved", labels->at("Saved").c_str());
        ctgItems[2] = new_item("Uncategorized", labels->at("Uncategorized").c_str());

        for(auto it = labels->begin(); it != labels->end(); ++it){
                if(it->first.compare("All") != 0 && it->first.compare("Saved") != 0 && it->first.compare("Uncategorized") != 0){
                        ctgItems[i] = new_item((it->first).c_str(), (it->second).c_str());
                        i++;
                }
        }

        ctgMenu = new_menu((ITEM **)ctgItems);

        ctgWin = newwin(LINES-2, 40, 0, 0);
        keypad(ctgWin, TRUE);

        set_menu_win(ctgMenu, ctgWin);
        set_menu_sub(ctgMenu, derwin(ctgWin, 0, 38, 3, 1));

        set_menu_mark(ctgMenu, "  ");

        win_show(ctgWin, strdup("Categories"),  1);

        menu_opts_off(ctgMenu, O_SHOWDESC);
        menu_opts_on(postsMenu, O_NONCYCLIC);

        post_menu(ctgMenu);
}
void CursesProvider::createPostsMenu(){
        int height, width;

        int n_choices, c, i = 0;

        const vector<PostData> *posts = feedly.giveStreamPosts("All");

        if(posts != NULL){
                n_choices = posts->size();
                postsItems = (ITEM **)calloc(sizeof(std::vector<PostData>::value_type)*n_choices, sizeof(ITEM *));

                for(auto it = posts->begin(); it != posts->end(); ++it){
                        postsItems[i] = new_item((it->title).c_str(), (it->id).c_str()); 
                        i++;
                }

                postsMenu = new_menu((ITEM **)postsItems);
                lastEntryRead = item_description(postsItems[0]);
        }
        else{
                postsMenu = new_menu(NULL);
                print_in_center(postsWin, 3, 1, height, width-4, strdup("All Posts Read"), 1);  
                currentCategoryRead = true;
        }

        postsWin = newwin(LINES-2, 0, 0, 40);
        keypad(postsWin, TRUE);

        getmaxyx(postsWin, height, width);

        set_menu_win(postsMenu, postsWin);
        set_menu_sub(postsMenu, derwin(postsWin, height-4, width-2, 3, 1));
        set_menu_format(postsMenu, height-4, 0);

        set_menu_fore(postsMenu, COLOR_PAIR(1) | A_REVERSE);
        set_menu_back(postsMenu, COLOR_PAIR(2));
        set_menu_grey(postsMenu, COLOR_PAIR(3));

        set_menu_mark(postsMenu, "*");

        win_show(postsWin, strdup("Posts"),  1);

        menu_opts_off(postsMenu, O_SHOWDESC);

        post_menu(postsMenu);
}
void CursesProvider::ctgMenuCallback(char* label){
        int startx, starty, height, width;

        getmaxyx(postsWin, height, width);
        getbegyx(postsWin, starty, startx);

        int n_choices, i = 0;
        const vector<PostData>* posts = feedly.giveStreamPosts(label);

        if(posts == NULL){
                unpost_menu(postsMenu);
                set_menu_items(postsMenu, NULL);
                post_menu(postsMenu);

                print_in_center(postsWin, 3, 1, height, width-4, strdup("All Posts Read"), 1);  
                currentCategoryRead = true;
                return;
        }

        n_choices = posts->size() + 1;
        ITEM** items = (ITEM **)calloc(sizeof(std::vector<PostData>::value_type)*n_choices, sizeof(ITEM *));

        for(auto it = posts->begin(); it != posts->end(); ++it){
                items[i] = new_item((it->title).c_str(), (it->id).c_str());
                i++;
        }

        items[i] = NULL;

        unpost_menu(postsMenu);
        set_menu_items(postsMenu, items);
        post_menu(postsMenu);

        set_menu_format(postsMenu, height, 0);
        lastEntryRead = item_description(items[0]);
        currentCategoryRead = false;
}
void CursesProvider::postsMenuCallback(ITEM* item){
        item_opts_off(item, O_SELECTABLE);

        PostData* container = feedly.getSinglePostData(item_index(item));

        ofstream myfile ("example.html");

        if (myfile.is_open())
                myfile << container->content;

        myfile.close();

        def_prog_mode();
        endwin();

        system("w3m example.html");

        vector<string> *temp = new vector<string>;
        temp->push_back(container->id);

        feedly.markPostsRead(const_cast<vector<string>*>(temp));

        reset_prog_mode();

        update_panels();
        lastEntryRead = item_description(item);
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
void CursesProvider::print_in_middle(WINDOW *win, int starty, int startx, int width, char *str, chtype color){   
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

        length = strlen(str);
        temp = (width - length)/ 2;
        x = startx + (int)temp;
        wattron(win, color);
        mvwprintw(win, y, x, "%s", str);
        wattroff(win, color);
}
void CursesProvider::print_in_center(WINDOW *win, int starty, int startx, int height, int width, char *str, chtype color){   
        int length, x, y;
        float tempX, tempY;

        if(win == NULL)
                win = stdscr;
        getyx(win, y, x);
        if(startx != 0)
                x = startx;
        if(starty != 0)
                y = starty;
        if(width == 0)
                width = 80;

        length = strlen(str);
        tempX = (width - length)/ 2;
        tempY = (height / 2);
        x = startx + (int)tempX;
        y = starty + (int)tempY; 
        wattron(win, color);
        mvwprintw(win, y, x, "%s", str);
        wattroff(win, color);
}
void CursesProvider::cleanup(){
        unpost_menu(ctgMenu);
        free_menu(ctgMenu);
        for(int i = 0; i < ARRAY_SIZE(ctgItems); ++i)
                free_item(ctgItems[i]);

        unpost_menu(postsMenu);
        free_menu(postsMenu);
        for(int i = 0; i < ARRAY_SIZE(postsItems); ++i)
                free_item(postsItems[i]);

        endwin();
        feedly.curl_cleanup();
}
