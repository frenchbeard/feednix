#include <stdio.h>
#include <stdlib.h>
#include <jsoncpp/json/json.h>
#include <iterator>
#include <istream>

using namespace std;

#include "FeedlyProvider.h"

FeedlyProvider::FeedlyProvider(){
        curl_global_init(CURL_GLOBAL_DEFAULT);
        verboseFlag = false;
}

void FeedlyProvider::authenticateUser(const string& email, const string& passwd){
        getCookies();

        FILE* data_holder = fopen("temp.txt", "wb");

        curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, string(GOOGLE_AUTH_URL).c_str()); 
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1 );
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookie");
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookie");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);

        enableVerbose();

        curl_res = curl_easy_perform(curl);

        curl_easy_setopt(curl, CURLOPT_REFERER, string(GOOGLE_AUTH_URL).c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, string("signIn=Sign+in&_utf8=&#9731;&bgresponse=js_disabled&GALX=" + user_data.galx + "&pstMsg=&dnCon=&checkConnection=youtube:1141:1&checkedDomains=youtube&Email=jorgemartinezhernandez&Passwd=trueforerunner117").c_str());

        curl_res = curl_easy_perform(curl);

        char *currentURL;

        if(curl_res == CURLE_OK){
                curl_res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &currentURL);
                string tempCode(currentURL);

                if((curl_res == CURLE_OK) && currentURL){
                        unsigned codeIndex = tempCode.find("code=") + 5;
                        user_data.code = tempCode.substr(codeIndex, (tempCode.find("&", codeIndex) - codeIndex));

                        parseAuthenticationResponse();
                        return;
                }
        }
        else{
                cerr << "ERROR: Could not authenticate user" << endl;
                fprintf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_strerror(curl_res));
        }

        curl_easy_cleanup(curl);
        fclose(data_holder);
}

void FeedlyProvider::parseAuthenticationResponse(){
        struct curl_httppost *formpost = NULL;
        FILE *tokenJSON = fopen("tokenFile.json", "wb");

        curl = curl_easy_init();

        feedly_url = FEEDLY_URI + string("auth/token?code=") + user_data.code +  CLIENT_ID +  CLIENT_SECRET +  REDIRECT_URI + SCOPE + "&grant_type=authorization_code";

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

        ifstream tokenFile("tokenFile.json", ifstream::binary);

        if(tokenJSON != NULL && curl_res == CURLE_OK){
                parsingSuccesful = reader.parse(tokenFile, root);

                if(parsingSuccesful){
                        user_data.authToken = (root["access_token"]).asString();
                        user_data.refreshToken = (root["refresh_token"]).asString();
                        user_data.id = (root["id"]).asString();
                }
        }

        if(!parsingSuccesful)
                cerr << "ERROR: Failed to parse tokens file: " << reader.getFormatedErrorMessages() << endl;
        if(curl_res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_strerror(curl_res));

        curl_easy_cleanup(curl);
        system("rm temp.txt");
        system("rm tokenFile.json");
} 


const map<string, string>* FeedlyProvider::getLabels(){
        curl_retrive("categories");

        Json::Reader reader;
        Json::Value root;

        bool parsingSuccesful;

        ifstream data("temp.txt", ifstream::binary);
        parsingSuccesful = reader.parse(data, root);

        if(data == NULL || curl_res != CURLE_OK || !parsingSuccesful){
                cerr << "ERROR: Failed to Retrive Categories" << endl;
                return NULL;
        }

        for(int i = 0; i < root.size(); i++)
                user_data.categories[(root[i]["label"]).asString()] = root[i]["id"].asString();

        return &(user_data.categories);
}
const map<string, PostData>* FeedlyProvider::giveStreamPosts(const string& category){
        feeds.clear(); 

        if(!category.compare("All"))
                curl_retrive("streams/contents?unreadOnly=true&streamId=" + string(curl_easy_escape(curl, ("user/"+ user_data.id + "/category/global.all").c_str(), 0)));
        else
                curl_retrive("streams/" + string(curl_easy_escape(curl, user_data.categories[category].c_str(), 0)) + "/contents?unreadOnly=true&count=35");

        Json::Reader reader;
        Json::Value root;

        bool parsingSuccesful;

        ifstream data("temp.txt", ifstream::binary);
        parsingSuccesful = reader.parse(data, root);

        if(data == NULL || curl_res != CURLE_OK || !parsingSuccesful){ 
                cerr << "ERROR: Failed to Retrive Categories" << endl;
                return NULL;
        }

        for(int i = 0; i < root["items"].size(); i++){
                feeds[root["items"][i]["id"].asString()] = PostData{root["items"][i]["summary"]["content"].asString(), root["items"][i]["title"].asString()};
        }

        return &(feeds);

}
PostData* FeedlyProvider::getSinglePostData(const string& id){
        return &(feeds[id]);
}
void FeedlyProvider::extract_galx_value(){
        string l;
        ifstream temp("cookie");
        size_t index , last;

        if(temp.is_open()){
                while(getline(temp, l)){
                        index = l.rfind("GALX");

                        if(index > 0 && index != string::npos){
                                last = l.find("X") + 2;
                                break;
                        }
                } 
        }

        user_data.galx = l.substr(last);
        temp.close();
        return;
}

void FeedlyProvider::getCookies(){
        FILE* data_holder = fopen("temp.txt", "wb");

        curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, string(GOOGLE_AUTH_URL).c_str()); 
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1 );
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "cookie");
        curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "cookie");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);

        enableVerbose();

        curl_res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        curl = curl_easy_init();

        extract_galx_value();
        curl_easy_setopt(curl, CURLOPT_REFERER, string(GOOGLE_AUTH_URL).c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, string("signIn=Sign+in&_utf8=&#9731;&bgresponse=js_disabled&GALX=" + user_data.galx + "&pstMsg=&dnCon=&checkConnection=youtube:1141:1&checkedDomains=youtube&Email=jorgemartinezhernandez&Passwd=trueforerunner117").c_str());

        curl_res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        fclose(data_holder);

}

void FeedlyProvider::enableVerbose(){
        if(verboseFlag)
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
}

void FeedlyProvider::curl_retrive(const string& uri){
        struct curl_slist *chunk = NULL;

        FILE* data_holder = fopen("temp.txt", "wb");
        chunk = curl_slist_append(chunk, ("Authorization: OAuth " + user_data.authToken).c_str());

        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, (string(FEEDLY_URI) + uri).c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        enableVerbose();

        curl_res = curl_easy_perform(curl);

        fclose(data_holder);
}

void FeedlyProvider::curl_cleanup(){
        curl_global_cleanup();
}
