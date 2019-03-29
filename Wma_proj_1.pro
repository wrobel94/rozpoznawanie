#-------------------------------------------------
#
# Project created by QtCreator 2016-05-26T16:44:45
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = Wma_proj_1
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH += F:\opencv\RELEASE\install\include

LIBS += F:\opencv\RELEASE\bin\libopencv_core249.dll
LIBS += F:\opencv\RELEASE\bin\libopencv_contrib249.dll
LIBS += F:\opencv\RELEASE\bin\libopencv_highgui249.dll
LIBS += F:\opencv\RELEASE\bin\libopencv_imgproc249.dll
LIBS += F:\opencv\RELEASE\bin\libopencv_calib3d249.dll

SOURCES += main.cpp
