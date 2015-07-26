SET(CNCQT_PACKAGE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
FILE(MAKE_DIRECTORY "${CNCQT_PACKAGE_DIRECTORY}")

# Generate source packages
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/package-source-tbz2.cpack.in ${CMAKE_CURRENT_BINARY_DIR}/package-source-tbz2.cpack)
ADD_CUSTOM_TARGET(package-source-tbz2
	WORKING_DIRECTORY "${CNCQT_PACKAGE_DIRECTORY}"
	COMMAND ${CMAKE_CPACK_COMMAND} --config "${CMAKE_CURRENT_BINARY_DIR}/package-source-tbz2.cpack"
	)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/package-source-tgz.cpack.in ${CMAKE_CURRENT_BINARY_DIR}/package-source-tgz.cpack)
ADD_CUSTOM_TARGET(package-source-tgz
	WORKING_DIRECTORY "${CNCQT_PACKAGE_DIRECTORY}"
	COMMAND ${CMAKE_CPACK_COMMAND} --config "${CMAKE_CURRENT_BINARY_DIR}/package-source-tgz.cpack"
	)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/package-source-zip.cpack.in ${CMAKE_CURRENT_BINARY_DIR}/package-source-zip.cpack)
ADD_CUSTOM_TARGET(package-source-zip
	WORKING_DIRECTORY "${CNCQT_PACKAGE_DIRECTORY}"
	COMMAND ${CMAKE_CPACK_COMMAND} --config "${CMAKE_CURRENT_BINARY_DIR}/package-source-zip.cpack"
	)

# Upload source packages to SourceForge
# ADD_CUSTOM_TARGET(upload-package-source-tbz2
# 	DEPENDS "${CNCQT_PACKAGE_DIRECTORY}/${CNCQT_CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2"
# 	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
# 	COMMAND rsync -e ssh -R --progress --protect-args "CNC-Qt ${CNCQT_VERSION}/${CNCQT_CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.bz2" "${HTTRAQT_SOURCEFORGE_USERNAME},k3d@frs.sourceforge.net:/home/frs/project/k/k3/k3d/HTTraQt Source/"
# 	)
# 
# ADD_CUSTOM_TARGET(upload-package-source-tgz
# 	DEPENDS "${CNCQT_PACKAGE_DIRECTORY}/${CNCQT_CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.gz"
# 	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
# 	COMMAND rsync -e ssh -R --progress --protect-args "CNC-Qt ${CNCQT_VERSION}/${CNCQT_CPACK_SOURCE_PACKAGE_FILE_NAME}.tar.gz" "${HTTRAQT_SOURCEFORGE_USERNAME},k3d@frs.sourceforge.net:/home/frs/project/k/k3/k3d/HTTraQt Source/"
# 	)
# 
# ADD_CUSTOM_TARGET(upload-package-source-zip
# 	DEPENDS "${CNCQT_PACKAGE_DIRECTORY}/${CNCQT_CPACK_SOURCE_PACKAGE_FILE_NAME}.zip"
# 	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
# 	COMMAND rsync -e ssh -R --progress --protect-args "CNC-Qt ${CNCQT_VERSION}/${CNCQT_CPACK_SOURCE_PACKAGE_FILE_NAME}.zip" "${HTTRAQT_SOURCEFORGE_USERNAME},k3d@frs.sourceforge.net:/home/frs/project/k/k3/k3d/HTTraQt Source/"
# 	)

