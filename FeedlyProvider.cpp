#include <stdio.h>
#include <stdlib.h>
#include <jsoncpp/json/json.h>
#include <iterator>
#include <istream>
#include <termios.h>
#include <unistd.h>

#include "FeedlyProvider.h"

#define HOME_PATH getenv("HOME")

FeedlyProvider::FeedlyProvider(){
        curl_global_init(CURL_GLOBAL_DEFAULT);
        verboseFlag = false;

        TEMP_PATH = std::string(HOME_PATH) + "/.config/feednix/temp.txt";
        TOKEN_PATH = std::string(HOME_PATH) + "/.config/feednix/tokenFile";
        COOKIE_PATH = std::string(HOME_PATH) + "/.config/feednix/cookie";

        Json::Value root;
        Json::Reader reader;

        std::ifstream tokenFile(std::string(std::string(HOME_PATH) + "/.config/feednix/config.json").c_str(), std::ifstream::binary);
        if(reader.parse(tokenFile, root)){
                rtrv_count = root["posts_retrive_count"].asString();
        }
}
void FeedlyProvider::askForCredentials(){
        std::string email, pswd;
        std::cout << "Enter email: ";
        std::cin >> email;

        std::cout << "Enter password: ";

        echo(false);
        std::cin >> pswd;
        echo(true);

        authenticateUser(email, pswd);
}
void FeedlyProvider::authenticateUser(const std::string& email, const std::string& passwd){
        getCookies();

        FILE* data_holder = fopen(TEMP_PATH.c_str(), "wb");

        curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, std::string(GOOGLE_AUTH_URL).c_str()); 
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1 );
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, COOKIE_PATH.c_str());
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, COOKIE_PATH.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);

        enableVerbose();

        curl_res = curl_easy_perform(curl);

        curl_easy_setopt(curl, CURLOPT_REFERER, std::string(GOOGLE_AUTH_URL).c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, std::string("signIn=Sign+in&_utf8=&#9731;&bgresponse=js_disabled&GALX=" + user_data.galx + "&pstMsg=&dnCon=&checkConnection=youtube:1141:1&checkedDomains=youtube&Email=" + email + "&Passwd=" + passwd).c_str());

        curl_res = curl_easy_perform(curl);

        char *currentURL;
        user_data.code = "";

        if(curl_res != CURLE_OK){
                std::cerr << "\nERROR: Could not authenticate user" << std::endl;
                fprintf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_strerror(curl_res));
                exit(EXIT_FAILURE);
        }

        curl_res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &currentURL);
        std::string tempCode(currentURL);

        if((curl_res == CURLE_OK) && currentURL){
                std::size_t codeIndex = tempCode.find("code=") + 5;
                user_data.code = tempCode.substr(codeIndex, (tempCode.find("&", codeIndex) - codeIndex));

                if(user_data.code.find("google") != std::string::npos){
                        std::cout << "\nCould not log you in" << std::endl; 
                        exit(EXIT_FAILURE);
                }
                else
                        parseAuthenticationResponse();
        }

        fclose(data_holder);
        system(std::string("rm " + TEMP_PATH + " 2> /dev/null").c_str());
}
void FeedlyProvider::parseAuthenticationResponse(){
        struct curl_httppost *formpost = NULL;
        FILE *tokenJSON = fopen(TOKEN_PATH.c_str(), "wb");
        system(std::string("chmod 600 " + TOKEN_PATH).c_str());

        feedly_url = FEEDLY_URI + std::string("auth/token?code=") + user_data.code +  CLIENT_ID +  CLIENT_SECRET +  REDIRECT_URI + SCOPE + "&grant_type=authorization_code";

        curl_easy_setopt(curl, CURLOPT_URL, feedly_url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, tokenJSON);

        enableVerbose();

        curl_res = curl_easy_perform(curl);

        bool parsingSuccesful;
        fclose(tokenJSON);

        Json::Value root;
        Json::Reader reader;

        std::ifstream tokenFile(TOKEN_PATH.c_str(), std::ifstream::binary);
        parsingSuccesful = reader.parse(tokenFile, root);

        if(!parsingSuccesful || curl_res != CURLE_OK || !root.isMember("access_token")){
                if(!parsingSuccesful)
                        std::cerr << "\nERROR: Failed to parse tokens file" << reader.getFormatedErrorMessages() << std::endl;
                if(curl_res != CURLE_OK)
                        fprintf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_strerror(curl_res));
                if(!root.isMember("access_token"))
                        std::cerr << "ERROR: Failed to attain access token" << std::endl;

                        exit(EXIT_FAILURE);
        }
        if(tokenJSON != NULL && curl_res == CURLE_OK){

                if(parsingSuccesful){
                        user_data.authToken = (root["access_token"]).asString();
                        user_data.refreshToken = (root["refresh_token"]).asString();
                        user_data.id = (root["id"]).asString();
                }
        }

        system(std::string("rm " + std::string(TOKEN_PATH) + " 2> /dev/null").c_str());
        curl_easy_cleanup(curl);

} 
const std::map<std::string, std::string>* FeedlyProvider::getLabels(){
        curl_retrive("categories");

        Json::Reader reader;
        Json::Value root;

        bool parsingSuccesful;

        std::ifstream data(TEMP_PATH.c_str(), std::ifstream::binary);
        parsingSuccesful = reader.parse(data, root);

        if(data == NULL || curl_res != CURLE_OK || !parsingSuccesful || !root.isValidIndex(0)){
                std::cerr << "ERROR: Failed to Retrive Categories" << std::endl;
                return NULL;
        }
        user_data.categories["All"] = "user/" + user_data.id + "/category/global.all"; 
        user_data.categories["Saved"] = "user/" + user_data.id + "/tag/global.saved"; 
        user_data.categories["Uncategorized"] = "user/" + user_data.id + "/category/global.uncategorized"; 

        for(int i = 0; i < root.size(); i++)
                user_data.categories[(root[i]["label"]).asString()] = root[i]["id"].asString();

        return &(user_data.categories);
}
const std::vector<PostData>* FeedlyProvider::giveStreamPosts(const std::string& category){
        feeds.clear(); 

        if(category == "All")
                curl_retrive("streams/contents?ranked=newest&count=" + rtrv_count + "&unreadOnly=true&streamId=" + std::string(curl_easy_escape(curl, (user_data.categories["All"]).c_str(), 0)));
        else if(category == "Uncategorized")
                curl_retrive("streams/contents?ranked=newest&count=" + rtrv_count + "&unreadOnly=true&streamId=" + std::string(curl_easy_escape(curl, (user_data.categories["Uncategorized"]).c_str(), 0)));
        else if(category == "Saved")
                curl_retrive("streams/contents?ranked=newest&count=" + rtrv_count + "&unreadOnly=true&streamId=" + std::string(curl_easy_escape(curl, (user_data.categories["Saved"]).c_str(), 0)));
        else
                curl_retrive("streams/" + std::string(curl_easy_escape(curl, user_data.categories[category].c_str(), 0)) + "/contents?unreadOnly=true&ranked=newest&count=" + rtrv_count);

        Json::Reader reader;
        Json::Value root;

        bool parsingSuccesful;

        std::ifstream data(TEMP_PATH.c_str(), std::ifstream::binary);
        parsingSuccesful = reader.parse(data, root);

        if(data == NULL || curl_res != CURLE_OK || !parsingSuccesful || !root.isMember("items")){ 
                if(!parsingSuccesful)
                        std::cerr << "\nERROR: Failed to parse tokens file" << reader.getFormatedErrorMessages() << std::endl;
                if(curl_res != CURLE_OK)
                        fprintf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_strerror(curl_res));

                exit(EXIT_FAILURE);
                return NULL;
        }

        if(root["items"].size() == 0)
                return NULL;

        for(int i = 0; i < root["items"].size(); i++)
                feeds.push_back(PostData{root["items"][i]["summary"]["content"].asString(), root["items"][i]["title"].asString(), root["items"][i]["id"].asString(), root["items"][i]["originId"].asString()});

        data.close();

        return &(feeds);

}
bool FeedlyProvider::markPostsRead(const std::vector<std::string>* ids){
        FILE* data_holder = fopen(TEMP_PATH.c_str(), "wb");
        int i = 0;

        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "entries";

        for(std::vector<std::string>::const_iterator it = ids->begin(); it != ids->end(); ++it)
                array.append("entryIds") = *it;

        jsonCont["entryIds"] = array;
        jsonCont["action"] = "markAsRead";

        Json::StyledWriter writer;
        std::string document = writer.write(jsonCont); 

        curl = curl_easy_init();

        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        chunk = curl_slist_append(chunk, ("Authorization: OAuth " + user_data.authToken).c_str());

        enableVerbose();

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_URL, ("https://sandbox.feedly.com/v3/markers"));
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);
        curl_easy_setopt(curl, CURLOPT_POST, true);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, document.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        curl_res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        fclose(data_holder);
        if(curl_res != CURLE_OK){
                std::cerr << "Could not mark post(s) as read" << std::endl;
                return false;
        }

        return true;
}
bool FeedlyProvider::markPostsUnread(const std::vector<std::string>* ids){
        FILE* data_holder = fopen(TEMP_PATH.c_str(), "wb");
        int i = 0;

        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "entries";

        for(std::vector<std::string>::const_iterator it = ids->begin(); it != ids->end(); ++it)
                array.append("entryIds") = *it;

        jsonCont["entryIds"] = array;
        jsonCont["action"] = "keepUnread";

        Json::StyledWriter writer;
        std::string document = writer.write(jsonCont); 

        curl = curl_easy_init();

        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        chunk = curl_slist_append(chunk, ("Authorization: OAuth " + user_data.authToken).c_str());

        enableVerbose();

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_URL, ("https://sandbox.feedly.com/v3/markers"));
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);
        curl_easy_setopt(curl, CURLOPT_POST, true);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, document.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        curl_res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        fclose(data_holder);
        if(curl_res != CURLE_OK){
                std::cerr << "Could not mark post(s) as read" << std::endl;
                return false;
        }

        return true;
}
bool FeedlyProvider::markCategoriesRead(const std::string& id, const std::string& lastReadEntryId){
        FILE* data_holder = fopen(TEMP_PATH.c_str(), "wb");
        int i = 0;

        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "categories";

        array.append("categoryIds") = id;

        jsonCont["lastReadEntryId"] = lastReadEntryId;
        jsonCont["categoryIds"] = array;
        jsonCont["action"] = "markAsRead";

        Json::StyledWriter writer;
        std::string document = writer.write(jsonCont); 

        curl = curl_easy_init();

        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        chunk = curl_slist_append(chunk, ("Authorization: OAuth " + user_data.authToken).c_str());

        enableVerbose();

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_URL, (std::string(FEEDLY_URI) + "markers").c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);
        curl_easy_setopt(curl, CURLOPT_POST, true);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, document.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        curl_res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        fclose(data_holder);
        if(curl_res != CURLE_OK){
                std::cerr << "Could not mark post(s) as read" << std::endl;
                return false;
        }

        return false;
}
bool FeedlyProvider::addSubscription(bool newCategory, const std::string& feed, std::vector<std::string> categories, const std::string& title){
        FILE* data_holder = fopen(TEMP_PATH.c_str(), "wb");
        int i = 0;

        Json::Value jsonCont;
        Json::Value array;

        jsonCont["id"] = "feed/" + feed;
        jsonCont["title"] = title;

        if(categories.size() > 0){
                for(auto it = categories.begin(); it != categories.end(); ++it){
                        jsonCont["categories"][i]["id"] = user_data.categories[*it];
                        jsonCont["categories"][i]["label"] = *it;
                        i++;
                }
        }
        else
                jsonCont["categories"] = Json::Value(Json::arrayValue);

        Json::StyledWriter writer;
        std::string document = writer.write(jsonCont); 

        curl = curl_easy_init();

        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        chunk = curl_slist_append(chunk, ("Authorization: OAuth " + user_data.authToken).c_str());

        enableVerbose();

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_URL, (std::string(FEEDLY_URI) + "subscriptions").c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);
        curl_easy_setopt(curl, CURLOPT_POST, true);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, document.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        curl_res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        fclose(data_holder);
        if(curl_res != CURLE_OK){
                std::cerr << "Could not mark post(s) as read" << std::endl;
                return false;
        }

        return false;
}

PostData* FeedlyProvider::getSinglePostData(const int index){
        return &(feeds[index]);
}

const std::string FeedlyProvider::getUserId(){
        return user_data.id; 
}

void FeedlyProvider::getCookies(){
        FILE* data_holder = fopen(TEMP_PATH.c_str(), "wb");

        curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, std::string(GOOGLE_AUTH_URL).c_str()); 
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1 );
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, COOKIE_PATH.c_str());
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, COOKIE_PATH.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);

        enableVerbose();

        curl_res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        curl = curl_easy_init();

        extract_galx_value();
        curl_easy_setopt(curl, CURLOPT_REFERER, std::string(GOOGLE_AUTH_URL).c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, std::string("signIn=Sign+in&_utf8=&#9731;&bgresponse=js_disabled&GALX=" + user_data.galx + "&pstMsg=&dnCon=&checkConnection=youtube:1141:1&checkedDomains=youtube&Email=jorgemartinezhernandez&Passwd=trueforerunner117").c_str());

        curl_res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        fclose(data_holder);

}
void FeedlyProvider::enableVerbose(){
        if(verboseFlag)
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
}
void FeedlyProvider::setVerbose(bool value){
        verboseFlag = value;
}
void FeedlyProvider::extract_galx_value(){
        std::string l;
        std::ifstream temp(COOKIE_PATH.c_str());
        std::size_t index , last;

        if(temp.is_open()){
                while(getline(temp, l)){
                        index = l.rfind("GALX");

                        if(index > 0 && index != std::string::npos){
                                last = l.find("X") + 2;
                                break;
                        }
                } 
        }

        user_data.galx = l.substr(last);
        temp.close();
        return;
}
void FeedlyProvider::curl_retrive(const std::string& uri){
        struct curl_slist *chunk = NULL;

        FILE* data_holder = fopen(TEMP_PATH.c_str(), "wb");
        chunk = curl_slist_append(chunk, ("Authorization: OAuth " + user_data.authToken).c_str());

        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, (std::string(FEEDLY_URI) + uri).c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        enableVerbose();

        curl_res = curl_easy_perform(curl);

        fclose(data_holder);
        curl_easy_cleanup(curl);
}
void FeedlyProvider::echo(bool on = true){
        struct termios settings;
        tcgetattr( STDIN_FILENO, &settings );
        settings.c_lflag = on
                ? (settings.c_lflag |   ECHO )
                : (settings.c_lflag & ~(ECHO));
        tcsetattr( STDIN_FILENO, TCSANOW, &settings );
}
void FeedlyProvider::curl_cleanup(){
        curl_global_cleanup();

        user_data.categories.clear();
        feeds.clear();
        user_data.code = "";
        user_data.authToken = "";
        user_data.galx = "";
}
