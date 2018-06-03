#CMakeLists.txt

#set_property(GLOBAL APPEND PROPERTY FOO-LIST "${FOO}")
# SET(TRANSLATIONS ${CMAKE_CURRENT_SOURCE_DIR}/translations/cnc_qt_en.ts
#                  ${CMAKE_CURRENT_SOURCE_DIR}/translations/cnc_qt_de.ts
#                  ${CMAKE_CURRENT_SOURCE_DIR}/translations/cnc_qt_ru.ts)

SET(SETTINGS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sControl.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sIO.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sMaterial.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sParser.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sSpeed.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sSystem.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sVis.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/sWorkbench.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/Settings.cpp
                 ${CMAKE_CURRENT_SOURCE_DIR}/SettingsDialog.cpp
                 PARENT_SCOPE)


SET(MOC_SETTINGS_HEADERS  ${CMAKE_CURRENT_SOURCE_DIR}/includes/sControl.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sIO.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sMaterial.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sParser.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sSpeed.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sSystem.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sVis.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/sWorkbench.h
                 ${CMAKE_CURRENT_SOURCE_DIR}/includes/SettingsDialog.h
                 PARENT_SCOPE)


SET(SETTINGS_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/includes/Settings.h
                 PARENT_SCOPE)

# 
SET(SETTINGS_FORMS  ${CMAKE_CURRENT_SOURCE_DIR}/forms/sControl.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sIO.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sMaterial.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sParser.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sSpeed.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sSystem.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sVis.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/sWorkbench.ui
                ${CMAKE_CURRENT_SOURCE_DIR}/forms/Settings.ui
                PARENT_SCOPE)
                 
# SET(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE} ${MAIN_SOURCES} ${MAIN_FORMS} ${MAIN_HEADERS} 
#                  PARENT_SCOPE)

SET(APP_RESOURCES  ${CMAKE_CURRENT_SOURCE_DIR}/../CNC-Qt.qrc
		 PARENT_SCOPE)




# cleanup $build/*.cpp~ on "make clean"
# SET_DIRECTORY_PROPERTIES(PROPERTIES
#     ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*~")

