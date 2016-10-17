#-------------------------------------------------
#
# Project created by QtCreator 2013-05-10T22:49:41
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cncqt
TEMPLATE = app
DESTDIR = build

OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR = $$DESTDIR/.moc
RCC_DIR = $$DESTDIR/.qrc
UI_DIR = $$DESTDIR/.ui


# TODO: please check this
win32:INCLUDEPATH += $$PWD/windows 
win32:INCLUDEPATH += C:/libusb-1.0.19/libusb
win32:INCLUDEPATH += C:/Program Files (x86)/libusb-1.0.19
win32:DEPENDPATH += C:/Program Files (x86)/libusb-1.0.19/libusb
win32:DEPENDPATH += C:/libusb-1.0.19/libusb
win32:DEPENDPATH += $$PWD/windows

win64:INCLUDEPATH += $$PWD/windows 
win64:INCLUDEPATH += C:/libusb-1.0.19/libusb
win64:INCLUDEPATH += C:/libusb-1.0.19/libusb
win64:INCLUDEPATH +=C:/Program Files/libusb-1.0.19
win64:DEPENDPATH += C:/Program Files/libusb-1.0.19/libusb
win64:DEPENDPATH += C:/libusb-1.0.19/libusb

# contains(QT_ARCH, i386) {
#     win32:INCLUDEPATH += C:/libusb-1.0.19/libusb
#     win32:INCLUDEPATH += C:/Program Files (x86)/libusb-1.0.19
#     win32:DEPENDPATH += C:/Program Files (x86)/libusb-1.0.19/libusb
#     win32:DEPENDPATH += C:/libusb-1.0.19/libusb
#     win32:message("32-bit")
# } else {
#     win32:INCLUDEPATH += C:/libusb-1.0.19/libusb
#     win32:INCLUDEPATH +=C:/Program Files/libusb-1.0.19
#     win32:DEPENDPATH += C:/Program Files/libusb-1.0.19/libusb
#     win32:DEPENDPATH += C:/libusb-1.0.19/libusb
#     win32:message("64-bit")
# }


# TODO: please add this for macx
# macx

unix:INCLUDEPATH +=/usr/local/include/
unix:INCLUDEPATH +=/usr/local/include/libusb-1.0/
unix:INCLUDEPATH +=/usr/include/libusb-1.0/


SOURCES  += sources/About.cpp \
	    sources/CuttingCalc.cpp \
	    sources/GCode.cpp \
	    sources/GLWidget.cpp \
	    sources/MainWindow.cpp \
	    sources/mk1Controller.cpp \
	    sources/ScanSurface.cpp \
	    sources/Translator.cpp \
	    sources/EditGCode.cpp \
	    sources/Geometry.cpp \
	    sources/main.cpp \
	    sources/ManualControl.cpp \
	    sources/Reader.cpp \
	    sources/Settings.cpp

HEADERS  += sources/includes/About.h \
	    sources/includes/CuttingCalc.h \
	    sources/includes/GCode.h \
	    sources/includes/GLWidget.h \
	    sources/includes/ManualControl.h \
	    sources/includes/Reader.h \
	    sources/includes/Settings.h \
	    sources/includes/version.h \
	    sources/includes/EditGCode.h \
	    sources/includes/Geometry.h \
	    sources/includes/MainWindow.h \
	    sources/includes/mk1Controller.h \
	    sources/includes/ScanSurface.h \
	    sources/includes/Translator.h

FORMS    += sources/forms/About.ui \
	    sources/forms/CodeGenerator.ui \
	    sources/forms/CuttingCalc.ui \
	    sources/forms/EditGCode.ui \
	    sources/forms/MainWindow.ui \
	    sources/forms/ManualControl.ui \
	    sources/forms/ScanSurface.ui \
	    sources/forms/Settings.ui

	    
# TODO: please check this
win32:LIBS += -L$$PWD/windows/ -lusb-1.0
# contains(QT_ARCH, i386) {
#     win32:LIBS += -LC:/Program Files (x86)/\libusb-1.0.19\Win32\Debug\lib\libusb-1.0
#     win32:LIBS += -LC:\libusb-1.0.19\Win32\Debug\lib\libusb-1.0
#     win32:message("32-bit")
# } else {
#     win32:LIBS += -LC:/Program Files/\libusb-1.0.19\Win32\Debug\lib\libusb-1.0
#     win32:LIBS += -LC:\libusb-1.0.19\Win32\Debug\lib\libusb-1.0
#     win32:message("64-bit")
# }


# TODO: please add this for macx
# macx:LIBS +=


unix:!macx: LIBS += -L/usr/local/lib -lusb-1.0



RESOURCES += sources/CNC-Qt.qrc


QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_RELEASE += -O3


