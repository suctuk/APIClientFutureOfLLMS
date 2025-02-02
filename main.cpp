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
            std::cout << "3. Quit\n";
            std::cout << "Choose an option (1-3): ";
            
            std::string choice;
            std::getline(std::cin, choice);
            
            if (choice == "1") {
                std::string recipient, message;
                std::cout << "Enter recipient username: ";
                std::getline(std::cin, recipient);
                std::cout << "Enter message: ";
                std::getline(std::cin, message);
                
                if (client.sendMessage(recipient, message)) {
                    std::cout << "Message sent successfully!" << std::endl;
                } else {
                    std::cout << "Failed to send message." << std::endl;
                }
            }
            else if (choice == "2") {
                client.getLatestMessage(username);
            }
            else if (choice == "3") {
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