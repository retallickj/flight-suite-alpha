TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    multicameratriangulator.cpp

HEADERS += \
    multicameratriangulator.h


CONFIG 	 += link_pkgconfig

PKGCONFIG += opencv
