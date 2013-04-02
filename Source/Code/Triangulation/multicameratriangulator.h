#ifndef multiCamTria_h
#define multiCamTria_h

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

class multiCameraTriangulator{
public:
    multiCameraTriangulator(int noOfCameras,vector<string> paramFiles);

    Point3f LinearLSTriangulation(vector<Point3f> points,vector<double>& weightings);
    Point3f iterativeLinearLSTriangulation(vector<Point3f> points,double epsilon,int iterations);

    vector<Mat> cameras;
    vector<Mat> projections;
    vector<Mat> distCoefs;
    Mat affineTrans;
};

#endif
