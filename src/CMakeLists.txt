set(
  fbx2jsonSources 
  ${CMAKE_CURRENT_SOURCE_DIR}/fbx_exporter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/fbx_exporter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fbx_importer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/fbx_importer.h
  ${CMAKE_CURRENT_SOURCE_DIR}/fbx_parser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/fbx_parser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  PARENT_SCOPE
)

add_subdirectory(JsonBox)