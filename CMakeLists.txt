## #################################################################
## Root CMakeLists.txt
## #################################################################

cmake_minimum_required(VERSION 3.0)

project(xmltocsv)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Build the XML to CSV converter

file( GLOB BBX_COMMON src/*.c )
file( GLOB BBX_COMMONH src/*.h )

set( bbx_includes "")
set( bbx_sources "")
list(APPEND bbx_sources ${BBX_COMMON} )
list(APPEND bbx_sources ${BBX_COMMONH} )
list(APPEND bbx_includes "src") 

set( libs "")
if(CMAKE_SYSTEM_NAME MATCHES Linux)
        list(APPEND libs m)
endif()

add_executable( "xmltocsv" ${bbx_sources} ${bbx_headers} )
target_link_libraries( "xmltocsv" ${libs} )


