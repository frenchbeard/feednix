#include <iostream>

using namespace std;

#include "FeedlyProvider.h"

int main(int argc, char **argv)
{
        FeedlyProvider provider;

        cout << provider.getAuthCode("jorgemartinezhernandez", "trueforerunner117") << endl;
        return 0;
}


