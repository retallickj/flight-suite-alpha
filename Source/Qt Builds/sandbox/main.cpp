#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "fftw3.h"

#include <iostream>
#include <sstream>

#define PI 3.1415926
#define FFT_FREQ_MIN .2

typedef unsigned int uINT;

using namespace std;
using namespace cv;

void genData(vector<Point2f> *dest, float (*func)(float, float[]), float xlow, float xhigh, int incs, float args[]);
float func(float x, float args[]);

void fft(vector<Point2f> *vals, vector<Point2f> *abs, vector<Point2f> *arg);

int main(int argc, char** argv)
{
    if(argc < 6)
        return -1;

    srand(time(NULL));

    vector<Point2f> *abs = new vector<Point2f>;
    vector<Point2f> *arg = new vector<Point2f>;
    vector<Point2f> *vals = new vector<Point2f>;

    float xlow = strtof(argv[1], NULL);
    float xhigh = strtof(argv[2], NULL);
    int incs = (int) strtof(argv[3], NULL);
    float f = strtof(argv[4], NULL);
    float phase = strtof(argv[5], NULL);
    float args[2] = {f, phase};

    genData(vals,func,xlow, xhigh, incs, args);

    fft(vals, abs, arg);

    for(uINT i=0; i < abs->size(); i++)
    {
        cout << "\nFreq:\t" << abs->at(i).x;
        cout << "\tAbs:\t" << abs->at(i).y;
        cout << "\tPhase:\t" << arg->at(i).y;
    }

    delete(abs);
    delete(arg);
    delete(vals);

    return 0;
}

void genData(vector<Point2f> *dest, float (*func)(float, float[]), float xlow, float xhigh, int incs, float args[])
{
    // clear dest

    dest->clear();

    // fill vector

    float dx = (xhigh-xlow)/(incs-1);
    Point2f p;

    for(float x = xlow; x <= xhigh; x += dx)
    {
        p.x = x;
        p.y = func(x, args);

        dest->push_back(p);
    }
}


float func(float x, float args[])
{
    float f = args[0];
    float phase = args[1];
    float noise = (rand()*2./RAND_MAX - 1);
    if(cos(2*PI*f*x + phase) > 0)
        return 1 + .2*noise;
    else
        return -1 + .2*noise;
}

void fft(vector<Point2f> *vals, vector<Point2f> *mag, vector<Point2f> *phase)
{
    fftw_complex *in, *out;
    fftw_plan p;

    uINT N = vals->size();

    float fps = 1./(vals->at(1).x - vals->at(0).x);

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // initialize input

    for(uINT i = 0; i < N; i++)
    {
        in[i][0] = vals->at(i).y;
        in[i][1] = 0;
    }

    // execute fft

    fftw_execute(p); // out now loaded with fft coefs (2D length N array)


    // format output

    // -- clear

    mag->clear();
    phase->clear();

    // -- port out to dest

    Point2d amp_p;
    Point2d arg_p;

    complex<double> coef;

    for(uINT i = 0; i < N; i++)
    {
        coef = complex<double>(out[i][0], out[i][1]);

        amp_p.x = (float) (i*fps)/N; // freq: scale by fps/N
        arg_p.x = amp_p.x;

        if(amp_p.x < FFT_FREQ_MIN)
        {
            amp_p.y = 0;
            arg_p.y = 0;
        }
        else
        {
            amp_p.y = abs(coef);
            arg_p.y = arg(coef);
        }

        mag->push_back(amp_p);
        phase->push_back(arg_p);
    }
}

