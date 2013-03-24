#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>

typedef unsigned int uInt;

using namespace std;
using namespace cv;


// *** GLOBALS ***

vector<

bool initialize();
void extract(Mat shape, Mat src, Mat axes, Mat *trans, Mat *rot);
void mapShape(Mat *shape, Mat up);

int main()
{
    // initialize

    if(!initialize())
        return -1;

    // compute average z direction

    for(int i=0; i < traj.rows; i++)
    {

    }

    return 0;
}

void extract(Mat shape, Mat src, Mat axes, Mat *trans, Mat *rot)
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
        transpose(shape.row(i),x1);
        x2 = src.row(i);

        C += x2*x1;
    }

    C = C/N;

    Mat w, u, vt;
    SVD::compute(C, w, u, vt);

    *rot = u*vt;
    *trans = cent;
}

void mapShape(Mat *shape, Mat up)
{
    // rotate z onto up

    Mat axes = Mat::zeros(3,3,CV_32FC1);
    Mat *trans, *rot;

    extract(*shape, up, axes, trans, rot);

    // map shape by rotation

    *shape = (*rot)*(*shape);
}
