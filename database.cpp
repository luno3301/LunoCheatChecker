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
        Logger::error(std::string("Error Table Accessing") + tableName + ": " + PQerrorMessage(conn));
        return false;
    }

    bool hasRole = (std::stoi(PQgetvalue(res, 0, 0)) > 0);

    PQclear(res);
    return hasRole;
}

bool createBan(PGconn* conn, uint64_t playerSteamID64, int moderatorStaffId, const std::string& reason, const std::string& evidence) {
    if (!conn) {
        Logger::error("Cannot create ban: database connection is null.");
        return false;
    }

    if (reason.empty()) {
        Logger::warning("Cannot create ban: reason is empty.");
        return false;
    }

    const char* paramValues[4];
    std::string playerSteamID64Str = std::to_string(playerSteamID64);
    std::string moderatorStaffIdStr = std::to_string(moderatorStaffId);

    paramValues[0] = playerSteamID64Str.c_str();
    paramValues[1] = moderatorStaffIdStr.c_str();
    paramValues[2] = reason.c_str();
    paramValues[3] = evidence.c_str();

    PGresult* res = PQexecParams(
        conn,
        "WITH created_ban AS ("
        "    INSERT INTO public.bans (user_id, server_id, banned_by_staff_id, reason, evidence) "
        "    SELECT u.id, s.server_id, s.id, $3, NULLIF($4, '') "
        "    FROM public.users u "
        "    JOIN public.staff s ON s.id = $2::integer "
        "    WHERE u.steamid = $1::bigint "
        "    AND s.end_at IS NULL "
        "    RETURNING id"
        "), updated_staff AS ("
        "    UPDATE public.staff s "
        "    SET ban_count = ("
        "        SELECT COUNT(*)::integer "
        "        FROM public.bans b "
        "        WHERE b.banned_by_staff_id = s.id"
        "    ) + ("
        "        SELECT COUNT(*)::integer "
        "        FROM created_ban cb "
        "    ) "
        "    WHERE s.id = $2::integer "
        "    AND EXISTS (SELECT 1 FROM created_ban) "
        "    RETURNING s.ban_count"
        ") "
        "SELECT cb.id, us.ban_count "
        "FROM created_ban cb "
        "LEFT JOIN updated_staff us ON true;",
        4,
        nullptr,
        paramValues,
        nullptr,
        nullptr,
        0
    );

    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (res) PQclear(res);
        Logger::error(std::string("Failed to create ban: ") + PQerrorMessage(conn));
        return false;
    }

    if (PQntuples(res) == 0) {
        PQclear(res);
        Logger::warning("Ban was not created: player or active moderator staff record was not found.");
        return false;
    }

    const std::string banId = PQgetvalue(res, 0, 0);
    const std::string banCount = PQgetvalue(res, 0, 1);
    PQclear(res);

    Logger::info("Ban " + banId + " created for SteamID64: " + playerSteamID64Str +
                 ". Moderator ban count: " + banCount);
    return true;
}

bool ifUserStaff(PGconn* conn, uint64_t steamid64) {
    return isUserInRoleTable(conn, steamid64, "public.staff");
}

bool authorizeModerator(PGconn* conn, uint64_t steamid64, const std::string& password, int& moderatorStaffId) {
    moderatorStaffId = 0;

    if (!conn) {
        Logger::error("Cannot authorize moderator: database connection is null.");
        return false;
    }

    if (password.empty()) {
        Logger::warning("Moderator authorization failed: empty password.");
        return false;
    }

    const char* paramValues[2];
    std::string steamid_str = std::to_string(steamid64);
    paramValues[0] = steamid_str.c_str();
    paramValues[1] = password.c_str();

    PGresult* res = PQexecParams(
        conn,
        "SELECT id FROM public.staff "
        "WHERE steamid = $1 "
        "AND end_at IS NULL "
        "AND password_hash IS NOT NULL "
        "AND password_hash = public.crypt($2, password_hash) "
        "LIMIT 1;",
        2,
        nullptr,
        paramValues,
        nullptr,
        nullptr,
        0
    );

    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK) {
        if (res) PQclear(res);
        Logger::error(std::string("Failed to authorize moderator: ") + PQerrorMessage(conn));
        return false;
    }

    const bool authorized = (PQntuples(res) > 0);
    if (authorized) {
        moderatorStaffId = std::stoi(PQgetvalue(res, 0, 0));
    }

    PQclear(res);
    return authorized;
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

