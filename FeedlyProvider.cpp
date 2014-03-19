#include <stdio.h>
#include <stdlib.h>
#include <jsoncpp/json/json.h>
#include <vector>
#include <iterator>
#include <istream>
#include <sstream>

using namespace std;

#include "FeedlyProvider.h"

FeedlyProvider::FeedlyProvider(){

        curl_global_init(CURL_GLOBAL_DEFAULT);
        verboseFlag = false;
        system("rm cookie");
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
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, string("signIn=Sign+in&_utf8=&#9731;&bgresponse=js_disabled&GALX=" + extract_galx_value() + "&pstMsg=&dnCon=&checkConnection=youtube:1141:1&checkedDomains=youtube&Email=jorgemartinezhernandez&Passwd=trueforerunner117").c_str());

        curl_res = curl_easy_perform(curl);

        char *currentURL;

        if(curl_res == CURLE_OK){
                curl_res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &currentURL);
                string tempCode(currentURL);

                string code;

                if((curl_res == CURLE_OK) && currentURL){
                        unsigned codeIndex = tempCode.find("code=") + 5;
                        code = tempCode.substr(codeIndex, (tempCode.find("&", codeIndex) - codeIndex));
                        userData["userAuthCode"] = code;
                        curl_easy_cleanup(curl);
                        parseAuthenticationResponse();
                        return;
                }
        }

        cerr << "ERROR: Could not authenticate user" << endl;
        fprintf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_strerror(curl_res));

        curl_easy_cleanup(curl);
        fclose(data_holder);
}
void FeedlyProvider::parseAuthenticationResponse(){

        struct curl_httppost *formpost = NULL;
        FILE *tokenJSON = fopen("tokenFile.json", "wb");

        curl = curl_easy_init();

        feedly_url = FEEDLY_URI + string("auth/token?code=") + userData["userAuthCode"] +  CLIENT_ID +  CLIENT_SECRET +  REDIRECT_URI + SCOPE + "&grant_type=authorization_code";

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

        ifstream tokenFile;
        tokenFile.open("tokenFile.json");

        if(tokenJSON != NULL && curl_res == CURLE_OK){
                parsingSuccesful = reader.parse(tokenFile, root);
                if(parsingSuccesful){
                        userData["userAccessToken"] = (root["access_token"]).asString();
                        userData["userRefreshToken"] = (root["refresh_token"]).asString();
                        userData["userID"] = (root["id"]).asString();
                }
        }

        if(!parsingSuccesful)
                cerr << "ERROR: Failed to parse tokens file: " << reader.getFormatedErrorMessages() << endl;
        if(curl_res != CURLE_OK)
                fprintf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_strerror(curl_res));

        curl_easy_cleanup(curl);

        system("rm temp.txt");

} 
void FeedlyProvider::giveAllUnread(){
        struct curl_slist *chunk = NULL;
        FILE* data_holder = fopen("temp.txt", "wb");
        chunk = curl_slist_append(chunk, ("Authorization: OAuth " + userData["userAccessToken"]).c_str());

        curl = curl_easy_init();
        curl_easy_setopt(curl, CURLOPT_URL, (string(FEEDLY_URI) + "streams/contents?unreadOnly=true&streamId=" + curl_easy_escape(curl, ("user/"+ userData["userID"] + "/category/global.all").c_str(), 0)).c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        enableVerbose();

        curl_res = curl_easy_perform(curl);

        Json::Reader reader;
        Json::Value root;

        fclose(data_holder);

        bool parsingSuccesful;

        ifstream data;
        data.open("temp.txt");
        parsingSuccesful = reader.parse(data, root);

        if(data != NULL && curl_res == CURLE_OK && parsingSuccesful && root["items"].size() > 0){
                string id;

                for(int i = 0; i < root["items"].size(); i++){
                        id = (root["items"][i]["id"]).asString();

                        feeds[(root["items"][i]["id"]).asString()].content = root["items"][i]["summary"]["content"].asString();
                        feeds[(root["items"][i]["id"]).asString()].title = root["items"][i]["title"].asString();
                }
        }
        else{
                cerr << "ERROR: Failed to Retrive Feeds" << endl;
        }
}
void getProfile(){
        Json::Value root;
        Json::Reader reader;
}
string FeedlyProvider::extract_galx_value(){

        string l;
        ifstream temp;
        temp.open("cookie");
        int index;

        while(getline(temp, l)){
                index = l.find("GALX");
                if(index > 0 && index != string::npos){
                        unsigned last = l.find("X") + 2;
                        temp.close();
                        return l.substr(last);

                }
        } 

        temp.close();
        return l;
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

        curl_easy_setopt(curl, CURLOPT_REFERER, string(GOOGLE_AUTH_URL).c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, string("signIn=Sign+in&_utf8=&#9731;&bgresponse=js_disabled&GALX=" + extract_galx_value() + "&pstMsg=&dnCon=&checkConnection=youtube:1141:1&checkedDomains=youtube&Email=jorgemartinezhernandez&Passwd=trueforerunner117").c_str());

        curl_res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
}
void FeedlyProvider::enableVerbose(){
        if(verboseFlag)
                curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
}
