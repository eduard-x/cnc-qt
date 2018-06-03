#CMakeLists.txt

#set_property(GLOBAL APPEND PROPERTY FOO-LIST "${FOO}")
# SET(TRANSLATIONS ${CMAKE_CURRENT_SOURCE_DIR}/translations/cnc_qt_en.ts
#                  ${CMAKE_CURRENT_SOURCE_DIR}/translations/cnc_qt_de.ts
#                  ${CMAKE_CURRENT_SOURCE_DIR}/translations/cnc_qt_ru.ts)

SET(PARSER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/GCodeScanner.cpp
                   ${CMAKE_CURRENT_SOURCE_DIR}/GerberScanner.cpp
                   ${CMAKE_CURRENT_SOURCE_DIR}/SVGScanner.cpp
                   ${CMAKE_CURRENT_SOURCE_DIR}/DXFScanner.cpp
                   ${CMAKE_CURRENT_SOURCE_DIR}/DataManager.cpp
                   PARENT_SCOPE)

# SET(PARSER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/DataManager.cpp
#                    PARENT_SCOPE)

SET(PARSER_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/includes/GCodeScanner.h
                   ${CMAKE_CURRENT_SOURCE_DIR}/includes/GerberScanner.h
                   ${CMAKE_CURRENT_SOURCE_DIR}/includes/SVGScanner.h
                   ${CMAKE_CURRENT_SOURCE_DIR}/includes/DXFScanner.h
                   ${CMAKE_CURRENT_SOURCE_DIR}/includes/DataManager.h
                   PARENT_SCOPE)

# cleanup $build/*.cpp~ on "make clean"
# SET_DIRECTORY_PROPERTIES(PROPERTIES
#     ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*~")

