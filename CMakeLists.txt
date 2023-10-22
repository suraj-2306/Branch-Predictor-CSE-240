cmake_minimum_required(VERSION 3.5)

# set the project name
project(CS240)

# Create a sources variable with a link to all c files to compile
set(SOURCES src/main.c src/predictor.c src/predictor.h)

# add the executable
add_executable(CS240 ${SOURCES})
# add_subdirectory(src)
