#CMakeLists.txt

#set_property(GLOBAL APPEND PROPERTY FOO-LIST "${FOO}")

SET(MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Translator.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/mk1Controller.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/Settings.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/GCode.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/Reader.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/About.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/Geometry.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/EditGCode.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/CuttingCalc.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ManualControl.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/ScanSurface.cpp)


SET(MAIN_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/includes/mk1Controller.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/Translator.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/Settings.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/Reader.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/GCode.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/About.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/vec.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/Geometry.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/EditGCode.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/CuttingCalc.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/MainWindow.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/ManualControl.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/ScanSurface.h)


SET(MAIN_FORMS  ${CMAKE_CURRENT_SOURCE_DIR}/forms/About.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/Settings.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/EditGCode.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/CuttingCalc.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/MainWindow.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/ManualControl.ui 
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

