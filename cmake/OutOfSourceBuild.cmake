# Disallow in-source build
STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" CNCQT_IN_SOURCE)
IF(CNCQT_IN_SOURCE)
    MESSAGE(FATAL_ERROR "CNC-Qt requires an out of source build. Please create a separate build directory and run 'cmake path_to_cncqt [options]' there.")
ENDIF(CNCQT_IN_SOURCE)

