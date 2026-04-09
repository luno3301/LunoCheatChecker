#pragma once

#include <cstdint>
#include <string>

#include <libpq-fe.h>

PGconn* connectToDatabase();

bool ifUserStaff(PGconn* conn, uint64_t steamid64);

bool saveSteamUser(PGconn* conn, uint64_t steamid64, const std::string& personaName, bool mostRecent,
                  const std::string& macAddress);

void exitWithError(PGconn* conn);

