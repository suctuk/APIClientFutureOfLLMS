import requests
import platform
import os
import subprocess
import time
import threading
import json
from datetime import datetime

class RadioClient:
    def __init__(self, server_url):
        self.server_url = server_url
        self.auto_check = False
        self.auto_check_thread = None
        self.last_message = ""
        self.settings = self.load_settings()

    def load_settings(self):
        try:
            if os.path.exists('settings.json'):
                with open('settings.json', 'r') as f:
                    return json.load(f)
        except Exception as e:
            print(f"Error loading settings: {e}")
        return {
            "voice_rate": 200,
            "auto_check_interval": 5,
            "save_messages": True,
            "message_history_file": "message_history.txt"
        }

    def save_message_history(self, username, message, is_sent=False):
        if not self.settings["save_messages"]:
            return
            
        try:
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            with open(self.settings["message_history_file"], "a") as f:
                if is_sent:
                    f.write(f"[{timestamp}] Sent to {username}: {message}\n")
                else:
                    f.write(f"[{timestamp}] Received from {username}: {message}\n")
        except Exception as e:
            print(f"Error saving message history: {e}")

    def start_auto_check(self, username):
        """Start automatic message checking in background"""
        if not self.auto_check:
            self.auto_check = True
            self.auto_check_thread = threading.Thread(
                target=self._auto_check_messages,
                args=(username,),
                daemon=True
            )
            self.auto_check_thread.start()
            print("Auto-check messages enabled")

    def stop_auto_check(self):
        """Stop automatic message checking"""
        self.auto_check = False
        if self.auto_check_thread:
            self.auto_check_thread.join()
            print("Auto-check messages disabled")

    def _auto_check_messages(self, username):
        """Background thread function for auto-checking messages"""
        while self.auto_check:
            try:
                self.get_latest_message(username, notify_if_unchanged=False)
                time.sleep(self.settings["auto_check_interval"])
            except KeyboardInterrupt:
                print("\nStopping auto-check...")
                break
            except Exception as e:
                print(f"Error in auto-check: {e}")
                time.sleep(self.settings["auto_check_interval"])

    def create_user(self, username):
        try:
            response = requests.post(
                f"{self.server_url}/create_user",
                data={"username": username}
            )
            if response.status_code == 200:
                print("User created successfully!")
                return True
            else:
                print(f"Failed to create user. Error: {response.json().get('error', 'Unknown error')}")
                return False
        except Exception as e:
            print(f"Error creating user: {e}")
            return False

    def get_users(self):
        try:
            response = requests.get(f"{self.server_url}/users")
            if response.status_code == 200:
                return response.json().get('users', [])
            else:
                print(f"Failed to get users. Error: {response.json().get('error', 'Unknown error')}")
                return []
        except Exception as e:
            print(f"Error getting users: {e}")
            return []

    def send_message(self, to_user, message):
        try:
            # Convert to_user to lowercase for comparison
            recipient = to_user.lower()
            
            if recipient == 'all':
                print("Broadcasting message to all users...")
            
            response = requests.post(
                f"{self.server_url}/send_message",
                data={
                    "sendto": recipient,  # Use lowercase version
                    "message": message
                }
            )
            if response.status_code == 200:
                if recipient == 'all':
                    print("Message broadcast successfully!")
                else:
                    print("Message sent successfully!")
                self.save_message_history(to_user, message, is_sent=True)
                return True
            else:
                print(f"Failed to send message. Error: {response.json().get('error', 'Unknown error')}")
                return False
        except Exception as e:
            print(f"Error sending message: {e}")
            return False

    def get_latest_message(self, username, notify_if_unchanged=True):
        try:
            response = requests.get(
                f"{self.server_url}/latest_message",
                params={"username": username}
            )
            if response.status_code == 200:
                message = response.json().get('message', '')
                if message and message != self.last_message:
                    print(f"Latest message for {username}: {message}")
                    self.save_message_history(username, message)
                    self.text_to_speech(message)
                    self.last_message = message
                elif message and notify_if_unchanged:
                    print("No new messages.")
                return True
            else:
                print(f"Failed to get message. Error: {response.json().get('error', 'Unknown error')}")
                return False
        except Exception as e:
            print(f"Error getting message: {e}")
            return False

    def text_to_speech(self, message):
        system = platform.system()
        try:
            if system == "Darwin":  # macOS
                subprocess.run(["say", "-r", str(self.settings["voice_rate"]), message])
            elif system == "Windows":
                command = f'Add-Type -AssemblyName System.Speech; (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak("{message}")'
                subprocess.run(["powershell", "-Command", command])
            else:  # Linux and others
                subprocess.run(["espeak", "-s", str(self.settings["voice_rate"]), message])
        except Exception as e:
            print(f"Error in text-to-speech: {e}")

    def change_settings(self):
        """Allow user to modify client settings"""
        print("\nCurrent Settings:")
        for key, value in self.settings.items():
            print(f"{key}: {value}")
        
        setting = input("\nEnter setting to change (or 'done' to finish): ").lower()
        if setting in self.settings:
            new_value = input(f"Enter new value for {setting}: ")
            try:
                # Convert to int for numeric settings
                if isinstance(self.settings[setting], int):
                    new_value = int(new_value)
                elif isinstance(self.settings[setting], bool):
                    new_value = new_value.lower() == 'true'
                self.settings[setting] = new_value
                
                # Save settings to file
                with open('settings.json', 'w') as f:
                    json.dump(self.settings, f, indent=4)
                print("Settings updated successfully!")
            except ValueError:
                print("Invalid value type!")
        elif setting != 'done':
            print("Setting not found!")

def main():
    try:
        client = RadioClient("http://localhost:3000")
        
        username = input("Enter your username: ")
        if not client.create_user(username):
            return

        while True:
            try:
                print("\nOptions:")
                print("1. Send message")
                print("2. Check messages")
                print("3. Toggle auto-check messages")
                print("4. Change settings")
                print("5. Get users")
                print("6. Quit")
                choice = input("Choose an option (1-6): ")

                if choice == "1":
                    recipient = input("Enter recipient username (or 'all' to broadcast): ")
                    message = input("Enter message: ")
                    client.send_message(recipient, message)
                elif choice == "2":
                    client.get_latest_message(username)
                elif choice == "3":
                    if client.auto_check:
                        client.stop_auto_check()
                    else:
                        client.start_auto_check(username)
                elif choice == "4":
                    client.change_settings()
                elif choice == "5":
                    users = client.get_users()
                    print("Current users:", users)
                elif choice == "6":
                    client.stop_auto_check()
                    break
                else:
                    print("Invalid option. Please try again.")
            except KeyboardInterrupt:
                print("\nStopping client...")
                client.stop_auto_check()
                break
            except Exception as e:
                print(f"Error: {e}")
    except KeyboardInterrupt:
        print("\nExiting...")
    finally:
        if 'client' in locals():
            client.stop_auto_check()

if __name__ == "__main__":
    main() 