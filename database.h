#pragma once

#include <cstdint>
#include <string>

#include <libpq-fe.h>

PGconn* connectToDatabase();

bool ifUserStaff(PGconn* conn, uint64_t steamid64);

bool authorizeModerator(PGconn* conn, uint64_t steamid64, const std::string& password, int& moderatorStaffId);

bool saveSteamUser(PGconn* conn, uint64_t steamid64, const std::string& personaName, bool mostRecent,
                  const std::string& macAddress);

bool createBan(PGconn* conn, uint64_t playerSteamID64, int moderatorStaffId, const std::string& reason, const std::string& evidence);

void exitWithError(PGconn* conn);

