#include <iostream>

using namespace std;

#include "FeedlyProvider.h"

int main(int argc, char **argv)
{
        FeedlyProvider provider;

        cout << provider.authenticateUser("jorgemartinezhernandez", "trueforerunner117") << endl;
        provider.parseAuthenticationResponse();
        return 0;
}


