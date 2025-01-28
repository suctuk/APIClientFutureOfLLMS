#include "Client.hpp"
#include <iostream>
#include <sstream>

RadioClient::RadioClient(const std::string& url) : serverUrl(url) {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
}

RadioClient::~RadioClient() {
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

size_t RadioClient::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool RadioClient::createUser(const std::string& username) {
    if (!curl) return false;

    curl_easy_reset(curl);
    
    // Set URL for user creation
    std::string endpoint = serverUrl + "/create_user";
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    
    // Set POST data
    std::string postData = "username=" + username;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    
    // Set POST method
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    
    return (res == CURLE_OK);
}

bool RadioClient::sendMessage(const std::string& toUser, const std::string& message) {
    if (!curl) return false;

    curl_easy_reset(curl);
    
    // Set URL
    std::string endpoint = serverUrl + "/send_message";
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    
    // Set POST data
    std::string postData = "sendto=" + toUser + "&message=" + message;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    
    // Set POST method
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    
    return (res == CURLE_OK);
}

bool RadioClient::getLatestMessage(const std::string& username) {
    if (!curl) return false;

    curl_easy_reset(curl);
    
    // Set URL for receiving messages
    std::string endpoint = serverUrl + "/latest_message?username=" + username;
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    
    // Set callback function
    receivedData.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &receivedData);

    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        if (!receivedData.empty()) {
            size_t msgStart = receivedData.find(":\"");
            if (msgStart != std::string::npos) {
                msgStart += 2; // now message start is the index of the first character of the actual message
                std::string truncated = receivedData.substr(msgStart);
                size_t msgEnd = truncated.find("\"");
                if (msgEnd != std::string::npos) {
                    std::string message = truncated.substr(0, msgEnd);
                    if (!message.empty()) {
                        std::cout << "Latest message for " << username << ": " << message << std::endl;
                    }
                }
            }
        }
        return true;
    }
    
    return false;
} 
