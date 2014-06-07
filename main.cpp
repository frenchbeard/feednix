#include <iostream>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "CursesProvider.h"

#define HOME_PATH getenv("HOME")

void atExitFunction(void){
        system(std::string("find " + std::string(HOME_PATH) + "/.config/feednix -type f -not -name \'config.json\' -delete 2> /dev/null").c_str());
}

void sighandler(int signum){
        system(std::string("find " + std::string(HOME_PATH) + "/.config/feednix -type f -not -name \'config.json\' -delete 2> /dev/null").c_str());
        signal(signum, SIG_DFL);
        kill(getpid(), signum);
}

int main(int argc, char **argv){
        signal(SIGINT, sighandler);
        signal(SIGTERM, sighandler);
        atexit(atExitFunction);

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
