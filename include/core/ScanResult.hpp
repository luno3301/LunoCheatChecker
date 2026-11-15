#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct SteamAccount {
    std::uint64_t steamId;
    std::string personaName;
    bool mostRecent = false;
};

struct ScanResult {
    std::vector<SteamAccount> accounts;
    std::vector<std::string> warnings;
    bool completed = false;
};
