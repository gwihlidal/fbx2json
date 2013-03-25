cmake_minimum_required (VERSION 2.6)
project (fbx2json)

FIND_LIBRARY(COCOA_LIBRARY Cocoa)
SET(EXTRA_LIBS ${COCOA_LIBRARY})

add_subdirectory(src)

include_directories ("${PROJECT_SOURCE_DIR}/include")
link_directories ("${PROJECT_SOURCE_DIR}/lib")

add_executable(fbx2json ${fbx2jsonSources})

target_link_libraries(fbx2json fbxsdk-2013.3-static JsonBox ${EXTRA_LIBS})

install (TARGETS fbx2json DESTINATION bin)