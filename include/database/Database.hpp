#pragma once

#include <cstdint>
#include <string>

#include <libpq-fe.h>

PGconn* connectToDatabase();

bool saveSteamUser(PGconn* conn, uint64_t steamid64, const std::string& personaName, bool mostRecent,
                  const std::string& macAddress);

void exitWithError(PGconn* conn);

