TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    multitrack.cpp

HEADERS += \
    multitrack.h

CONFIG 	 += link_pkgconfig

PKGCONFIG += opencv

