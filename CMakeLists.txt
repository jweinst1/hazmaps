# Project HazMaps main cmake file

cmake_minimum_required(VERSION 3.6)

project(hazmaps VERSION 0.0.1)

# Option to install targets to env location
if(DEFINED ENV{HAZMAPS})
     message("-- Will install HAZMAPS binaries and libraries to location: " $ENV{HAZMAPS})
	 set(CMAKE_INSTALL_PREFIX $ENV{HAZMAPS})
else(DEFINED ENV{HAZMAPS})
     message("-- Will install HAZMAPS binaries and libraries to location: " ${CMAKE_CURRENT_BINARY_DIR})
	 set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR})
endif(DEFINED ENV{HAZMAPS})


# Disable some windows specific warnings for using more modern c functions
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC)

# Options
option(WITH_testing "Allows tests to be run" ON)

if(WITH_testing MATCHES ON)
   enable_testing()
endif(WITH_testing MATCHES ON)

add_subdirectory(tests)
