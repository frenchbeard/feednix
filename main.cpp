#include <iostream>
#include <string.h>

#include "CursesProvider.h"

int main(int argc, char **argv)
{
        CursesProvider *curses;
        if(argc >= 2){
                if(argv[1][1] == 'v' && strlen(argv[1]) <= 2)
                        curses = new CursesProvider(true);
                else
                        curses = new CursesProvider(false);
        }
        else{
                curses = new CursesProvider(false);
        }

        curses->init();
        curses->control();

        delete curses;
        return 0;
}


