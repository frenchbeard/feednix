#include <iostream>

using namespace std;

#include "FeedlyProvider.h"

int main(int argc, char **argv)
{
        FeedlyProvider provider;

        cout << provider.getAuthCode() << endl;
        return 0;
}


