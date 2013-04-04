#-------------------------------------------------
#
# Project created by QtCreator 2012-03-28T13:16:34
#
#-------------------------------------------------

QT       += core gui sql

TARGET = AsistenciaBus
TEMPLATE = app


SOURCES += main.cpp\
        bus.cpp \
    dblocal.cpp \
    dbsync.cpp \
    csv_lector.cpp

HEADERS  += bus.h \
    dblocal.h \
    dbsync.h \
    csv_lector.h

FORMS    += bus.ui

CONFIG += static

static { # everything below takes effect with CONFIG += static

    CONFIG += static
    CONFIG += staticlib # this is needed if you create a static library, not a static executable
    DEFINES += STATIC
    message("~~~ static build ~~~") # this is for information, that the static build is done
    win32: TARGET = $$join(TARGET,,,s) #this adds an s in the end, so you can seperate static build from non static build
}
