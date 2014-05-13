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
                }
        }
        else{
                cerr << "ERROR: Could not authenticate user" << endl;
                fprintf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_strerror(curl_res));
        }

        curl_easy_cleanup(curl);
        fclose(data_holder);
        system("rm tokenFile.json");
        system("rm temp.txt");
}
void FeedlyProvider::parseAuthenticationResponse(){
        struct curl_httppost *formpost = NULL;
        FILE *tokenJSON = fopen("tokenFile.json", "wb");

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
        user_data.categories["All"] = "user/" + user_data.id + "/category/global.all"; 
        user_data.categories["Saved"] = "user/" + user_data.id + "/tag/global.saved"; 
        user_data.categories["Uncategorized"] = "user/" + user_data.id + "/category/global.uncategorized"; 

        for(int i = 0; i < root.size(); i++)
                user_data.categories[(root[i]["label"]).asString()] = root[i]["id"].asString();

        return &(user_data.categories);
}
const vector<PostData>* FeedlyProvider::giveStreamPosts(const string& category){
        feeds.clear(); 

        if(category == "All")
                curl_retrive("streams/contents?ranked=newest&count=" + to_string(DEFAULT_FCOUNT) + "&unreadOnly=true&streamId=" + string(curl_easy_escape(curl, ("user/"+ user_data.id + "/category/global.all").c_str(), 0)));
        else if(category == "Uncategorized")
                curl_retrive("streams/contents?ranked=newest&count=" + to_string(DEFAULT_FCOUNT) + "&unreadOnly=true&streamId=" + string(curl_easy_escape(curl, ("user/"+ user_data.id + "/category/global.uncategorized").c_str(), 0)));
        else
                curl_retrive("streams/" + string(curl_easy_escape(curl, user_data.categories[category].c_str(), 0)) + "/contents?unreadOnly=true&ranked=newest&count=" + to_string(DEFAULT_FCOUNT));

        Json::Reader reader;
        Json::Value root;

        bool parsingSuccesful;

        ifstream data("temp.txt", ifstream::binary);
        parsingSuccesful = reader.parse(data, root);

        if(data == NULL || curl_res != CURLE_OK || !parsingSuccesful){ 
                cerr << "ERROR: Failed to Retrive Categories" << endl;
                return NULL;
        }

        if(root["items"].size() == 0)
                return NULL;

        for(int i = 0; i < root["items"].size(); i++)
                feeds.push_back(PostData{root["items"][i]["summary"]["content"].asString(), root["items"][i]["title"].asString(), root["items"][i]["id"].asString()});

        data.close();

        return &(feeds);

}
bool FeedlyProvider::markPostsRead(const vector<string>* ids){
        FILE* data_holder = fopen("temp.txt", "wb");
        int i = 0;

        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "entries";

        for(std::vector<string>::const_iterator it = ids->begin(); it != ids->end(); ++it)
                array.append("entryIds") = *it;

        jsonCont["entryIds"] = array;
        jsonCont["action"] = "markAsRead";

        Json::StyledWriter writer;
        string document = writer.write(jsonCont); 
        system(string("echo \'" + document + "\' > check").c_str());

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
        if(curl_res == CURLE_OK){
                return true;
        }
        else{
                cerr << "Could not mark post(s) as read" << endl;
                return false;
        }

        curl_easy_cleanup(curl);

        fclose(data_holder);
        return false;
}
bool FeedlyProvider::markCategoriesRead(const string& id, const string& lastReadEntryId){
        FILE* data_holder = fopen("temp.txt", "wb");
        int i = 0;

        Json::Value jsonCont;
        Json::Value array;

        jsonCont["type"] = "categories";

        array.append("categoryIds") = id;

        jsonCont["lastReadEntryId"] = lastReadEntryId;
        jsonCont["categoryIds"] = array;
        jsonCont["action"] = "markAsRead";

        Json::StyledWriter writer;
        string document = writer.write(jsonCont); 
        system(string("echo \'" + document + "\' > check").c_str());

        curl = curl_easy_init();

        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
        chunk = curl_slist_append(chunk, ("Authorization: OAuth " + user_data.authToken).c_str());

        enableVerbose();

        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0");
        curl_easy_setopt(curl, CURLOPT_URL, (string(FEEDLY_URI) + "markers").c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, data_holder);
        curl_easy_setopt(curl, CURLOPT_POST, true);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, document.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

        curl_res = curl_easy_perform(curl);
        if(curl_res == CURLE_OK){
                return true;
        }
        else{
                cerr << "Could not mark post(s) as read" << endl;
                return false;
        }

        curl_easy_cleanup(curl);

        fclose(data_holder);
        return false;
}

PostData* FeedlyProvider::getSinglePostData(const int index){
        return &(feeds[index]);
}

const string FeedlyProvider::getUserId(){
        return user_data.id; 
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
        curl_easy_cleanup(curl);
}
void FeedlyProvider::curl_cleanup(){
        curl_global_cleanup();
}
