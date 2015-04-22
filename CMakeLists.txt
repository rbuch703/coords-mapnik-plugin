CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

PROJECT (coords-mapnik-plugin)
SET(CMAKE_BUILD_TYPE Release)
#ADD_EXECUTABLE(tester coordsDataSource.cc coordsFeatureSet.cc mem_map.cc osmMappedTypes.cc osmTypes.cc pluginTest.cc)
#TARGET_LINK_LIBRARIES( tester -lmapnik -licui18n -licuuc)

SET( CXX_FLAGS "-std=c++11 -Wall -Wextra -s")
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}" )

EXECUTE_PROCESS(COMMAND mapnik-config --input-plugins 
                OUTPUT_VARIABLE MAPNIK_PLUGIN_DIR 
                OUTPUT_STRIP_TRAILING_WHITESPACE)
#SET( MAPNIK_PLUGIN_DIR "/usr/lib/mapnik/input")

ADD_LIBRARY(coords.input SHARED 
    src/coordsDataSource.cc 
    src/coordsFeatureSet.cc
    src/genericGeometry.cc
)

TARGET_LINK_LIBRARIES( coords.input -lmapnik -licui18n -licuuc)

SET_TARGET_PROPERTIES(coords.input PROPERTIES PREFIX "") #remove leading "lib" in library name
SET_TARGET_PROPERTIES(coords.input PROPERTIES SUFFIX "") #remove trailing ".so" in library name

INSTALL (TARGETS coords.input DESTINATION ${MAPNIK_PLUGIN_DIR} )

INSTALL (FILES scripts/renderMapMergedStyle.py  DESTINATION /usr/share/coords/ PERMISSIONS WORLD_EXECUTE OWNER_EXECUTE GROUP_EXECUTE WORLD_READ OWNER_READ GROUP_READ OWNER_WRITE)
INSTALL (FILES scripts/testMapnik.py DESTINATION /usr/share/coords/ PERMISSIONS WORLD_EXECUTE OWNER_EXECUTE GROUP_EXECUTE WORLD_READ OWNER_READ GROUP_READ OWNER_WRITE)


