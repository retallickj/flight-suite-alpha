TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    feed.cpp \
    plotcore.cpp

CONFIG 	 += link_pkgconfig

PKGCONFIG += opencv
PKGCONFIG += fftw3

HEADERS += \
    feed.h \
    plotcore.h
