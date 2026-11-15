#include "core/Logger.hpp"
#include "core/System.hpp"
#include "database/Database.hpp"
#include "scanner/SteamScanner.hpp"

#include <windows.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string_view>

namespace {

struct Options {
    std::filesystem::path steamDirectory;
    bool save = false;
    bool help = false;
};

void printHelp() {
    std::cout
        << "LunoScanner\n"
        << "Usage: LunoScanner.exe [--steam-path <directory>] [--save]\n\n"
        << "  --steam-path <directory>  Scan this Steam folder instead of using the registry.\n"
        << "  --save                    Save collected accounts to PostgreSQL (DB_CONNINFO).\n"
        << "  --help, -h                Show this help.\n";
}

bool parseOptions(int argc, wchar_t* argv[], Options& options) {
    for (int i = 1; i < argc; ++i) {
        const std::wstring_view argument(argv[i]);
        if (argument == L"--help" || argument == L"-h") {
            options.help = true;
        } else if (argument == L"--save") {
            options.save = true;
        } else if (argument == L"--steam-path") {
            if (i + 1 >= argc || *argv[i + 1] == L'\0'
                || std::wstring_view(argv[i + 1]).substr(0, 2) == L"--") {
                std::cerr << "--steam-path requires a Steam directory.\n";
                return false;
            }
            options.steamDirectory = argv[++i];
        } else {
            std::cerr << "Unknown argument. Use --help for available options.\n";
            return false;
        }
    }
    return true;
}

void printResult(const ScanResult& result) {
    for (const auto& warning : result.warnings) {
        Logger::warning(warning);
    }
    for (const auto& account : result.accounts) {
        std::cout << "SteamID64: " << account.steamId
                  << " | PersonaName: " << account.personaName
                  << " | MostRecent: " << (account.mostRecent ? "true" : "false") << '\n';
    }
    std::cout << "Accounts: " << result.accounts.size()
              << " | Warnings: " << result.warnings.size()
              << " | Completed: " << (result.completed ? "yes" : "no") << '\n';
}

bool saveAccounts(const ScanResult& result) {
    if (result.accounts.empty()) {
        Logger::warning("No accounts to save.");
        return false;
    }

    using Connection = std::unique_ptr<PGconn, decltype(&PQfinish)>;
    Connection connection(connectToDatabase(), &PQfinish);
    if (!connection) {
        return false;
    }
    const auto macAddress = getMacAddress();
    if (macAddress.empty()) {
        Logger::warning("Could not retrieve MAC address.");
    }

    std::size_t saved = 0;
    for (const auto& account : result.accounts) {
        if (saveSteamUser(connection.get(), account.steamId, account.personaName,
                          account.mostRecent, macAddress)) {
            ++saved;
        }
    }
    std::cout << "Saved accounts: " << saved << '/' << result.accounts.size() << '\n';
    return saved == result.accounts.size();
}

} // namespace

int wmain(int argc, wchar_t* argv[]) {
    SetConsoleOutputCP(CP_UTF8);
    try {
        Options options;
        if (!parseOptions(argc, argv, options)) {
            return 2;
        }
        if (options.help) {
            printHelp();
            return 0;
        }

        const SteamScanner scanner;
        const auto result = options.steamDirectory.empty()
            ? scanner.scan() : scanner.scan(options.steamDirectory);
        printResult(result);
        const bool saved = !options.save || saveAccounts(result);
        return result.completed && saved ? 0 : 1;
    } catch (const std::exception& error) {
        Logger::error(error.what());
        return 1;
    }
}
