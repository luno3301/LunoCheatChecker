#include "scanner/SteamScanner.hpp"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <iterator>
#include <regex>
#include <system_error>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

bool parseSteamId(const std::string& text, std::uint64_t& steamId) {
    const auto parsed = std::from_chars(text.data(), text.data() + text.size(), steamId);
    return parsed.ec == std::errc{} && parsed.ptr == text.data() + text.size()
        && text.compare(0, 4, "7656") == 0;
}

void mergeAccount(std::vector<SteamAccount>& accounts, const SteamAccount& account) {
    const auto existing = std::find_if(accounts.begin(), accounts.end(), [&](const SteamAccount& item) {
        return item.steamId == account.steamId;
    });
    if (existing == accounts.end()) {
        accounts.push_back(account);
    } else {
        *existing = account;
    }
}

std::string unescapeName(const std::string& text) {
    std::string name;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\\' && i + 1 < text.size()
            && (text[i + 1] == '\\' || text[i + 1] == '"')) {
            ++i;
        }
        name += text[i];
    }
    return name;
}

void readAvatarCache(const std::filesystem::path& directory, ScanResult& result) {
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directory)) {
            if (!entry.is_regular_file()) {
                continue;
            }
            const auto stem = entry.path().stem().string();
            std::uint64_t steamId = 0;
            if (!parseSteamId(stem, steamId)) {
                result.warnings.push_back("Invalid SteamID in avatarcache: " + stem);
                continue;
            }
            mergeAccount(result.accounts, {steamId, "Pay Attention, Cleared Account!", false});
        }
    } catch (const std::filesystem::filesystem_error& error) {
        result.warnings.push_back(std::string("Could not read avatarcache: ") + error.what());
    }
}

} // namespace

ScanResult SteamScanner::parseLoginUsers(const std::filesystem::path& filePath) const {
    ScanResult result;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.warnings.push_back("Failed to open file: " + filePath.string());
        return result;
    }

    // Account IDs must be keys, never numbers occurring inside a persona name.
    const std::regex steamIdPattern(R"vdf(^\s*"([0-9]+)"\s*\{?\s*$)vdf");
    const std::regex fieldPattern(R"vdf(^\s*"(PersonaName|MostRecent)"\s+"((?:\\.|[^"\\])*)")vdf");
    const std::regex closingBracePattern(R"(^\s*\}\s*(?://.*)?$)");

    SteamAccount current{};
    bool hasPersonaName = false;
    const auto flushAccount = [&] {
        if (current.steamId != 0 && hasPersonaName) {
            mergeAccount(result.accounts, current);
        }
        current = {};
        hasPersonaName = false;
    };

    std::string line;
    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_match(line, match, steamIdPattern)) {
            flushAccount();
            const auto text = match[1].str();
            if (!parseSteamId(text, current.steamId)) {
                current.steamId = 0;
                result.warnings.push_back("Invalid SteamID in loginusers.vdf: " + text);
            }
        } else if (current.steamId != 0 && std::regex_search(line, match, fieldPattern)) {
            if (match[1].str() == "PersonaName") {
                current.personaName = unescapeName(match[2].str());
                hasPersonaName = true;
            } else {
                current.mostRecent = (match[2].str() == "1");
            }
        } else if (std::regex_match(line, closingBracePattern)) {
            flushAccount();
        }
    }
    flushAccount();

    if (file.bad()) {
        result.warnings.push_back("Failed to read file: " + filePath.string());
        return result;
    }
    result.completed = true;
    return result;
}

ScanResult SteamScanner::scan(const std::filesystem::path& steamDirectory) const {
    ScanResult result;
    if (steamDirectory.empty()) {
        result.warnings.push_back("Steam installation path is empty.");
        return result;
    }

    const auto config = steamDirectory / "config";
    readAvatarCache(config / "avatarcache", result);
    auto loginUsers = parseLoginUsers(config / "loginusers.vdf");
    for (const auto& account : loginUsers.accounts) {
        mergeAccount(result.accounts, account);
    }
    result.warnings.insert(result.warnings.end(),
        std::make_move_iterator(loginUsers.warnings.begin()),
        std::make_move_iterator(loginUsers.warnings.end()));
    result.completed = loginUsers.completed;

    std::sort(result.accounts.begin(), result.accounts.end(), [](const SteamAccount& a, const SteamAccount& b) {
        return a.steamId < b.steamId;
    });
    return result;
}

ScanResult SteamScanner::scan() const {
    ScanResult result;
#ifdef _WIN32
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Valve\\Steam", 0,
                     KEY_READ | KEY_WOW64_32KEY, &key) != ERROR_SUCCESS) {
        result.warnings.push_back("Failed to open Steam registry key.");
        return result;
    }

    DWORD type = 0;
    DWORD size = 0;
    auto status = RegQueryValueExW(key, L"InstallPath", nullptr, &type, nullptr, &size);
    std::vector<wchar_t> path(size / sizeof(wchar_t) + 1, L'\0');
    if (status == ERROR_SUCCESS && type == REG_SZ) {
        status = RegQueryValueExW(key, L"InstallPath", nullptr, &type,
                                 reinterpret_cast<LPBYTE>(path.data()), &size);
    }
    RegCloseKey(key);

    if (status != ERROR_SUCCESS || type != REG_SZ || path.front() == L'\0') {
        result.warnings.push_back("Failed to read Steam InstallPath value.");
        return result;
    }
    return scan(std::filesystem::path(path.data()));
#else
    result.warnings.push_back("Steam registry discovery is only available on Windows; specify a Steam directory.");
    return result;
#endif
}
