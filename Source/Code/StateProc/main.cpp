#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>

typedef unsigned int uInt;

using namespace std;
using namespace cv;


// *** GLOBALS ***

bool initialize();
void extract(Mat dest, Mat src, Mat *trans, Mat *rot);
void mapdest(Mat *dest, Mat up);

int main()
{
    // initialize

    if(!initialize())
        return -1;

    // compute average z direction



    return 0;
}

bool initialize()
{
    return true;
}

void extract(Mat dest, Mat src, Mat *trans, Mat *rot)
{
    int N = src.rows;

    // compute centroid

    Mat cent;
    reduce(src,cent,0,CV_REDUCE_AVG,CV_32FC1);

    // translate to zero

    for(int i=0; i < N; i++)
    {
        src.row(i) = src.row(i) - cent;
    }

    // compute rotation matrix

    Mat x1,x2;

    Mat C = Mat::zeros(3,3,CV_32FC1);

    for(int i=0; i < N; i++)
    {
        transpose(dest.row(i),x1);
        x2 = src.row(i);

        C += x2*x1;
    }

    C = C/N;

    Mat w, u, vt;
    SVD::compute(C, w, u, vt);

    *rot = u*vt;
    *trans = cent;
}

void mapdest(Mat *dest, Mat up)
{
    // rotate z onto up

    Mat *trans = 0, *rot = 0;

    extract(*dest, up, trans, rot);

    // map dest by rotation

    *dest = (*rot)*(*dest);
}
