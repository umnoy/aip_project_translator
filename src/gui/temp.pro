QT += core gui widgets network

TARGET = c_project
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += \
    ../core/src \
    ../requests/src \
    ../core/include 