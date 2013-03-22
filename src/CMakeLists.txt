set(
  fbx2jsonSources 
  ${CMAKE_CURRENT_SOURCE_DIR}/FBXExporter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/FBXExporter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/FBXImporter.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/FBXImporter.h
  ${CMAKE_CURRENT_SOURCE_DIR}/FBXParser.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/FBXParser.h
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  PARENT_SCOPE
)

add_subdirectory(JsonBox)