#ifndef FEED_H
#define FEED_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "fftw3.h"

#include <iostream>
#include <sstream>


#define LOCK_IN_T 60
#define LOCK_IN_SCALE 100000

#define L_SMOOTH_T 4

#define WIN_X 800
#define WIN_Y 600

#define SUB_X 100
#define SUB_Y 100

#define SUB_PLOT_FLAG 0

#define TRIGGER_WEIGHT_DEFAULT      50000
#define TRIGGER_WEIGHT_MAX          100000

#define FFT_MAX_DEFAULT 100
#define FFT_MAX_LIM 100

#define FFT_FREQ_MIN 5

#define SIGNAL_HUE                  60

#define TIME_DISPLAY_DIGS 2
#define TIME_DISPLAY_Y 20

#define TRIG_DISPLAY_OFFSET 40
#define SIG_DISPLAY_OFFSET 10

#define R_THICK 3

#define START_PAUSE 100

#define FFT_PHASE_BOUND .5

#define MIN_SIG_SIZE 20

typedef unsigned int uINT;


using namespace cv;
using namespace std;

template <class TYPE>
string toStr(TYPE in)
{
    stringstream ss;
    ss << in;
    return ss.str();
}

class Feed
{
public:
    Feed(string filename, string winname, int* thresh);
    ~Feed();

    // *** Methods ***

    int start();
    int advance();

    void setTrigROI(Rect &roi);
    void setSigROI(Rect &roi);

    void reset();

    void write(FileStorage fs, string ID);

    vector<Point2d>* getSigLog();
    vector<Point2d>* getTrigLog();

    vector<Point2d>* getfft_amp();
    vector<Point2d>* getfft_arg();

    void setOffset(double offset);
    double getOffset();

    Mat* getImageAddress();

    void fft(vector<Point2d> *amp, vector<Point2d> *angle);

    void setTBounds(Point2d* time_bounds);

private:

    // *** Private Methods ***

    float calcInt(bool trig, bool display);
    float lockin(float nextVal);
    float localSmooth(float nextVal);

    // *** Private Members ***

    VideoCapture cap;
    Mat image;
    Mat thresh_image;
    Mat trig_image;

    Rect trig_roi;
    Rect sig_roi;

    bool thresh_plotted;
    bool trig_plotted;

    double fps;
    double frames;

    double currentFrame;
    double currentTime;
    double offsetTime;

    Point2d *timeBounds;

    string winname;

    string filepath;

    vector<Point2d> triglog;
    vector<Point2d> siglog;

    vector<Point2d> *fft_amp;
    vector<Point2d> *fft_arg;

    int* thresh;

    bool logging;

    bool trigRoiSet;
    bool sigRoiSet;
};

#endif // FEED_H
