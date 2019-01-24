#CMakeLists.txt

#set_property(GLOBAL APPEND PROPERTY FOO-LIST "${FOO}")
# SET(TRANSLATIONS ${CMAKE_CURRENT_SOURCE_DIR}/translations/cnc_qt_en.ts
#                  ${CMAKE_CURRENT_SOURCE_DIR}/translations/cnc_qt_de.ts
#                  ${CMAKE_CURRENT_SOURCE_DIR}/translations/cnc_qt_ru.ts)

SET(MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Translator.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/mk1Controller.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/GData.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/About.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/Geometry.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/EditGCode.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/GCodeTableModel.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ScanSurface.cpp)


SET(MOC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/includes/mk1Controller.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/About.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/EditGCode.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/GCodeTableModel.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/MainWindow.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/ScanSurface.h)


SET(MAIN_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/includes/Geometry.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/Translator.h
                 PARENT_SCOPE)

# 
SET(MAIN_FORMS  ${CMAKE_CURRENT_SOURCE_DIR}/forms/About.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/EditGCode.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/MainWindow.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/ScanSurface.ui
                PARENT_SCOPE)


IF(${QT_USE_QTOPENGL})
  SET(MAIN_SOURCES ${MAIN_SOURCES}
                   ${GL_SOURCES}
                   ${CMAKE_CURRENT_SOURCE_DIR}/GLWidget.cpp
                   PARENT_SCOPE)

  SET(MOC_HEADERS  ${MOC_HEADERS}
                   ${GL_HEADERS}
                   ${CMAKE_CURRENT_SOURCE_DIR}/includes/GLWidget.h
                   PARENT_SCOPE)
ELSE()
  SET(MAIN_SOURCES ${MAIN_SOURCES}
                 PARENT_SCOPE)
                 
  SET(MOC_HEADERS ${MOC_HEADERS}
                 PARENT_SCOPE)
ENDIF()

                 
# SET(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${MAIN_SOURCES} ${MAIN_FORMS} ${MAIN_HEADERS} 
#                  PARENT_SCOPE)

SET(APP_RESOURCES  
                ${CMAKE_CURRENT_SOURCE_DIR}/CNC-Qt.qrc
                ${CMAKE_CURRENT_SOURCE_DIR}/shaders.qrc
                PARENT_SCOPE)




# cleanup $build/*.cpp~ on "make clean"
# SET_DIRECTORY_PROPERTIES(PROPERTIES
#     ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*~")

