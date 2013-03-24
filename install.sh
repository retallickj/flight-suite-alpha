#!/bin/bash

sudo su

## INSTALL OPENCV

echo "removing previous version of ffmpeg if found..."
sudo apt-get remove ffmpeg

echo "installing image libraries..."
sudo apt-get -y install build-essential cmake pkg-config
sudo apt-get -y install libjpeg-dev
sudo apt-get -y install libtiff4-dev libjasper-dev

echo "installing GTK dev library..."
sudo apt-get -y install libgtk2.0-dev

echo "installing video libraries..."
sudo apt-get -y install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev

echo "installing video streaming libraries..."
sudo apt-get -y install libdc1394-22-dev
sudo apt-get -y install libxine-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev

echo "installing Python dev-env and Numpy..."
sudo apt-get -y install python-dev python-numpy

echo "installing intel tbb parallel code processing library..."
sudo apt-get -y install libtbb-dev

echo "installing QT4 dev library..."
sudo apt-get -y install libqt4-dev

echo "creating and moving to OpenCV source install directory..."
mkdir -p ~/source/opencv
cd ~/source/opencv

echo "downloading OpenCV 2.4.3 source..."
wget http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.3/OpenCV-2.4.3.tar.bz2

echo " --- unpacking source ..."
tar -xvf OpenCV-2.4.3.tar.bz2

echo " --- building source ..."
cd OpenCV-2.4.3
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local \
-D WITH_TBB=ON -D BUILD_NEW_PYTHON_SUPPORT=ON -D WITH_V4L=ON \
-D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON \
-D BUILD_EXAMPLES=ON -D WITH_QT=ON -D WITH_OPENGL=ON ..

echo " --- compiling source"
make
sudo make install

echo "OpenCV-2.4.3 successfully installed"


## INSTALL QT-SDK


