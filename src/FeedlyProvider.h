#ifndef PROVIDER_H_
#define PROVIDER_H_
#include <curl/curl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#define DEFAULT_FCOUNT 500
#define FEEDLY_URI "https://cloud.feedly.com/v3/"
#define HOME_PATH getenv("HOME")

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
    void authenticateUser();
    bool markPostsRead(const std::vector<std::string>* ids);
    bool markPostsSaved(const std::vector<std::string>* ids);
    bool markPostsUnsaved(const std::vector<std::string>* ids);
    bool markCategoriesRead(const std::string& id, const std::string& lastReadEntryId);
    bool markPostsUnread(const std::vector<std::string>* ids);
    bool addSubscription(bool newCategory, const std::string& feed, std::vector<std::string> categories, const std::string& title = "");
    const std::vector<PostData>* giveStreamPosts(const std::string& category);
    const std::map<std::string, std::string>* getLabels();
    const std::string getUserId();
    PostData* getSinglePostData(const int index);
    void setVerbose(bool value);
    void setChangeTokensFlag(bool value);
    void curl_cleanup();
  private:
    CURL *curl;
    CURLcode curl_res;
    std::string feedly_url;
    std::string userAuthCode;
    std::string TOKEN_PATH, TEMP_PATH, COOKIE_PATH, rtrv_count;
    UserData user_data;
    bool verboseFlag, changeTokens;
    std::vector<PostData> feeds;
    void getCookies();
    void enableVerbose();
    void curl_retrive(const std::string&);
    void extract_galx_value();
    void echo(bool on);
};

#endif //
