#include <iostream>
#include <string>
#include <Windows.h>
#include <cstdint>
#include <filesystem>
#include <regex>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "logger.h"
#include "database.h"
#include "system.h"

namespace fs = std::filesystem;

bool ifUserCheckActive(uint64_t steamid64) {
    // Conection to API TODO
    return true;
}

void ParseSteamID(const std::string& filePath, std::unordered_map<uint64_t, std::pair<std::string, bool>>& steamData) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    std::string line;
    std::regex steamIDPattern(R"(\b7656\d+\b)");
    std::regex personaNamePattern("\"PersonaName\"\\s+\"([^\"]+)\"");
    std::regex mostRecentPattern("\"MostRecent\"\\s+\"(\\d)\"");

    uint64_t currentSteamID = 0;
    std::string currentPersonaName;
    bool currentMostRecent = false;

    while (std::getline(file, line)) {
        std::smatch match;

        if (std::regex_search(line, match, steamIDPattern)) {
            if (currentSteamID != 0 && !currentPersonaName.empty()) {
                steamData[currentSteamID] = {currentPersonaName, currentMostRecent};
            }
            currentSteamID = std::stoull(match.str());
            currentPersonaName.clear();
            currentMostRecent = false;
        }

        if (std::regex_search(line, match, personaNamePattern)) {
            currentPersonaName = match[1].str();
        }

        if (std::regex_search(line, match, mostRecentPattern)) {
            currentMostRecent = (match[1].str() == "1");
        }
    }

    if (currentSteamID != 0 && !currentPersonaName.empty()) {
        steamData[currentSteamID] = {currentPersonaName, currentMostRecent};
    }

    file.close();
}
void GetDirectoryFiles(const std::string& directoryPath, bool splitString, std::vector<std::string>& data) {
    try {
        fs::path path(directoryPath);
        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (fs::is_regular_file(entry)) {
                    if (splitString) {
                        data.push_back(entry.path().stem().string());
                    } else {
                        data.push_back(entry.path().filename().string());
                    }
                }
            }
        } else {
            Logger::warning(std::string("Maybe M-acc, check Steam folder ") + directoryPath);
        }
    } catch (const fs::filesystem_error& e) {
        Logger::error(std::string("Error accessing directory: ") + e.what());
    }
}
std::unordered_map<uint64_t, std::pair<std::string, bool>> GetSteamId() {
    HKEY hKey;
    const char* subKey = "SOFTWARE\\WOW6432Node\\Valve\\Steam";
    const char* valueName = "InstallPath";
    std::unordered_map<uint64_t, std::pair<std::string, bool>> steamData;
    char installPath[MAX_PATH] = {};
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD bufferSize = sizeof(installPath);
        DWORD valueType;

        if (RegQueryValueExA(hKey, valueName, NULL, &valueType, (LPBYTE)installPath, &bufferSize) == ERROR_SUCCESS) {
            if (valueType == REG_SZ)
            {
                std::cout << installPath << std::endl;
            }
            else {
                Logger::error("Unexpected value type for InstallPath.");
            }
        }
        else {
            Logger::error("Failed to read InstallPath value.");
        }
        RegCloseKey(hKey);
    }
    else {
        Logger::error("Failed to open registry key.");
    }
    std::string loginUsersPath = (std::string) std::string(installPath) + "\\config\\loginusers.vdf";
    std::string directoryPath = (std::string) std::string(installPath) + "\\config\\avatarcache";
    std::vector <std::string> data;
    GetDirectoryFiles(directoryPath, 1, data);  
    for (const std::string& str : data) {
        try {
            uint64_t steamID = std::stoull(str);
            steamData[steamID] = {"Pay Attention, Cleared Account!", false};
        }
        catch (const std::invalid_argument& e) {
            Logger::warning(std::string("Invalid SteamID in avatarcache: ") + str);
        }
        catch (const std::out_of_range& e) {
            Logger::warning(std::string("SteamID out of range in avatarcache: ") + str);
        }
    }
    ParseSteamID(loginUsersPath, steamData);
    return steamData;
}

void PrintMainMenu() {
    std::cout << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << "      Luno Cheat Checker      " << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << "1. Start check" << std::endl;
    std::cout << "2. Moderator authorization" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Select action: ";
}

void PrintModMenu(uint64_t moderatorSteamID64) {
    std::cout << std::endl; 
    std::cout << "==============================" << std::endl;
    std::cout << "      Welcome " + std::to_string(moderatorSteamID64) << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << "1. Check player bans" << std::endl;
    std::cout << "2. Ban player" << std::endl;
    std::cout << "3. Unban Player" << std::endl;
    std::cout << "4. My Stats" << std::endl;
    std::cout << "0. Exit" << std::endl;
    std::cout << "Select action: ";
}

bool ReadSteamID64(uint64_t& steamid64, const std::string& prompt) {
    std::cout << prompt;

    std::string input;
    if (!std::getline(std::cin, input)) {
        Logger::warning("Input closed while reading SteamID64.");
        return false;
    }

    const std::regex steamIDPattern(R"(^\s*7656\d+\s*$)");
    if (!std::regex_match(input, steamIDPattern)) {
        Logger::warning("Invalid SteamID64 format.");
        return false;
    }

    try {
        steamid64 = std::stoull(input);
    }
    catch (const std::exception& e) {
        Logger::error(std::string("Failed to parse SteamID64: ") + e.what());
        return false;
    }

    return true;
}

bool ReadPassword(std::string& password) {
    std::cout << "Enter moderator password: ";

    HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    DWORD originalMode = 0;
    const bool consoleModeAvailable = GetConsoleMode(inputHandle, &originalMode);

    if (consoleModeAvailable) {
        SetConsoleMode(inputHandle, originalMode & ~ENABLE_ECHO_INPUT);
    }

    const bool readOk = static_cast<bool>(std::getline(std::cin, password));

    if (consoleModeAvailable) {
        SetConsoleMode(inputHandle, originalMode);
    }

    std::cout << std::endl;

    if (!readOk) {
        Logger::warning("Input closed while reading moderator password.");
        return false;
    }

    if (password.empty()) {
        Logger::warning("Moderator password cannot be empty.");
        return false;
    }

    return true;
}

bool RunSteamCheck() {  
    PGconn* conn = connectToDatabase();
    if (!conn) {
        return false;
    }

    std::unordered_map<uint64_t, std::pair<std::string, bool>> steamData = GetSteamId();
    std::string macAddress = getMacAddress();
    if (macAddress.empty()) {
        Logger::warning("Could not retrieve MAC address.");
    }

    uint64_t mainSteamID64 = 0;
    for (const auto& [steamID, data] : steamData) {
        const auto& [personaName, mostRecent] = data;
        if (mostRecent) {
            mainSteamID64 = steamID;
        }
        std::cout << "SteamID: " << steamID
                  << ", PersonaName: " << personaName
                  << ", MostRecent: " << (mostRecent ? "true" : "false")
                  << std::endl;
        saveSteamUser(conn, steamID, personaName, mostRecent, macAddress);
    }

    if (mainSteamID64 == 0) {
        Logger::warning("Main SteamID (MostRecent) was not found.");
    }

    if (ifUserCheckActive(mainSteamID64)) {
        const bool isStaff = ifUserStaff(conn, mainSteamID64);
        Logger::info(std::string("Main user roles: staff=") + (isStaff ? "true" : "false"));
        /*
    Request to DiscordAPI || Check if SteamID requested for RCC Connection 
    
    */
    }
    
    PQfinish(conn);

    return true;
}
void CheckPlayerBans(PGconn* conn, uint64_t moderatorSteamID64);
void BanPlayer(PGconn* conn, int moderatorStaffId);
void UnbanPlayer(PGconn* conn, int moderatorStaffId);
void ShowMyStats(PGconn* conn, int moderatorStaffId, uint64_t moderatorSteamID64);

void RunModeratorMenu(PGconn* conn, int moderatorStaffId, uint64_t moderatorSteamID64) {
    while(1){
        PrintModMenu(moderatorSteamID64);
        std::string choice;
        if (!std::getline(std::cin, choice)) {
            Logger::warning("Input closed.");
            break;
        }
        else if(choice == "1") {
            CheckPlayerBans(conn, moderatorSteamID64);
        }
        else if(choice == "2") {
            BanPlayer(conn, moderatorStaffId);
        }
        else if(choice == "3") {
            UnbanPlayer(conn, moderatorStaffId);
        }
        else if(choice == "4") {
            ShowMyStats(conn, moderatorStaffId, moderatorSteamID64);
        }
        else if (choice == "0") {
            Logger::info("ModMenu closed.");
            return;
        }
        else {
            Logger::warning("Unknown moderator menu action: " + choice);
            std::cout << "Unknown action. Try again." << std::endl;
        }
    }
}
void CheckPlayerBans(PGconn* conn, uint64_t moderatorSteamID64) {}
void BanPlayer(PGconn* conn, int moderatorStaffId) {
    uint64_t playerSteamID64 = 0;
    if (!ReadSteamID64(playerSteamID64, "Enter player SteamID64: ")) {
        return;
    }

    std::string reason;
    std::cout << "Enter ban reason: ";
    if (!std::getline(std::cin, reason)) {
        Logger::warning("Input closed while reading ban reason.");
        return;
    }

    if (reason.empty()) {
        Logger::warning("Ban reason cannot be empty.");
        std::cout << "Ban reason cannot be empty." << std::endl;
        return;
    }

    std::string evidence;
    std::cout << "Enter evidence (optional): ";
    if (!std::getline(std::cin, evidence)) {
        Logger::warning("Input closed while reading ban evidence.");
        return;
    }

    const bool created = createBan(conn, playerSteamID64, moderatorStaffId, reason, evidence);
    if (created) {
        std::cout << "Ban created." << std::endl;
    }
    else {
        std::cout << "Failed to create ban." << std::endl;
    }
}
void UnbanPlayer(PGconn* conn, int moderatorStaffId) {}
void ShowMyStats(PGconn* conn, int moderatorStaffId, uint64_t moderatorSteamID64) {}


void AuthorizeModerator() {
    uint64_t moderatorSteamID64 = 0;
    if (!ReadSteamID64(moderatorSteamID64, "Enter moderator SteamID64: ")) {
        return;
    }

    std::string password;
    if (!ReadPassword(password)) {
        return;
    }

    PGconn* conn = connectToDatabase();
    if (!conn) {
        std::fill(password.begin(), password.end(), '\0');
        return;
    }

    int moderatorStaffId = 0;
    const bool authorized = authorizeModerator(conn, moderatorSteamID64, password, moderatorStaffId);
    std::fill(password.begin(), password.end(), '\0');
    password.clear();

    if (authorized) {
        Logger::info("Moderator authorization successful for SteamID64: " + std::to_string(moderatorSteamID64));
        std::cout << "Moderator authorized." << std::endl;
        RunModeratorMenu(conn, moderatorStaffId, moderatorSteamID64);
    }
    else {
        Logger::warning("Moderator authorization failed for SteamID64: " + std::to_string(moderatorSteamID64));
        std::cout << "Access denied." << std::endl;
    }

    PQfinish(conn);
}


int main() {
    while (true) {
        PrintMainMenu();

        std::string choice;
        if (!std::getline(std::cin, choice)) {
            Logger::warning("Input closed.");
            break;
        }

        if (choice == "1") {
            RunSteamCheck();
        }
        else if (choice == "2") {
            AuthorizeModerator();
        }
        else if (choice == "0") {
            Logger::info("Application closed.");
            break;
        }
        else {
            Logger::warning("Unknown menu action: " + choice);
            std::cout << "Unknown action. Try again." << std::endl;
        }
    }

    return 0;
}
