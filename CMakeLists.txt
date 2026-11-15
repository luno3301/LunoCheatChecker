cmake_minimum_required(VERSION 3.16)
project(LunoCheatChecker VERSION 0.1.0 LANGUAGES CXX)

option(LUNO_BUILD_SCANNER_APP "Build the Windows console scanner" ${WIN32})

add_library(luno_scanner src/scanner/SteamScanner.cpp)
target_include_directories(luno_scanner PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(luno_scanner PUBLIC cxx_std_17)
set_target_properties(luno_scanner PROPERTIES CXX_EXTENSIONS OFF)
if(WIN32)
    target_link_libraries(luno_scanner PRIVATE advapi32)
endif()

if(LUNO_BUILD_SCANNER_APP)
    if(NOT WIN32)
        message(FATAL_ERROR "The console application requires Windows. Use -DLUNO_BUILD_SCANNER_APP=OFF to build only the scanner library.")
    endif()
    find_package(PostgreSQL REQUIRED)

    add_library(luno_core src/core/Logger.cpp)
    target_include_directories(luno_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(luno_core PUBLIC cxx_std_17)

    add_library(luno_database src/database/Database.cpp)
    target_include_directories(luno_database PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(luno_database PUBLIC cxx_std_17)
    target_link_libraries(luno_database PUBLIC PostgreSQL::PostgreSQL PRIVATE luno_core)

    add_library(luno_system src/core/System.cpp)
    target_include_directories(luno_system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_features(luno_system PUBLIC cxx_std_17)
    target_link_libraries(luno_system PRIVATE iphlpapi ws2_32)

    add_executable(LunoScanner src/main.cpp)
    target_link_libraries(LunoScanner PRIVATE luno_scanner luno_database luno_system luno_core)
    if(MINGW)
        target_link_options(LunoScanner PRIVATE -municode)
    endif()
endif()
