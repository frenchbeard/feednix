#include <curses.h>
#include <map>
#include <panel.h>
#include <menu.h> 
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "CursesProvider.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD   4

#define HOME_PATH getenv("HOME")

CursesProvider::CursesProvider(bool verbose){
        feedly.setVerbose(verbose);
        feedly.askForCredentials();

        initscr();
        start_color();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);

        lastEntryRead = "";
        currentCategoryRead = false;
        feedly.setVerbose(false);
}
void CursesProvider::init(){
        init_pair(1, COLOR_RED, 0);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
        init_pair(4, COLOR_CYAN, COLOR_BLACK);
        init_pair(5, COLOR_WHITE, COLOR_BLACK);

        createCategoriesMenu();
        createPostsMenu();

        panels[0] = new_panel(ctgWin);
        panels[1] = new_panel(postsWin);

        set_panel_userptr(panels[0], panels[1]);
        set_panel_userptr(panels[1], panels[0]);

        update_panels();

        attron(COLOR_PAIR(4));
        mvprintw(LINES - 1, 0, "Enter: See Preview  A: mark all read  u: mark unread  r: mark read  R: refresh  o: Open in plain-text  O: Open in Browser  F1: exit");
        attroff(COLOR_PAIR(4));
        doupdate();

        top = panels[1];
        top_panel(top);
}
void CursesProvider::control(){
        int ch;
        MENU* curMenu = postsMenu;
        ITEM* curItem = current_item(curMenu);

        update_counter();

        while((ch = getch()) != KEY_F(1)){
                curItem = current_item(curMenu);

                switch(ch){
                        case 10:
                                if(curMenu == ctgMenu){
                                        top = (PANEL *)panel_userptr(top);
                                        top_panel(top);

                                        mvprintw(LINES-2, 0, "Updating stream...");
                                        refresh();
                                        ctgMenuCallback(strdup(item_name(current_item(curMenu))));
                                        clear_updateline();

                                        win_show(postsWin, strdup("Posts"), 1, true);
                                        win_show(ctgWin, strdup("Categories"), 1, false);
                                        if(currentCategoryRead)
                                                curMenu = ctgMenu;
                                        else
                                                curMenu = postsMenu;
                                }
                                else if(panel_window(top) == postsWin){
                                        if(item_opts(curItem) == 1){
                                                numRead++;
                                                numUnread--; 
                                        }

                                        postsMenuCallback(curItem, true);
                                }
                                break;
                        case 9:
                                if(curMenu == ctgMenu){
                                        curMenu = postsMenu;
                                        win_show(postsWin, strdup("Posts"), 1, true);
                                        win_show(ctgWin, strdup("Categories"), 1, false);

                                        move(LINES-1, 0);
                                        clrtoeol();
                                        attron(COLOR_PAIR(4));
                                        mvprintw(LINES - 1, 0, "Enter: See Preview  A: mark all read  u: mark unread  r: mark read  R: refresh  o: Open in plain-text  O: Open in Browser  F1: exit");
                                        attroff(COLOR_PAIR(4));
                                        refresh();
                                }
                                else{
                                        curMenu = ctgMenu;
                                        win_show(ctgWin, strdup("Categories"), 1, true);
                                        win_show(postsWin, strdup("Posts"), 1, false);

                                        move(LINES-1, 0);
                                        clrtoeol();

                                        attron(COLOR_PAIR(4));
                                        mvprintw(LINES - 1, 0, "Enter: Fetch Stream  A: mark all read  R: refresh  F1: exit");
                                        attroff(COLOR_PAIR(4));

                                        refresh();
                                }

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
                                         std::vector<std::string> *temp = new std::vector<std::string>;
                                         temp->push_back(item_description(curItem));

                                         mvprintw(LINES-2, 0, "Marking post unread...");
                                         refresh();

                                         feedly.markPostsUnread(temp);
                                         clear_updateline();

                                         item_opts_on(curItem, O_SELECTABLE);
                                         numRead--;
                                         numUnread++;

                                         break;
                                 }
                        case 'r':{
                                         std::vector<std::string> *temp = new std::vector<std::string>;
                                         temp->push_back(item_description(curItem));

                                         mvprintw(LINES-2, 0, "Marking post read...");
                                         refresh();

                                         feedly.markPostsRead(temp);
                                         clear_updateline();

                                         item_opts_off(curItem, O_SELECTABLE);
                                         numUnread--;
                                         numRead++;

                                         break;
                                 }
                        case 'R':
                                 mvprintw(LINES-2, 0, "Updating stream...");
                                 refresh();

                                 ctgMenuCallback(strdup(item_name(current_item(ctgMenu))));
                                 clear_updateline();
                                 break;
                        case 'o':
                                 postsMenuCallback(curItem, false);
                                 break;
                        case 'O':{
                                 termios oldt;
                                 tcgetattr(STDIN_FILENO, &oldt);
                                 termios newt = oldt;
                                 newt.c_lflag &= ~ECHO;
                                 tcsetattr(STDIN_FILENO, TCSANOW, &newt);

                                 PostData* data = feedly.getSinglePostData(item_index(curItem));
                                 std::vector<std::string> *temp = new std::vector<std::string>;
                                 temp->push_back(data->id);

                                 system(std::string("xdg-open \"" + data->originURL + "\" > /dev/null").c_str());
                                 mvprintw(LINES-2, 0, "Marking post read...");
                                 refresh();

                                 feedly.markPostsRead(temp);
                                 clear_updateline();
                                 item_opts_off(curItem, O_SELECTABLE);

                                 break;
                                 }
                        case 'a':{
                                         char feed[200];
                                         char title[200];
                                         char ctg[200];
                                         echo();

                                         mvprintw(LINES - 2, 0, "[ENTER FEED]:");
                                         mvgetnstr(LINES-2, strlen("[ENTER FEED]") + 1, feed, 200); 
                                         mvaddch(LINES-2, 0, ':');

                                         clrtoeol();

                                         mvprintw(LINES - 2, 0, "[ENTER TITLE]:");
                                         mvgetnstr(LINES-2, strlen("[ENTER TITLE]") + 1, title, 200); 
                                         mvaddch(LINES-2, 0, ':');

                                         clrtoeol();

                                         mvprintw(LINES - 2, 0, "[ENTER CATEGORY]:");
                                         mvgetnstr(LINES-2, strlen("[ENTER CATEGORY]") + 1, ctg, 200); 
                                         mvaddch(LINES-2, 0, ':');

                                         std::istringstream ss(ctg);
                                         std::istream_iterator<std::string> begin(ss), end;

                                         std::vector<std::string> arrayTokens(begin, end);

                                         noecho();
                                         clrtoeol();

                                         mvprintw(LINES-2, 0, "Adding subscription...");
                                         refresh();

                                         if(strlen(feed) != 0)
                                                 feedly.addSubscription(false, feed, arrayTokens, title); 

                                         clear_updateline();
                                         break;
                                 }
                        case 'A':{
                                         mvprintw(LINES-2, 0, "Marking category read...");
                                         refresh();

                                         feedly.markCategoriesRead(item_description(current_item(ctgMenu)), lastEntryRead);
                                         clear_updateline();

                                         ctgMenuCallback(strdup(item_name(curItem)));
                                         currentCategoryRead = true;
                                         break;
                                 }
                }
                update_counter();
                update_panels();
                doupdate();
        }
        cleanup();
}
void CursesProvider::createCategoriesMenu(){
        int n_choices, c, i = 3;
        const std::map<std::string, std::string> *labels = feedly.getLabels();

        n_choices = labels->size() + 1;
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

        win_show(ctgWin, strdup("Categories"),  1, false);

        menu_opts_off(ctgMenu, O_SHOWDESC);
        menu_opts_on(postsMenu, O_NONCYCLIC);

        post_menu(ctgMenu);
}
void CursesProvider::createPostsMenu(){
        int height, width;

        int n_choices, c, i = 0;

        const std::vector<PostData> *posts = feedly.giveStreamPosts("All");
        totalPosts = posts->size();
        numUnread = totalPosts;

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

        win_show(postsWin, strdup("Posts"),  1, true);

        menu_opts_off(postsMenu, O_SHOWDESC);

        post_menu(postsMenu);
}
void CursesProvider::ctgMenuCallback(char* label){
        int startx, starty, height, width;

        getmaxyx(postsWin, height, width);
        getbegyx(postsWin, starty, startx);

        int n_choices, i = 0;
        const std::vector<PostData>* posts = feedly.giveStreamPosts(label);
        totalPosts = posts->size();
        numRead = 0;
        numUnread = totalPosts;

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
void CursesProvider::postsMenuCallback(ITEM* item, bool preview){
        PostData* container = feedly.getSinglePostData(item_index(item));

        if(preview){
                std::string PREVIEW_PATH = std::string(HOME_PATH) + "/.config/feednix/preview.html";
                std::ofstream myfile (PREVIEW_PATH.c_str());

                if (myfile.is_open())
                        myfile << container->content;

                myfile.close();

                def_prog_mode();
                endwin();

                system(std::string("w3m " + PREVIEW_PATH).c_str());
                reset_prog_mode();
        }
        else{
                system(std::string("w3m \'" + container->originURL + "\'").c_str());
        }
        if(item_opts(item)){
                item_opts_off(item, O_SELECTABLE);
                mvprintw(LINES-2, 0, "Marking post read...");
                refresh();

                std::vector<std::string> *temp = new std::vector<std::string>;
                temp->push_back(container->id);

                feedly.markPostsRead(const_cast<std::vector<std::string>*>(temp));
                clear_updateline();

                update_panels();
        }
        lastEntryRead = item_description(item);
        system(std::string("rm " + std::string(HOME_PATH) + "/.config/preview.html 2> /dev/null").c_str());
}
void CursesProvider::win_show(WINDOW *win, char *label, int label_color, bool highlight){
        int startx, starty, height, width;

        getbegyx(win, starty, startx);
        getmaxyx(win, height, width);

        mvwaddch(win, 2, 0, ACS_LTEE);
        mvwhline(win, 2, 1, ACS_HLINE, width - 2);
        mvwaddch(win, 2, width - 1, ACS_RTEE);

        if(highlight){
                wattron(win, COLOR_PAIR(label_color));
                box(win, 0, 0);
                mvwaddch(win, 2, 0, ACS_LTEE);
                mvwhline(win, 2, 1, ACS_HLINE, width - 2);
                mvwaddch(win, 2, width - 1, ACS_RTEE);
                print_in_middle(win, 1, 0, width, label, COLOR_PAIR(label_color));
                wattroff(win, COLOR_PAIR(label_color));
        }
        else{
                wattron(win, COLOR_PAIR(5));
                box(win, 0, 0);
                mvwaddch(win, 2, 0, ACS_LTEE);
                mvwhline(win, 2, 1, ACS_HLINE, width - 2);
                mvwaddch(win, 2, width - 1, ACS_RTEE);
                print_in_middle(win, 1, 0, width, label, COLOR_PAIR(5));
                wattroff(win, COLOR_PAIR(5));
        }

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
        mvwprintw(win, y, x, "%s", str);
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
void CursesProvider::clear_updateline(){
        move(LINES-2, 0);
        clrtoeol();
}
void CursesProvider::update_counter(){
        std::stringstream sstm;
        sstm << "[" << numUnread << ":" << numRead << "/" << totalPosts << "]";
        const char* counter = sstm.str().c_str(); 

        move(LINES - 2, COLS - strlen(counter));
        clrtoeol();
        mvprintw(LINES - 2, COLS - strlen(counter), counter);
        refresh();
        update_panels();
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
