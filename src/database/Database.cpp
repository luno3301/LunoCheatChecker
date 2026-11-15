#include "database/Database.hpp"
#include "core/Logger.hpp"

#include <cstdlib>

void exitWithError(PGconn* conn) {
    Logger::error(std::string("Database error: ") + PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
}

static std::string getConnectionString() {
    const char* value = std::getenv("DB_CONNINFO");

    if (value == nullptr || *value == '\0') {
        Logger::error("DB_CONNINFO is not configured");
        return {};
    }

    return std::string(value);
}

PGconn* connectToDatabase() {
    const auto conninfo = getConnectionString();

    if (conninfo.empty()) {
        return nullptr;
    }

    PGconn* conn = PQconnectdb(conninfo.c_str());

    if (conn == nullptr) {
        Logger::error("Failed to allocate DB connection");
        return nullptr;
    }

    if (PQstatus(conn) != CONNECTION_OK) {
        Logger::error(
            std::string("Database connection failed: ")
            + PQerrorMessage(conn)
        );

        PQfinish(conn);
        return nullptr;
    }

    Logger::info("Database connection established");
    return conn;
}

bool saveSteamUser(PGconn* conn, uint64_t steamid64, const std::string& personaName, bool mostRecent,
                  const std::string& macAddress) {
    if (!conn) {
        Logger::error("Cannot save steam user: database connection is null.");
        return false;
    }

    const char* paramValues[4];
    std::string steamid_str = std::to_string(steamid64);
    std::string mostRecentStr = mostRecent ? "1" : "0";

    paramValues[0] = steamid_str.c_str();
    paramValues[1] = macAddress.c_str();
    paramValues[2] = personaName.c_str();
    paramValues[3] = mostRecentStr.c_str();

    PGresult* res = PQexecParams(
        conn,
        "INSERT INTO public.users (steamid, mac_address, persona_name, most_recent) "
        "VALUES ($1, $2, $3, $4) "
        "ON CONFLICT (steamid) DO UPDATE SET mac_address = EXCLUDED.mac_address, persona_name = EXCLUDED.persona_name, most_recent = EXCLUDED.most_recent;",
        4,
        nullptr,
        paramValues,
        nullptr,
        nullptr,
        0
    );

    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
        if (res) PQclear(res);
        Logger::error(std::string("Failed to save steam user: ") + PQerrorMessage(conn));
        return false;
    }

    PQclear(res);
    Logger::info("Steam user " + std::to_string(steamid64) + " saved to database.");
    return true;
}

