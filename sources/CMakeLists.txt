#CMakeLists.txt

#set_property(GLOBAL APPEND PROPERTY FOO-LIST "${FOO}")

SET(MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Translator.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/mk1Controller.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sControl.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sIO.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sMaterial.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sParser.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sSpeed.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sSystem.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sVis.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sWorkbench.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/Settings.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/SettingsDialog.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/GCode.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/Reader.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/About.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/Geometry.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/EditGCode.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ScanSurface.cpp)


SET(MAIN_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/includes/mk1Controller.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/Translator.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sControl.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sIO.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sMaterial.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sParser.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sSpeed.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sSystem.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sVis.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sWorkbench.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/Settings.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/SettingsDialog.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/Reader.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/GCode.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/About.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/Geometry.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/EditGCode.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/MainWindow.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/ScanSurface.h)


SET(MAIN_FORMS  ${CMAKE_CURRENT_SOURCE_DIR}/forms/About.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sControl.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sIO.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sMaterial.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sParser.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sSpeed.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sSystem.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sVis.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sWorkbench.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/Settings.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/EditGCode.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/MainWindow.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/ScanSurface.ui
                PARENT_SCOPE)

                
IF(${QT_USE_QTOPENGL})
  SET(MAIN_SOURCES ${MAIN_SOURCES}
                 ${CMAKE_CURRENT_SOURCE_DIR}/GLWidget.cpp
                 PARENT_SCOPE)
                 
  SET(MAIN_HEADERS ${MAIN_HEADERS}
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/GLWidget.h
                 PARENT_SCOPE)
ELSE()
  SET(MAIN_SOURCES ${MAIN_SOURCES}
                 PARENT_SCOPE)
                 
  SET(MAIN_HEADERS ${MAIN_HEADERS}
                 PARENT_SCOPE)
ENDIF()


SET(APP_RESOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/CNC-Qt.qrc
		PARENT_SCOPE)


# cleanup $build/*.cpp~ on "make clean"
SET_DIRECTORY_PROPERTIES(PROPERTIES
    ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*~")

