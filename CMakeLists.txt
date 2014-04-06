
project(karia2)
cmake_minimum_required(VERSION 2.8.9)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_LINK clang++)
set(CMAKE_C_FLAGS "-g -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
add_definitions(-g -O2)
add_definitions(-DHAVE_CONFIG_H)
add_definitions(-DKARIA2_VERSION=\"0.1.62\")
add_definitions(-DNXDATADIR=\"\\\"DATADIR\\\"\")
add_definitions(-DNXPKGDATADIR=\"\\\"PKGDATADIR\\\"\")
add_definitions(-DTHREADSAFE=1)   # for sqlite thread-safe feature

###
add_definitions(-DLWS_LIBRARY_VERSION=\"\\\"1.2\\\"\" -DLWS_BUILD_HASH=\"\\\"1.2hash\\\"\"
  -DLWS_OPENSSL_CLIENT_CERTS=\"\\\"/etc/pki/tls/certs/\\\"\")


set(CMAKE_INCLUDE_CURRENT_DIR on)
set(CMAKE_AUTOMOC on)

find_package(Qt5Core)
find_package(Qt5Network)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Xml)
find_package(Qt5Sql)

include_directories("qtsingleapplication" "tmp")
include_directories("aria2-1.18.5" "aria2-1.18.5/src" "aria2-1.18.5/lib" "aria2-1.18.5/src/includes")
include_directories("libmaia" "qjsonrpc/src")
include_directories("libwebsockets/lib")
link_directories(aria2-1.18.5/src/.libs aria2-1.18.5/deps/wslay/lib/.libs/)

add_subdirectory(src)
add_subdirectory(qtsingleapplication)


