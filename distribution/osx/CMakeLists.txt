#OPTION(CNCQT_INSTALL_AQSIS "Include Aqsis in the K-3D installer" OFF)
#OPTION(CNCQT_INSTALL_CGKIT "Include cgkit in the K-3D installer" OFF)
#OPTION(CNCQT_INSTALL_PYGTK "Include PyGTK in the K-3D installer" OFF)
#OPTION(CNCQT_INSTALL_PYOPENGL "Include PyOpenGL in the K-3D installer" OFF)
#OPTION(CNCQT_INSTALL_PYTHON "Include Python in the K-3D installer" ON)
OPTION(CNCQT_CREATE_PACKAGEMAKER_PACKAGE "Include an option to build a MacOSX PackageMaker package." OFF)

SET(CNCQT_BINARY_LOCATIONS )
LIST(APPEND CNCQT_BINARY_LOCATIONS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cncqt")
LIST(APPEND CNCQT_BINARY_LOCATIONS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cncqt-make-module-proxy")
LIST(APPEND CNCQT_BINARY_LOCATIONS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cncqt-renderframe")
LIST(APPEND CNCQT_BINARY_LOCATIONS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cncqt-renderjob")
LIST(APPEND CNCQT_BINARY_LOCATIONS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cncqt-resource-compiler")
LIST(APPEND CNCQT_BINARY_LOCATIONS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cncqt-sl2xml")
LIST(APPEND CNCQT_BINARY_LOCATIONS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/cncqt-uuidgen")

FILE(GLOB_RECURSE CNCQT_PLUGIN_BINARIES "${CNCQT_BINARY_DIR}/${CNCQT_LIBDIR}/cncqt/plugins/*.module")
LIST(APPEND CNCQT_BINARY_LOCATIONS ${CNCQT_PLUGIN_BINARIES})

# Enable automatic installation of library dependencies ...
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/dependencies.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/dependencies.cmake" @ONLY)
INSTALL(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/dependencies.cmake")

# Install some runtime dependencies that can't be detected via static analysis ...

# This is an ugly hack, we should be able to detect this automatically, but I'm feeling lazy ...
SET(CNCQT_MACPORTS_DIR "/opt/local" CACHE PATH "Root directory containing installed MacPorts packages")
MARK_AS_ADVANCED(CNCQT_MACPORTS_DIR)

INSTALL(
	PROGRAMS
		${CNCQT_MACPORTS_DIR}/lib/libgailutil.18.dylib
	DESTINATION
		lib
	)

INSTALL(
	DIRECTORY
		${CNCQT_MACPORTS_DIR}/lib/gtk-2.0/2.10.0
	DESTINATION
		lib/gtk-2.0
	USE_SOURCE_PERMISSIONS
	)

INSTALL(
	DIRECTORY
		${CNCQT_MACPORTS_DIR}/lib/gtk-2.0/modules
	DESTINATION
		lib/gtk-2.0/2.10.0
	USE_SOURCE_PERMISSIONS
	)

INSTALL(
	DIRECTORY
		${CNCQT_MACPORTS_DIR}/lib/pango
	DESTINATION
		lib
	USE_SOURCE_PERMISSIONS
	)

INSTALL(
	DIRECTORY
		${CNCQT_MACPORTS_DIR}/etc/gtk-2.0
		${CNCQT_MACPORTS_DIR}/etc/fonts
		${CNCQT_MACPORTS_DIR}/etc/pango
	DESTINATION
		etc
	)

# Bundle packaging is always available ...
ADD_SUBDIRECTORY(bundle)

# We optionally support creating packagemaker packages ...
IF(CNCQT_CREATE_PACKAGEMAKER_PACKAGE)
	ADD_SUBDIRECTORY(packagemaker)
ENDIF(CNCQT_CREATE_PACKAGEMAKER_PACKAGE)

