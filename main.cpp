#include <iostream>

using namespace std;

#include "FeedlyProvider.h"
#include "CursesProvider.h"

int main(int argc, char **argv)
{
        CursesProvider curses;
        FeedlyProvider feedly;

        feedly.authenticateUser("jorgemartinezhernandez", "trueforerunner117");
        feedly.giveAllUnread();
        curses.create_menu(feedly.giveLabels());
        return 0;
}


