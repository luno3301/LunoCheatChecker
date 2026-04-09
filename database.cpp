#include "database.h"
#include "logger.h"

#include <cstdlib>
#include <cstring>

void exitWithError(PGconn* conn) {
    Logger::error(std::string("Database error: ") + PQerrorMessage(conn));
    PQfinish(conn);
    exit(1);
}

static std::string getConnectionString() {
    const char* fromEnv = std::getenv("DB_CONNINFO");
    if (fromEnv && std::strlen(fromEnv) > 0) {
        return std::string(fromEnv);
    }

    // fallback по умолчанию, можно вынести в конфиг
    return "dbname=postgres user=postgres password=12345 host=localhost port=5432";
}

PGconn* connectToDatabase() {
    const std::string conninfo = getConnectionString();
    PGconn* conn = PQconnectdb(conninfo.c_str());

    if (PQstatus(conn) != CONNECTION_OK) {
        Logger::error(std::string("Failed to connect to database: ") + PQerrorMessage(conn));
        PQfinish(conn);
        return nullptr;
    }

    Logger::info("Connected to the database successfully.");
    return conn;
}

static bool isUserInRoleTable(PGconn* conn, uint64_t steamid64, const std::string& tableName) {
    if (!conn) {
        Logger::error("Database connection is null in isUserInRoleTable.");
        return false;
    }

    const char* paramValues[1];
    std::string steamid_str = std::to_string(steamid64);
    paramValues[0] = steamid_str.c_str();

    const std::string query = "SELECT COUNT(*) FROM " + tableName + " WHERE steamid = $1;";

    PGresult* res = PQexecParams(
        conn,
        query.c_str(),
        1,
        nullptr,
        paramValues,
        nullptr,
        nullptr,
        0
    );

    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (res) PQclear(res);
        Logger::error(std::string("Ошибка запроса к таблице ") + tableName + ": " + PQerrorMessage(conn));
        return false;
    }

    bool hasRole = (std::stoi(PQgetvalue(res, 0, 0)) > 0);

    PQclear(res);
    return hasRole;
}

bool ifUserStaff(PGconn* conn, uint64_t steamid64) {
    return isUserInRoleTable(conn, steamid64, "public.staff");
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

