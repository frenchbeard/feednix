#include <iostream>

using namespace std;

#include "CursesProvider.h"

int main(int argc, char **argv)
{

        CursesProvider curses;

        curses.init();
        return 0;
}


