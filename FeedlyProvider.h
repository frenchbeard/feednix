#include <curl/curl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <jsoncpp/json/json.h>

using namespace std;

#define FEEDLY_URI "https://sandbox.feedly.com/v3/"
#define CLIENT_ID "&client_id=sandbox"
#define CLIENT_SECRET "&client_secret=V0H9C3O75ODIXFSSX9OH"
#define REDIRECT_URI "&redirect_uri=http://localhost"
#define SCOPE "&scope=https://cloud.feedly.com/subscriptions"
#define RESPONSE_TYPE "code"
#define GOOGLE_AUTH_URL "https://accounts.google.com/ServiceLogin?service=lso&passive=1209600&continue=https://accounts.google.com/o/oauth2/auth?scope%3Dprofile%2Bemail%26response_type%3Dcode%26redirect_uri%3Dhttps://sandbox.feedly.com/v3/auth/callback%26state%3DAQAAUyd7ImkiOiJzYW5kYm94IiwiciI6Imh0dHA6Ly9sb2NhbGhvc3QiLCJwIjoiR29vZ2xlUGx1cyJ9%26client_id%3D534890559860-r6gn7e3agcpiriehe63dkeus0tpl5i4i.apps.googleusercontent.com%26hl%3Den-US%26from_login%3D1%26as%3D-2db9a188819d4778&ltmpl=popup&shdf=CqQCCxIRdGhpcmRQYXJ0eUxvZ29VcmwatwEvL2ltYWdlcy1sc28tb3BlbnNvY2lhbC5nb29nbGV1c2VyY29udGVudC5jb20vZ2FkZ2V0cy9wcm94eT91cmw9aHR0cDovL3MzLmZlZWRseS5jb20vaW1nL2ZlZWRseS01Ny5wbmcmY29udGFpbmVyPWxzbyZnYWRnZXQ9YSZyZXdyaXRlTWltZT1pbWFnZS8qJnJlc2l6ZV9oPTEyMCZyZXNpemVfdz0xMjAmbm9fZXhwYW5kPTEMCxIVdGhpcmRQYXJ0eURpc3BsYXlOYW1lGgZmZWVkbHkMCxIGZG9tYWluGgZmZWVkbHkMCxIVdGhpcmRQYXJ0eURpc3BsYXlUeXBlGgdERUZBVUxUDBIDbHNvIhRhJqSrb86_hHd7eTUhjZ1i1qnWIigBMhSajq396AwWDyD1ehxFFZdiMnj4bw&sarp=1&scc=1"

#ifndef _PROVIDER_H_
#define _PROVIDER_H_

struct UserData{
        map<string, string> categories;
        string id;
        string code;
        string authToken;
        string refreshToken;
        string galx;
};

struct PostData{
        string content;
        string title;
};

class FeedlyProvider{
        public:
                FeedlyProvider();
                void authenticateUser(const string& email, const string& passwd);
                const map<string, PostData>* giveAllUnread();
                void parseAuthenticationResponse();
                void getCookies();
                void curl_cleanup();
                const map<string, PostData>* giveStreamPosts(const string& category);
                const map<string, string>* getLabels();
                PostData* getSinglePostData(const string& id);
        private:
                CURL *curl;
                CURLcode curl_res;
                string feedly_url;
                string userAuthCode;
                UserData user_data;
                bool verboseFlag;
                map<string, PostData> feeds;
                void enableVerbose();
                void curl_retrive(const string&);
                void extract_galx_value();
};

#endif
