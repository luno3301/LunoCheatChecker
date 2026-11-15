#pragma once

#include "core/ScanResult.hpp"

#include <filesystem>

class SteamScanner {
public:
    ScanResult scan() const;

    ScanResult scan(const std::filesystem::path& steamDirectory) const;
    ScanResult parseLoginUsers(const std::filesystem::path& filePath) const;
};
