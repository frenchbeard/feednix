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

        bool verboseEnabled = false;
        CursesProvider *curses;
        if(argc >= 2){
                for(int i = 1; i < argc; ++i){
                        if(argv[i][0] == '-' && argv[i][1] == 'v' && strlen(argv[1]) <= 2)
                                verboseEnabled = true;
                        else if(argv[i][0] == '-' && argv[i][1] == 'h' && strlen(argv[1]) <= 2){
                                std::cout << "Usage: feednix [OPTIONS]" << std::endl;
                                std::cout << "  An ncurses-based console client for Feedly written in C++" << std::endl;
                                std::cout << "\n Options:\n  -h        Display this help and exit\n  -v        Set curl to output in verbose mode during login" << std::endl;
                                std::cout << "\n Config:\n   Feednix uses a config file to set colors and\n   and the amount of posts to be retrived per\n   request." << std::endl;
                                std::cout << "\n   This file can be found and must be placed in:\n     $HOME/.config/feednix" << std::endl;
                                std::cout << "\n Author:\n   Copyright Jorge Martinez Hernandez <jorgemartinezhernandez@gmail.com>\n   Licensing information can be found in the source code" << std::endl;
                                std::cout << "\n Bugs:\n   Please report any bugs on Github <https://github.com/Jarkore/Feednix" << std::endl;
                                exit(EXIT_SUCCESS);

                        }
                        else{
                                std::cerr << "ERROR: Invalid option " << "\'" << argv[i] << "\'" << std::endl;
                                exit(EXIT_FAILURE);
                        }
                }
                if(verboseEnabled)
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
