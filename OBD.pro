#-------------------------------------------------
#
# Project created by QtCreator 2015-01-19T11:40:55
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OBD
TEMPLATE = app


SOURCES += main.cpp\
        obdsur.cpp \
    portcom.cpp

HEADERS  += obdsur.h \
    portcom.h

FORMS    += obdsur.ui

 include(qextserialport/qextserialport.pri)
