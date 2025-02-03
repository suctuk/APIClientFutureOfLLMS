#include "Client.hpp"
#include <iostream>
#include <string>

int main() {
    try {
        // RadioClient client("https://apiclientfutureofllms.onrender.com");
        RadioClient client("http://localhost:3000"); 
        // First, create a user
        std::string username;
        std::cout << "Enter your username: ";
        std::getline(std::cin, username);
        
        if (!client.createUser(username)) {
            std::cout << "Failed to create user." << std::endl;
            return 1;
        }
        
        std::cout << "User created successfully!" << std::endl;
        
        while (true) {
            std::cout << "\nOptions:\n";
            std::cout << "1. Send message\n";
            std::cout << "2. Check messages\n";
            std::cout << "3. Toggle auto-check messages\n";
            std::cout << "4. Change settings\n";
            std::cout << "5. Quit\n";
            std::cout << "Choose an option (1-5): ";
            
            std::string choice;
            std::getline(std::cin, choice);
            
            if (choice == "1") {
                std::string recipient, message;
                std::cout << "Enter recipient username (or 'all' to broadcast): ";
                std::getline(std::cin, recipient);
                std::cout << "Enter message: ";
                std::getline(std::cin, message);
                client.sendMessage(recipient, message);
            }
            else if (choice == "2") {
                client.getLatestMessage(username);
            }
            else if (choice == "3") {
                static bool autoCheckEnabled = false;
                if (autoCheckEnabled) {
                    client.stopAutoCheck();
                } else {
                    client.startAutoCheck(username);
                }
                autoCheckEnabled = !autoCheckEnabled;
            }
            else if (choice == "4") {
                client.changeSettings();
            }
            else if (choice == "5") {
                break;
            }
            else {
                std::cout << "Invalid option. Please try again." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 