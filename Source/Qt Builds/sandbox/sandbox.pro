TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

CONFIG 	 += link_pkgconfig

PKGCONFIG += opencv
PKGCONFIG += fftw3

