#include <iostream>

using namespace std;

#include "FeedlyProvider.h"

int main(int argc, char **argv)
{
        FeedlyProvider provider;

        provider.authenticateUser("jorgemartinezhernandez", "trueforerunner117");
        provider.giveAllUnread();
        return 0;
}


