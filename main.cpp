#include <iostream>
#include <string>
#include <Windows.h>
#include <cstdint>
#include <filesystem>
#include <regex>
#include <fstream>
#include <vector>
#include <unordered_map>

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

        // Ищем PersonaName
        if (std::regex_search(line, match, personaNamePattern)) {
            currentPersonaName = match[1].str();
        }

        // Ищем MostRecent
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
    char installPath[MAX_PATH];
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
        uint64_t steamID = std::stoull(str);
        steamData[steamID] = {"Pay Attention, Cleared Account!", false};
    }
    ParseSteamID(loginUsersPath, steamData);
    return steamData;
}

int main() {
    PGconn* conn = connectToDatabase();
    if (!conn) {
        return 1;
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
        const bool isModerator = ifUserModerator(conn, mainSteamID64);
        const bool isAdmin = ifUserAdmin(conn, mainSteamID64);
        Logger::info(std::string("Main user roles: moderator=") + (isModerator ? "true" : "false") +
                     ", admin=" + (isAdmin ? "true" : "false"));
        /*
    Request to DiscordAPI || Check if SteamID requested for RCC Connection 
    
    */
    }
    /*
    Request to DiscordAPI || Check if SteamID requested for RCC Connection 
    
    */
    /*
    
    PGresult *res = PQexec(conn, "SELECT * FROM users");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        PQclear(res);
        exitWithError(conn);
    }
    int rows = PQntuples(res);
    int cols = PQnfields(res);

    for (int i = 0; i < cols; i++) {
        std::cout << PQfname(res, i) << "\t";
    }
    std::cout << std::endl;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            std::cout << PQgetvalue(res, i, j) << "\t";
        }
        std::cout << std::endl;
    }

    PQclear(res);
    */

   
    PQfinish(conn);

    return 0;
}