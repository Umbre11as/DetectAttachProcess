cmake_minimum_required(VERSION 3.26)
project(DetectAttachDriver)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include("${CMAKE_CURRENT_LIST_DIR}/cmake/global-flags.cmake")

add_compile_options("/W0 /GL- /Ox /Ob1 /Os /EHa") # No warnings, no whole program optimization, optimization favor speed, only __inline, intrinsic functions, favor small code, SEH exceptions support
add_link_options("/ENTRY:DriverEntry")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(WDK REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")

wdk_add_driver(DetectAttachDriver ${SOURCES})
