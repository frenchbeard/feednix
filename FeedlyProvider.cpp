#include <FeedlyProvider.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <curl/curl.h>
#include <libjson.h>

FeedlyProvider::FeedlyProvider(){
		  curl_global_init(CURL_GLOBAL_DEFAULT);
		  curl = curl_easy_init();
}

const char* FeedlyProvider::getAuthCode(){
		  //Feedly's Authentication Handler
		  curl_easy_setopt(curl, CURLOPT_URL, FEEDLY_URI+"/auth/auth?" +  CLIENT_ID +  REDIRECT_URI +  SCOPE + "&response_type=" + RESPONSE_TYPE);
		  //Follow OAuth Redirect
		  curl_easy_setopt(curl, CURLOPT_FOLLOWLCOATION, true);
		  //Set referer on Redirect
		  curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);

		  //Perfomr request
		  curl_res = curl_easy_perform(curl);

		  char *tokenURL;
		  //Chekc for errors
		  if(curl_res != CURL_OK){
					 printf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_sterror(res));
					 curl_easy_cleanup(curl);
		  }
		  else{
					 curl_res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &tokenURL);
					 string tempCode(tokenURL);

					 if((curl_res == CURL_OK) && url){
								printf("EFFECTIVE URL: %s", url);
                        const char *code = (tempCode.substr(tempCode.find("code=") + 5, tempCode.rfind("&")).c_str();
					 }

                curl_easy_cleanup(curl);
                return code;
		  }


}
const char* FeedlyProvider::getTokens(){
        struct curl_httppost *formpost = NULL;
        FILE *tokenJSON;
        //Authentication URI (POST /v3/auth/token)
        curl_easy_setopt(curl, CURLOPT_URL, FEEDLY_URI + "/auth/token?code=" + this.getAuthCode() +  CLIENT_ID +  CLIENT_SECRET +  REDIRECT_URI + SCOPE + "&gran_type=authorization_code");
        //Follow OAuth Redirect
        curl_easy_setopt(curl, CURLOPT_FOLLOWLCOATION, true);
        //Set referer on Redirect
        curl_easy_setopt(curl, CURLOPT_AUTOREFERER, true);
        //Required for access and refresh tokens
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
        //Write data functions
        curl_res = curl_easy_perform(curl);
 
        tokenJSON - fopen("tokenFile.json", "wb");
        if(tokenJSON == NULL){
                 curl_easy_cleanup(curl);
                 return -1;
        }

        //Perfomr request
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, WRITEDATA, tokenJSON);

        curl_res = curl_easy_perform(curl);

        if(curl_res != CURL_OK){
		           printf(stderr, "curl_easy_perform() failed : %s\n", curl_easy_sterror(res));
        }

        fclose(tokenJSON);
        curl_easy_cleanup(curl);

} 
void FeedlyProvider::giveAll(){
}
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream){
        int written = fwrite(ptr, size, mmemb, (FILE *)stream);
        return written;
}
