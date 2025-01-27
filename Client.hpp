#pragma once
#include <string>
#include <curl/curl.h>

class RadioClient {
public:
    RadioClient(const std::string& serverUrl);
    ~RadioClient();
    
    // Create a new user
    bool createUser(const std::string& username);
    
    // Send message to specific user
    bool sendMessage(const std::string& toUser, const std::string& message);
    
    // Get latest message for a user
    bool getLatestMessage(const std::string& username);

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    CURL* curl;
    std::string serverUrl;
    std::string receivedData;
}; 