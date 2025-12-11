QT += core gui widgets sql

CONFIG += c++17

SOURCES += \
    main.cpp \
    databasemanager.cpp \
    logindialog.cpp \
    studentwindow.cpp \
    vendorwindow.cpp

HEADERS += \
    databasemanager.h \
    logindialog.h \
    studentwindow.h \
    vendorwindow.h

FORMS += \
    logindialog.ui \
    studentwindow.ui \
    vendorwindow.ui

# Release configuration
CONFIG += release

# Output directories
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
UI_DIR = $$PWD/build/ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
