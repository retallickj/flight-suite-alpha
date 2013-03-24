#ifndef PLOTCORE_H
#define PLOTCORE_H

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <sstream>

using namespace std;
using namespace cv;

typedef unsigned int uInt;

namespace PCG
{
const Scalar RED = Scalar(0,0,255);
const Scalar GREEN = Scalar(0,255,0);
const Scalar BLUE = Scalar(255,0,0);
const Scalar BLACK = Scalar(0,0,0);
const Scalar WHITE = Scalar(255,255,255);

const int LINE_WIDTH = 1;
const int BOUND_WIDTH = 15;
const int STEM_WIDTH = 1;
const int CIRC_RAD = 2;

const float AXIS_DASH = 5;
const int DASH_WIDTH = 1;

const float PLOT_OFFSET = .1;

const int TITLE_OFFSET = 15;
const double TITLE_SCALE = .4;
const int TITLE_FONT = CV_FONT_HERSHEY_COMPLEX;
const int TITLE_THICK = 1;
const Scalar TITLE_COLOR = BLACK;

const int LABEL_OFFSET = 10;
const double LABEL_SCALE = .4;
const int LABEL_FONT = CV_FONT_HERSHEY_COMPLEX;
const int LABEL_THICK = 1;
const Scalar LABEL_COLOR = BLACK;

const Scalar VLINE_COLOR = RED;
const int VLINE_THICK = 3;


enum STYLE{STEM, CROSS, CIRC, PLUS, LINE};

void mouseCallback(int event, int x, int y, int flag, void* param);

template <class TYPE>
string toStr(TYPE in)
{
    stringstream ss;
    ss << in;
    return ss.str();
}
}

struct Series{
    vector<Point2d> *data;
    PCG::STYLE style;
    Scalar color;

    string title;
    Point2d ll_act;
    Point2d rr_act;

    float xscale;
    float yscale;
};

struct Plot{
    Mat image;

    string title;
    string xlabel;
    string ylabel;

    Point2d *ll_act; // lower left
    Point2d *rr_act; // upper right

    float xscale;
    float yscale;

    vector<struct Series> series;
};

class PlotCore
{
public:
    PlotCore(string winname, int width, int height, bool cycleSeries=false);
    ~PlotCore();

    void addPlot();
    void addSeries(int plot_index, vector<Point2d> *src, string title="", Scalar color=PCG::GREEN,\
                   PCG::STYLE style = PCG::STEM);

    void changePlot(int plot_index, bool showflag);
    void shiftPlot(bool right, bool showflag);

    void changeSeries(int series_index, bool showflag);
    void shiftSeries(bool right, bool showflag);

    void plot(int plot_index);

    void hide();
    void show(vector<int*> *val=0, int maxVal=-1, int* maxX=0, int maxX_Lim = -1);

    void update(int plot_index);
    void updateAll(bool showflag);

    void clear(int plot_index);
    void clearAll();

    void emptyPlot(int plot_index);
    void emptyAllPlots();

    void addVLine(int x, Scalar color = PCG::VLINE_COLOR, int thickness = PCG::VLINE_THICK);
    Point2d getPoint(int plot_index, int x, int y);

    void setLabels(string title, string xlabel, string ylabel);

    void addHLine(int *y);

    int l1, l2;

private:

    // private methods

    void drawBoundBox(int plot_index);
    void drawAxes(int plot_index);
    void drawLabels(int plot_index);
    void drawDash(Mat &img, Point2i p1, Point2i p2);
    void drawSeries(struct Plot *plt, struct Series *src);
    void drawHLines();

    void checkBounds(struct Plot *p, struct Series *ser);

    string winname;
    string trackname;

    Point2i *ll;
    Point2i *rr;

    Point2i dims;

    int currentPlot;
    int currentSeries;

    bool cycleSeries;
    int* maxX;

    vector<int*> hlines;

    vector<struct Plot*> plots;
};

#endif // PLOTCORE_H
