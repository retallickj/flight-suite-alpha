#ifndef MULTITRACK_H
#define MULTITRACK_H

#define WIN_X 800
#define WIN_Y 600

#define THRESHWIN_X 200
#define THRESHWIN_Y 200

#define RECT_THICK 4
#define L 10

#define H_MAX 180

#define H_VAR 3
#define S_LOW 100
#define V_LOW 24
#define S_MAX 255
#define V_MAX 255

#define LIM_FRAMES 2000
#define WCAM_FPS 30

#define START_PAUSE 1000

#define TIME_DISPLAY_Y 20
#define TIME_DISPLAY_DIGS 1

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

typedef unsigned int uINT;

struct Rectangle{
    Point center;
    int width, height;
    Scalar color;
    Scalar color_hsv;


    int start_frame;
    bool logging;
    vector<Point3f> log;
    int lastWeight;
};

template <class TYPE>
string toStr(TYPE in)
{
    stringstream ss;
    ss << in;
    return ss.str();
}

Mat* crop(Mat* src, struct Rectangle* p_rect);



class MultiTrack{
public:
    MultiTrack(string filename, string winname, Scalar* show_color);
    ~MultiTrack();

    double start();
    bool advance();

    MultiTrack* getAddress();
    Mat* getImageAddress();

    void addRectangle(struct Rectangle* p_rect);
    void drawRectangle(int index);
    void drawRectangles();
    void clearRectangles();

    void track(int index, bool logflag);
    void trackAll();

    void setOffset(double offset);

    void exportAll(string filename);

    Scalar *show_color;

private:

    void showColor();

    vector<struct Rectangle*> p_rects;
    VideoCapture cap;
    Mat image;
    string winname;
    double fps, frames;

    double offsetTime, currentTime;
    unsigned int currentFrame;
    bool live_feed;
};
#endif // MULTITRACK_H
