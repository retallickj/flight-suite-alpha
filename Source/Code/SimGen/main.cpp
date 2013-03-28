#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <sstream>

using namespace std;
using namespace cv;

// DEFINES

// -- video properties

namespace VID
{

const int FPS = 120;
const int X = 800;
const int Y = 600;

const int LENGTH_MS = 20000;
const int NUM_FEEDS = 2;

const Scalar BKGRND = Scalar(255,255,0);

//  trigger settings

const Point2i TRIG_POS = .8*Point2i(X, Y);
const int TRIG_SIZE = 6;
const Scalar TRIG_COL  = Scalar(255, 0, 0);
const int TRIG_NOISE = 10;
const double TRIG_START_MS = 3250;
const double TRIG_END_MS = 12820.5;

//  signal settings

const Point2i SIG_POS = .9*Point2i(X, Y);
const int SIG_SIZE = 50;
const Scalar SIG_COL  = Scalar(0, 255, 0);
const int SIG_NOISE = 10;
const float SIG_FREQ = 10.15;

// offset bounds

const double OFFSET_SCALE_MS = 2000;
}

// -- file properties

#define WRITE_NAME "./output"
#define PI 3.14159265358979323846264338


// GLOBALS VARIABLES

int num_feeds;
double current_time;
int current_frame;

vector<VideoWriter> outputs;
vector<double> offsets;
vector<Mat> images;


// FUNCTION DECLARATIONS

bool initiate(int argc, char** argv);

void cleanImages();

void addTrig();
void addSig();

void addPoints();

void writeVid();

void genNoiseMat(Mat &dest, float scale, Scalar base);
void insertMat(Mat &dest, Mat &src, Point2i origin);

template <class T>
string toStr(T src)
{
    stringstream ss;
    ss << src;
    return ss.str();
}

int main(int argc, char **argv)
{
    if(!initiate(argc, argv))
        return -1;

    for(int i=0; i < VID::LENGTH_MS*VID::FPS*.001;i++)
    {
        // clean images

        cleanImages();

        // add trig and sig

        //addTrig();
        addSig();

        // write

        writeVid();

        current_time += 1./VID::FPS;
        current_frame++;
    }



    return 0;
}


// FUNCTION DEFINITIONS

bool initiate(int argc, char **argv)
{
    num_feeds = VID::NUM_FEEDS;

    current_time=0;
    current_frame=0;

    srand(time(NULL));

    outputs.resize(num_feeds);
    images.resize(num_feeds);
    offsets.resize(num_feeds);

    Size S = Size(VID::X, VID::Y);

    for(int i=0; i < num_feeds; i++)
    {
        outputs.at(i).open(WRITE_NAME+toStr(i),CV_FOURCC('X', 'V', 'I', 'D'),VID::FPS, S, true);
        images.at(i) = Mat(S, CV_8UC3);
        if(i)
            offsets.at(i) = rand()*VID::OFFSET_SCALE_MS/RAND_MAX;
        else
            offsets.at(i) = 0;

        // check file open

        if(!outputs.at(i).isOpened())
        {
            cout << "Failed to open file...\t" << toStr(i) << endl;
            return false;
        }

        cout << "Loaded feed " << i << "\t offset: " << offsets.at(i) << endl;
    }

    return true;
}

void cleanImages()
{
    for(int i=0; i < num_feeds; i++)
    {
        images.at(i).setTo(VID::BKGRND);
    }
}

void addTrig()
{
    Mat trig = Mat(VID::TRIG_SIZE, VID::TRIG_SIZE, CV_8UC3);
    float scale;
    double loc_time;
    Scalar col(0,0,0);

    cout << "Trig:\t";

    for(int i=0; i < num_feeds; i++)
    {
        // compute trig intensity

        loc_time = current_time*1000 + offsets.at(i);

        if(loc_time < VID::TRIG_START_MS || loc_time > VID::TRIG_END_MS)
            scale = 0;
        else
            scale = .8;

        // generate noise

        col[0] = VID::TRIG_COL[0]*scale;
        col[1] = VID::TRIG_COL[1]*scale;
        col[2] = VID::TRIG_COL[2]*scale;

        genNoiseMat(trig, VID::TRIG_NOISE, col);

        trig += col;

        // insert into image

        insertMat(images.at(i), trig, VID::TRIG_POS);

        cout << trig << '\t';
    }

}

void addSig()
{
    Mat sig(VID::SIG_SIZE, VID::SIG_SIZE, CV_8UC3);
    float scale;
    double loc_time;
    Scalar col;

    cout << "Sig: \t";

    for(int i=0; i < num_feeds; i++)
    {
        // compute trig intensity

        loc_time = current_time*1000 + offsets.at(i);

        scale = .5 + .3*sin(2*PI*VID::SIG_FREQ*loc_time*.001) + .1*(rand()*2./RAND_MAX - 1);

        // generate noise

        // generate noise

        col[0] = VID::SIG_COL[0]*scale;
        col[1] = VID::SIG_COL[1]*scale;
        col[2] = VID::SIG_COL[2]*scale;

        genNoiseMat(sig, VID::SIG_NOISE, col);

        // insert into image

        insertMat(images.at(i), sig, VID::SIG_POS);

        cout << col << '\t';
    }

    cout << endl;
}

void addPoints()
{

}

void writeVid()
{
    cout << "Writing video ...\t" << current_frame << '\t' << current_time << endl;

    for(int i=0; i < num_feeds; i++)
    {
        outputs.at(0) << images.at(i);
    }
}

void genNoiseMat(Mat &dest, float scale, Scalar base)
{
    int X, Y;

    X = dest.cols;
    Y = dest.rows;

    Scalar noise;

    for(int x=0; x < X; x++)
    {
        for(int y=0; y < Y; y++)
        {
            for(int i=0; i < 3; i++)
            {
                noise[i] = (rand()*2./RAND_MAX-1)*scale;
                if(noise[i]+base[i] > 255)
                    noise[i] = 255 - base[i];
                else if(noise[i]+base[i] < 0)
                    noise[i] = -base[i];
            }

            dest.at<Scalar>(y, x) = noise;
        }
    }
}


void insertMat(Mat &dest, Mat &src, Point2i origin)
{
    if((origin.x > dest.cols-1) || (origin.y >= dest.rows))
        return;

    int X, Y;
    int x, y;

    X = src.cols;
    Y = src.rows;

    for(int i = 0; i < X; i++)
    {
        x = (origin.x - X/2) + i;

        if(x == X)
            break;

        for(int j=0; j < Y; j++)
        {
            y = (origin.y - Y/2) + j;

            if(y == Y)
                break;

            dest.at<Scalar>(y,x) = src.at<Scalar>(j,i);
        }
    }
}

