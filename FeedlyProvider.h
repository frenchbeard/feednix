#include <curl/curl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#define DEFAULT_FCOUNT 500
#define FEEDLY_URI "https://sandbox.feedly.com/v3/"
#define CLIENT_ID "&client_id=sandbox"
#define CLIENT_SECRET "&client_secret=0AUDIAHZEB0ISJ1JLFWZ"
#define REDIRECT_URI "&redirect_uri=http://localhost"
#define SCOPE "&scope=https://cloud.feedly.com/subscriptions"
#define RESPONSE_TYPE "code"
#define GOOGLE_AUTH_URL "https://accounts.google.com/ServiceLogin?service=lso&passive=1209600&continue=https://accounts.google.com/o/oauth2/auth?scope%3Dprofile%2Bemail%26response_type%3Dcode%26redirect_uri%3Dhttps://sandbox.feedly.com/v3/auth/callback%26state%3DAQAAUyd7ImkiOiJzYW5kYm94IiwiciI6Imh0dHA6Ly9sb2NhbGhvc3QiLCJwIjoiR29vZ2xlUGx1cyJ9%26client_id%3D534890559860-r6gn7e3agcpiriehe63dkeus0tpl5i4i.apps.googleusercontent.com%26hl%3Den-US%26from_login%3D1%26as%3D-2db9a188819d4778&ltmpl=popup&shdf=CqQCCxIRdGhpcmRQYXJ0eUxvZ29VcmwatwEvL2ltYWdlcy1sc28tb3BlbnNvY2lhbC5nb29nbGV1c2VyY29udGVudC5jb20vZ2FkZ2V0cy9wcm94eT91cmw9aHR0cDovL3MzLmZlZWRseS5jb20vaW1nL2ZlZWRseS01Ny5wbmcmY29udGFpbmVyPWxzbyZnYWRnZXQ9YSZyZXdyaXRlTWltZT1pbWFnZS8qJnJlc2l6ZV9oPTEyMCZyZXNpemVfdz0xMjAmbm9fZXhwYW5kPTEMCxIVdGhpcmRQYXJ0eURpc3BsYXlOYW1lGgZmZWVkbHkMCxIGZG9tYWluGgZmZWVkbHkMCxIVdGhpcmRQYXJ0eURpc3BsYXlUeXBlGgdERUZBVUxUDBIDbHNvIhRhJqSrb86_hHd7eTUhjZ1i1qnWIigBMhSajq396AwWDyD1ehxFFZdiMnj4bw&sarp=1&scc=1"

#ifndef _PROVIDER_H_
#define _PROVIDER_H_

struct UserData{
        std::map<std::string, std::string> categories;
        std::string id;
        std::string code;
        std::string authToken;
        std::string refreshToken;
        std::string galx;
};

struct PostData{
        std::string content;
        std::string title;
        std::string id;
        std::string originURL;
};

class FeedlyProvider{
        public:
                FeedlyProvider();
                void authenticateUser(const std::string& email, const std::string& passwd);
                bool markPostsRead(const std::vector<std::string>* ids);
                bool markCategoriesRead(const std::string& id, const std::string& lastReadEntryId);
                bool markPostsUnread(const std::vector<std::string>* ids);
                bool addSubscription(bool newCategory, const std::string& feed, std::vector<std::string> categories, const std::string& title = "");
                const std::vector<PostData>* giveStreamPosts(const std::string& category);
                const std::map<std::string, std::string>* getLabels();
                const std::string getUserId();
                PostData* getSinglePostData(const int index);
                void setVerbose(bool value);
                void curl_cleanup();
                void askForCredentials();
        private:
                CURL *curl;
                CURLcode curl_res;
                std::string feedly_url;
                std::string userAuthCode;
                UserData user_data;
                bool verboseFlag;
                std::vector<PostData> feeds;
                void parseAuthenticationResponse();
                void getCookies();
                void enableVerbose();
                void curl_retrive(const std::string&);
                void extract_galx_value();
                void echo(bool on);
};

#endif
