class FeedlyProvider{
		  public:
					 FeedlyProvider();
                const char* getAuthCode(){
					 void giveAll() const;
					 void giveUnread();
					 void giveSaved();
					 void giveTopics();
					 void giveLabels();
					 void searchUserFeeds();
					 void giveSingleEntry();
		  private:
					 CURL *curl;
					 CURLcode curl_res;
                #define FEEDLY_URI "https://sandbox.feedly.com/v3/"
                #define CLIENT_ID "client_id=sandbox"
                #define CLIENT_SECRET "client_secret=QNFQRFCFM1IQCJB367ON"
                #define REDIRECT_URI "redirect_uri=http://localhost"
                #define SCOPE "scope=https://cloud.feedly.com/subscriptions"
                #define RSPONSE_TYPE "code"
                static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream); };
