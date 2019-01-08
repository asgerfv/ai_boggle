cmake_minimum_required(VERSION 3.5)

project(Game_Boggle)

add_definitions(-std=c++14)


# Source files
file(GLOB_RECURSE SRC_FILES "src/*.cpp")

include_directories(".", "src")

# Executable dependencies
add_executable(game_boggle ${SRC_FILES}) 

install (TARGETS game_boggle DESTINATION bin)
