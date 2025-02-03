#pragma once
#include <string>
#include <curl/curl.h>
#include <thread>
#include <atomic>
#include <fstream>
#include <chrono>
#include <map>
#include <vector>

#ifdef __APPLE__
    #define TTS_COMMAND "say"
#elif defined(_WIN32)
    #define TTS_COMMAND "powershell -Command \"Add-Type –AssemblyName System.Speech; (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak('"
#else
    #define TTS_COMMAND "espeak"
#endif

struct Settings {
    int voice_rate = 200;
    int auto_check_interval = 5;
    bool save_messages = true;
    std::string message_history_file = "message_history.txt";
};

std::string toLower(std::string str);

class RadioClient {
public:
    RadioClient(const std::string& serverUrl);
    ~RadioClient();
    
    // Create a new user
    bool createUser(const std::string& username);
    
    // Send message to specific user
    bool sendMessage(const std::string& toUser, const std::string& message);
    
    // Get latest message for a user
    bool getLatestMessage(const std::string& username, bool notify_if_unchanged = true);

    // Auto-check messages
    void startAutoCheck(const std::string& username);
    void stopAutoCheck();

    // Settings management
    void changeSettings();
    void loadSettings();
    void saveSettings();

    // Get all users
    std::vector<std::string> getUsers();

private:
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    void autoCheckMessages(const std::string& username);
    void saveMessageHistory(const std::string& username, const std::string& message, bool is_sent = false);
    std::string getCurrentTimestamp();
    
    CURL* curl;
    std::string serverUrl;
    std::string receivedData;
    std::string lastMessage;
    Settings settings;
    std::atomic<bool> autoCheck;
    std::thread autoCheckThread;
}; 