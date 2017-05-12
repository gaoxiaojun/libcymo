cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME LIBCRMO)
set(PROJECT_BRIEF "c trading framework")

project(${PROJECT_NAME} C)

set(CMAKE_C_STANDARD 11)

# be nice to vim users
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# silence policy CMP0042 warning by enabling RPATH explicitly
if(APPLE AND NOT DEFINED CMAKE_MACOSX_RPATH)
    set(CMAKE_MACOSX_RPATH true)
endif()

################################################################################
#                              set project options                             #
################################################################################
option(LIBCYMO_ENABLE_TESTING "Enable unit test" ON)

################################################################################
#                              get version                                     #
################################################################################
# read content of config.hpp
file(READ "include/cymo_version.h" VERSION_H)
# get line containing the version
string(REGEX MATCH "#define CYMO_VERSION_MAJOR [0-9]+" VERSION_LINE "${VERSION_H}")
# extract version number from line
string(REGEX MATCH "[0-9]+" CYMO_VERSION_MAJOR "${VERSION_LINE}")
# get line containing the version
string(REGEX MATCH "#define CYMO_VERSION_MINOR [0-9]+" VERSION_LINE "${VERSION_H}")
# extract version number from line
string(REGEX MATCH "[0-9]+" CYMO_VERSION_MINOR "${VERSION_LINE}")
# get line containing the version
string(REGEX MATCH "#define CYMO_VERSION_PATCH [0-9]+" VERSION_LINE "${VERSION_H}")
# extract version number from line
string(REGEX MATCH "[0-9]+" CYMO_VERSION_PATCH "${VERSION_LINE}")

# create full version string
set(CYMO_VERSION
    "${CYMO_VERSION_MAJOR}.${CYMO_VERSION_MINOR}.${CYMO_VERSION_PATCH}")

################################################################################
#   set output paths for binaries and libraries if not provided by the user    #
################################################################################
# prohibit in-source builds
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
    message(FATAL_ERROR "In-source builds are not allowed. Please use "
            "./configure to choose a build directory and "
            "initialize the build configuration.")
endif()
# set module path appropriately
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
# set binary output path if not defined by user
if("${EXECUTABLE_OUTPUT_PATH}" STREQUAL "")
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/bin")
endif()
# set library output path if not defined by user, but always set
# library output path to binary output path for Xcode projects
if("${CMAKE_GENERATOR}" STREQUAL "Xcode")
    set(LIBRARY_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}")
elseif("${LIBRARY_OUTPUT_PATH}" STREQUAL "")
    set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/lib")
endif()

################################################################################
#                                compiler setup                                #
################################################################################

# set build default build type to RelWithDebInfo if not set
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# add extra flags
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${EXTRA_FLAGS})
################################################################################
#                                Doxygen setup                                 #
################################################################################
# check for doxygen and add custom "doc" target to Makefile
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in"
            "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile"
            @ONLY)
    add_custom_target(doc "${DOXYGEN_EXECUTABLE}"
            "${CMAKE_HOME_DIRECTORY}/Doxyfile"
            WORKING_DIRECTORY "${CMAKE_HOME_DIRECTORY}"
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
endif(DOXYGEN_FOUND)
################################################################################
#                                Boost setup                                   #
################################################################################
find_package(libuv REQUIRED)
if(LIBUV_FOUND)
    include_directories(${LIBUV_INCLUDE_DIRS})
endif()
################################################################################
#                       set inclue paths for subprojects                       #
################################################################################
# all projects need the headers of the core components
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

# Build the targets
add_subdirectory(src)

# enable tests if not disabled
if (LIBCYMO_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
################################################################################
#                                print summary                                 #
################################################################################
# collect all compiler flags
string(TOUPPER "${CMAKE_BUILD_TYPE}" UPPER_BUILD_TYPE)
set(ALL_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${UPPER_BUILD_TYPE}}")

# done
if(NOT LIBCYMO_NO_SUMMARY)
    message(STATUS
            "\n====================|  Build Summary  |===================="
            "\n"
            "\nLibLIBCYMO version:     ${CYMO_VERSION}"
            "\n"
            "\nBuild type:        ${CMAKE_BUILD_TYPE}"
            "\nBuild static:      ${LIBCYMO_BUILD_STATIC}"
            "\nBuild static only: ${LIBCYMO_BUILD_STATIC_ONLY}"
            "\nC:                 ${CMAKE_C_COMPILER}"
            "\nCFLAGS:            ${ALL_C_FLAGS}"
            "\nLIBRARIES:         ${LD_FLAGS}"
            "\n"
            "\nSource directory:  ${CMAKE_CURRENT_SOURCE_DIR}"
            "\nBuild directory:   ${CMAKE_CURRENT_BINARY_DIR}"
            "\nExecutable path:   ${EXECUTABLE_OUTPUT_PATH}"
            "\nLibrary path:      ${LIBRARY_OUTPUT_PATH}"
            "\nInstall prefix:    ${CMAKE_INSTALL_PREFIX}"
            "\nGenerator:         ${CMAKE_GENERATOR}"
            "\n"
            "\n===========================================================\n")
endif()
################################################################################
#                                THE END                                       #
################################################################################
