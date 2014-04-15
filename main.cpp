#include <iostream>

using namespace std;

#include "FeedlyProvider.h"
#include "CursesProvider.h"

int main(int argc, char **argv)
{
        FeedlyProvider feedly;
        feedly.authenticateUser("jorgemartinezhernandez", "trueforerunner117");

        CursesProvider curses(feedly.getLabels());

        //feedly.getLabel();
        curses.init();
        feedly.giveCategoryPosts("Another");
        feedly.curl_cleanup();
        return 0;
}


