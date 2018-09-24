#CMakeLists.txt


SET(GL_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/gcodedrawer.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/heightmapdrawer.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/origindrawer.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/shaderdrawable.cpp
               ${CMAKE_CURRENT_SOURCE_DIR}/tooldrawer.cpp
               PARENT_SCOPE)


SET(GL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/includes/gcodedrawer.h
               ${CMAKE_CURRENT_SOURCE_DIR}/includes/heightmapdrawer.h
               ${CMAKE_CURRENT_SOURCE_DIR}/includes/origindrawer.h
               ${CMAKE_CURRENT_SOURCE_DIR}/includes/shaderdrawable.h
               ${CMAKE_CURRENT_SOURCE_DIR}/includes/tooldrawer.h
               PARENT_SCOPE)

# cleanup $build/*.cpp~ on "make clean"
# SET_DIRECTORY_PROPERTIES(PROPERTIES
#     ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*~")

