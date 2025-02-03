#include "Client.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <thread>
#include <chrono>
#include <algorithm>  // for std::transform
#include <nlohmann/json.hpp>

using json = nlohmann::json;

RadioClient::RadioClient(const std::string& url) : serverUrl(url), autoCheck(false) {
    curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    loadSettings();
}

RadioClient::~RadioClient() {
    stopAutoCheck();
    if (curl) {
        curl_easy_cleanup(curl);
    }
}

void RadioClient::loadSettings() {
    try {
        std::ifstream f("settings.json");
        if (f.is_open()) {
            json j;
            f >> j;
            settings.voice_rate = j["voice_rate"];
            settings.auto_check_interval = j["auto_check_interval"];
            settings.save_messages = j["save_messages"];
            settings.message_history_file = j["message_history_file"];
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading settings: " << e.what() << std::endl;
    }
}

void RadioClient::saveSettings() {
    try {
        json j;
        j["voice_rate"] = settings.voice_rate;
        j["auto_check_interval"] = settings.auto_check_interval;
        j["save_messages"] = settings.save_messages;
        j["message_history_file"] = settings.message_history_file;
        
        std::ofstream f("settings.json");
        f << std::setw(4) << j << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error saving settings: " << e.what() << std::endl;
    }
}

std::string RadioClient::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void RadioClient::saveMessageHistory(const std::string& username, const std::string& message, bool is_sent) {
    if (!settings.save_messages) return;

    try {
        std::ofstream f(settings.message_history_file, std::ios::app);
        std::string timestamp = getCurrentTimestamp();
        if (is_sent) {
            f << "[" << timestamp << "] Sent to " << username << ": " << message << std::endl;
        } else {
            f << "[" << timestamp << "] Received from " << username << ": " << message << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error saving message history: " << e.what() << std::endl;
    }
}

void RadioClient::startAutoCheck(const std::string& username) {
    if (!autoCheck) {
        autoCheck = true;
        autoCheckThread = std::thread(&RadioClient::autoCheckMessages, this, username);
        std::cout << "Auto-check messages enabled" << std::endl;
    }
}

void RadioClient::stopAutoCheck() {
    if (autoCheck) {
        autoCheck = false;
        if (autoCheckThread.joinable()) {
            autoCheckThread.join();
        }
        std::cout << "Auto-check messages disabled" << std::endl;
    }
}

void RadioClient::autoCheckMessages(const std::string& username) {
    while (autoCheck) {
        getLatestMessage(username, false);
        std::this_thread::sleep_for(std::chrono::seconds(settings.auto_check_interval));
    }
}

void RadioClient::changeSettings() {
    std::cout << "\nCurrent Settings:\n";
    std::cout << "1. Voice rate: " << settings.voice_rate << std::endl;
    std::cout << "2. Auto-check interval: " << settings.auto_check_interval << std::endl;
    std::cout << "3. Save messages: " << (settings.save_messages ? "true" : "false") << std::endl;
    std::cout << "4. Message history file: " << settings.message_history_file << std::endl;

    std::string choice;
    std::cout << "\nEnter setting number to change (or 'done' to finish): ";
    std::getline(std::cin, choice);

    if (choice == "1") {
        std::cout << "Enter new voice rate: ";
        std::string value;
        std::getline(std::cin, value);
        settings.voice_rate = std::stoi(value);
    } else if (choice == "2") {
        std::cout << "Enter new auto-check interval (seconds): ";
        std::string value;
        std::getline(std::cin, value);
        settings.auto_check_interval = std::stoi(value);
    } else if (choice == "3") {
        std::cout << "Save messages (true/false): ";
        std::string value;
        std::getline(std::cin, value);
        settings.save_messages = (value == "true");
    } else if (choice == "4") {
        std::cout << "Enter new message history file path: ";
        std::getline(std::cin, settings.message_history_file);
    }

    saveSettings();
    std::cout << "Settings updated successfully!" << std::endl;
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
    
    // Set callback for response
    receivedData.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &receivedData);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    
    // Print request details for debugging
    std::cout << "Sending request to: " << endpoint << std::endl;
    std::cout << "POST data: " << postData << std::endl;
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }
    
    // Print response for debugging
    std::cout << "Server response: " << receivedData << std::endl;
    
    return true;
}

bool RadioClient::sendMessage(const std::string& toUser, const std::string& message) {
    if (!curl) return false;

    curl_easy_reset(curl);
    
    // Set URL
    std::string endpoint = serverUrl + "/send_message";
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    
    // Convert toUser to lowercase for comparison
    std::string recipient = toLower(toUser);
    
    // Set POST data
    std::string postData = "sendto=" + recipient + "&message=" + message;
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    
    // Set POST method
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    
    // Set callback for response
    receivedData.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &receivedData);
    
    // Set headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (recipient == "all") {
        std::cout << "Broadcasting message to all users..." << std::endl;
    }
    
    // Perform request
    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    
    if (res == CURLE_OK) {
        try {
            json response = json::parse(receivedData);
            if (response.contains("error")) {
                std::cout << "Error: " << response["error"] << std::endl;
                return false;
            }
            if (recipient == "all") {
                std::cout << "Message broadcast successfully!" << std::endl;
            } else {
                std::cout << "Message sent successfully!" << std::endl;
            }
            saveMessageHistory(toUser, message, true);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing response: " << e.what() << std::endl;
            return false;
        }
    }
    
    return false;
}

bool RadioClient::getLatestMessage(const std::string& username, bool notify_if_unchanged) {
    if (!curl) return false;

    curl_easy_reset(curl);
    
    std::string endpoint = serverUrl + "/latest_message?username=" + username;
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    
    receivedData.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &receivedData);

    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK) {
        if (!receivedData.empty()) {
            size_t msgStart = receivedData.find(":\"");
            if (msgStart != std::string::npos) {
                msgStart += 2;
                std::string truncated = receivedData.substr(msgStart);
                size_t msgEnd = truncated.find("\"");
                if (msgEnd != std::string::npos) {
                    std::string message = truncated.substr(0, msgEnd);
                    if (!message.empty() && message != lastMessage) {
                        std::cout << "Latest message for " << username << ": " << message << std::endl;
                        saveMessageHistory(username, message);
                        
                        std::string command;
                        #ifdef __APPLE__
                            command = "/usr/bin/say -r " + std::to_string(settings.voice_rate) + " \"" + message + "\"";
                        #elif defined(_WIN32)
                            command = TTS_COMMAND + message + "')\""
                        #else
                            command = TTS_COMMAND " -s " + std::to_string(settings.voice_rate) + " \"" + message + "\"";
                        #endif
                        
                        system(command.c_str());
                        lastMessage = message;
                    } else if (notify_if_unchanged && !message.empty()) {
                        std::cout << "No new messages." << std::endl;
                    }
                }
            }
        }
        return true;
    }
    
    return false;
}

std::vector<std::string> RadioClient::getUsers() {
    std::vector<std::string> users;
    if (!curl) return users;

    curl_easy_reset(curl);
    
    std::string endpoint = serverUrl + "/users";
    curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
    
    receivedData.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &receivedData);

    CURLcode res = curl_easy_perform(curl);
    
    if (res == CURLE_OK && !receivedData.empty()) {
        try {
            json j = json::parse(receivedData);
            if (j.contains("users")) {
                return j["users"].get<std::vector<std::string>>();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing users: " << e.what() << std::endl;
        }
    }
    
    return users;
}

// Helper function to convert string to lowercase
std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}
