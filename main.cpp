#include <iostream>
#include <libpq-fe.h>
#include <string>
#include <Windows.h>
#include <cstdint>
#include <filesystem>
#include <regex>
#include <fstream>
#include <set>
#include <vector>
#include <unordered_map>

namespace fs = std::filesystem;


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
            std::cerr << "Maybe M-acc, check Steam folder " << directoryPath << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.what() << std::endl;
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
                std::cerr << "Unexpected value type." << std::endl;
            }
        }
        else {
            std::cerr << "Failed to read InstallPath value." << std::endl;
        }
        RegCloseKey(hKey);
    }
    else {
        std::cerr << "Failed to open registry key." << std::endl;
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

void exitWithError(PGconn *conn) {
    std::cerr << "Error: " << PQerrorMessage(conn) << std::endl;    
    PQfinish(conn);
    exit(1);
}

int main() {
    const char *conninfo = "dbname=postgres user=postgres password=3301 host=localhost port=5432";
    PGconn *conn = PQconnectdb(conninfo);
    
    if (PQstatus(conn) != CONNECTION_OK) {
        exitWithError(conn);
    }
    
    std::cout << "Connected to the database successfully!" << std::endl;
    std::unordered_map<uint64_t, std::pair<std::string, bool>> steamData = GetSteamId();
    for (const auto& [steamID, data] : steamData) {
        const auto& [personaName, mostRecent] = data;
        std::cout << "SteamID: " << steamID
                  << ", PersonaName: " << personaName
                  << ", MostRecent: " << (mostRecent ? "true" : "false")
                  << std::endl;
    }
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
