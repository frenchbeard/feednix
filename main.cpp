#include <iostream>

using namespace std;

#include "FeedlyProvider.h"
#include "CursesProvider.h"

int main(int argc, char **argv)
{
//        CursesProvider curses;
//        curses.create_menu();
        FeedlyProvider feedly;

        feedly.authenticateUser("jorgemartinezhernandez", "trueforerunner117");
        feedly.giveAllUnread();
        feedly.giveLabels();
        return 0;
}


