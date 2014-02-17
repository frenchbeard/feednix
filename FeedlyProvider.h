#include <curl/curl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>

using namespace std;

class FeedlyProvider{
        public:
                FeedlyProvider();
                string getAuthCode(const string& email, const string& passwd);
                void giveAll();
                void giveUnread();
                void giveSaved();
                void giveTopics();
                void giveLabels();
                void searchUserFeeds();
                void giveSingleEntry();
                void getTokens();
                void getCookies(FILE*);
        private:
                CURL *curl;
                CURLcode curl_res;
                string feedly_url;
                string userAuthCode;
                string extract_galx_value();
                map<string, string> userData;
                #define FEEDLY_URI "https://sandbox.feedly.com/v3/"
                #define CLIENT_ID "&client_id=sandbox"
                #define CLIENT_SECRET "&client_secret=CM786L1D4P3M9VYUPOB8"
                #define REDIRECT_URI "&redirect_uri=http://localhost"
                #define SCOPE "&scope=https://cloud.feedly.com/subscriptions"
                #define RESPONSE_TYPE "code"
                #define GOOGLE_AUTH_URL "https://accounts.google.com/ServiceLogin?service=lso&passive=1209600&continue=https://accounts.google.com/o/oauth2/auth?scope%3Dprofile%2Bemail%26response_type%3Dcode%26redirect_uri%3Dhttps://sandbox.feedly.com/v3/auth/callback%26state%3DAQAAUyd7ImkiOiJzYW5kYm94IiwiciI6Imh0dHA6Ly9sb2NhbGhvc3QiLCJwIjoiR29vZ2xlUGx1cyJ9%26client_id%3D534890559860-r6gn7e3agcpiriehe63dkeus0tpl5i4i.apps.googleusercontent.com%26hl%3Den-US%26from_login%3D1%26as%3D-2db9a188819d4778&ltmpl=popup&shdf=CqQCCxIRdGhpcmRQYXJ0eUxvZ29VcmwatwEvL2ltYWdlcy1sc28tb3BlbnNvY2lhbC5nb29nbGV1c2VyY29udGVudC5jb20vZ2FkZ2V0cy9wcm94eT91cmw9aHR0cDovL3MzLmZlZWRseS5jb20vaW1nL2ZlZWRseS01Ny5wbmcmY29udGFpbmVyPWxzbyZnYWRnZXQ9YSZyZXdyaXRlTWltZT1pbWFnZS8qJnJlc2l6ZV9oPTEyMCZyZXNpemVfdz0xMjAmbm9fZXhwYW5kPTEMCxIVdGhpcmRQYXJ0eURpc3BsYXlOYW1lGgZmZWVkbHkMCxIGZG9tYWluGgZmZWVkbHkMCxIVdGhpcmRQYXJ0eURpc3BsYXlUeXBlGgdERUZBVUxUDBIDbHNvIhRhJqSrb86_hHd7eTUhjZ1i1qnWIigBMhSajq396AwWDyD1ehxFFZdiMnj4bw&sarp=1&scc=1"
};
