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

add_executable( "upperlower" ${xml_sources} ${xml_headers} "TestCode/upperlower.c")
target_include_directories("upperlower" SYSTEM PRIVATE ${xml_includes})
target_link_libraries( "upperlower" ${libs} )

add_executable( "xmltojson" ${xml_sources} ${xml_headers} "TestCode/xmltojson.c"
"TestCode/options.c" "TestCode/options.h")
target_include_directories("xmltojson" SYSTEM PRIVATE ${xml_includes})
target_link_libraries( "xmltojson" ${libs} )

add_executable( "xmltocsv" ${xml_sources} ${xml_headers} "TestCode/xmltocsv.c" 
"TestCode/options.c" "TestCode/options.h")
target_include_directories("xmltocsv" SYSTEM PRIVATE ${xml_includes})
target_link_libraries( "xmltocsv" ${libs} )

add_executable( "striptags" ${xml_sources} ${xml_headers} "TestCode/striptags.c")
target_include_directories("striptags" SYSTEM PRIVATE ${xml_includes})
target_link_libraries( "striptags" ${libs} )

add_executable( "directory" ${xml_sources} ${xml_headers} "TestCode/directory.c")
target_include_directories("directory" SYSTEM PRIVATE ${xml_includes})
target_link_libraries( "directory" ${libs} )

add_executable( "listdirectory" ${xml_sources} ${xml_headers} "TestCode/listdirectory.c")
target_include_directories("listdirectory" SYSTEM PRIVATE ${xml_includes})
target_link_libraries( "listdirectory" ${libs} )

add_executable( "directorytoxml" ${xml_sources} ${xml_headers} "TestCode/directorytoxml.c")
target_include_directories("directorytoxml" SYSTEM PRIVATE ${xml_includes})
target_link_libraries( "directorytoxml" ${libs} )


