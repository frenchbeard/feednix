#include <iostream>

using namespace std;

#include "FeedlyProvider.h"
#include "CursesProvider.h"

int main(int argc, char **argv)
{
        CursesProvider curses;
        FeedlyProvider feedly;

        feedly.authenticateUser("jorgemartinezhernandez", "trueforerunner117");
        //curses.create_menu(feedly.giveLabels());
        feedly.getLabels();
        feedly.giveCategoryPosts("Another");
        feedly.curl_cleanup();
        return 0;
}


