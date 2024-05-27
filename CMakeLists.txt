## #################################################################
## Root CMakeLists.txt
## #################################################################

cmake_minimum_required(VERSION 3.5)

project(testminixml)

if(CMAKE_SYSTEM_NAME MATCHES Windows)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Build the XML Parser tests

file( GLOB XML_COMMON src/*.c )
file( GLOB XML_COMMONH src/*.h )

set( xml_includes "")
set( xml_sources "")
list(APPEND xml_sources ${XML_COMMON} )
list(APPEND xml_sources ${XML_COMMONH} )
list(APPEND xml_includes "src") 

set( libs "")
if(CMAKE_SYSTEM_NAME MATCHES Linux)
        list(APPEND libs m)
endif()

add_executable( "simpletest" ${xml_sources} ${xml_headers} "TestCode/simpletest.c")
target_include_directories("simpletest" SYSTEM PRIVATE ${xml_includes})
target_link_libraries( "simpletest" ${libs} )


