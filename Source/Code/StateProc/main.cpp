#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

typedef unsigned int uInt;

using namespace std;
using namespace cv;

// *** GLOBAL CONSTANTS ***

#define NUM_POINTS 4

namespace BODY{

Point3f points[NUM_POINTS] =
{
    Point3f(440, 440, 0),
    Point3f(440, -440, 0),
    Point3f(-440, -440, 0),
    Point3f(-440, 440, 0)
};
}

#define WRITE_ARG   1
#define READ_ARG    2
#define MAX_ARG     3

// *** GLOBAL VARIABLES ***

Mat shape;
Mat c_axes;

vector<Mat> pos;

string triang_path;
string write_path;

Mat attitude;
Mat trajectory;


// *** GLOBAL FUNCTIONS ***

bool initialize(int argc, char** argv);
void extract(Mat source, Mat dest, Mat *trans, Mat *rot, bool zero=false);
bool readTriang();
Mat compAxes(Mat &zs);
float lenMat(Mat src);
Mat crossMat(Mat A, Mat B);
Mat getAtt(Mat R);
void toXML();
void toCSV();


// *** AUX FUNCTIONS ***

template <class TYPE>
string toStr(TYPE in)
{
    stringstream ss;
    ss << in;
    return ss.str();

}


// *** MAIN FUNCTION ***


int main(int argc, char** argv)
{
    // initialize

    if(!initialize(argc, argv))
        return -1;

    // compute average z direction

    int num_times = pos.size();
    Mat trans, rot;
    Mat zs = Mat::zeros(num_times, 3, CV_32F);

    for(int i = 0; i < num_times; i++)
    {
        // compute rotation from known shape to position

        extract(shape, pos.at(i), &trans, &rot, true);

        // map 'z' by rotation matrix

        transpose(rot*c_axes.col(2), zs.row(i));
    }

    // compute the world frame axes

    //cout << "Camera Axes: \n" << c_axes << endl;

    Mat axes = compAxes(zs);

    //cout << "World Frame Axes: \n" << axes << endl;

    // compute the rotation matrix from the camera frame to the world frame

    extract(axes, c_axes, &trans, &rot, false);

    // rotate shape into world reference frame

    Mat rot_w(3,3,CV_32F);
    rot.copyTo(rot_w);

    //cout << rot << endl;
    //cout << axes << endl;

    //cout << "Before: \n" << shape << endl << endl;

    shape = (rot*shape.t()).t();

    //cout << "After \n" << shape << endl << endl;

    // calculate the rotation matrices from the new shape to the positions

    Mat traj;

    for(int i = 0; i < num_times; i++)
    {
        extract(shape, pos.at(i), &trans, &rot, true);

        //cout << "Rotation Matrix \n" << rot << endl << endl;

        attitude.push_back(getAtt(rot));

        //cout << "Attitude: \t" << attitude.row(i) << endl;
        //cout << "Translation: \t" << trans << endl;

        traj = (rot_w.inv()*trans.t()).t();

        trajectory.push_back(traj);

        //cout << "Trajectory: \t" << trajectory.row(i) << endl << endl;
    }

    // export data

    toXML();
    toCSV();

    return 0;
}

bool initialize(int argc, char** argv)
{
    if(argc < MAX_ARG)
        return false;

    // initialize the body shape

    shape = Mat::zeros(NUM_POINTS, 3, CV_32F);

    for(int i=0; i < NUM_POINTS; i++)
    {
        shape.at<float>(i,0) = BODY::points[i].x;
        shape.at<float>(i,1) = BODY::points[i].y;
        shape.at<float>(i,2) = BODY::points[i].z;
    }

    c_axes = Mat::zeros(3,3, CV_32F);

    for(int i=0; i < 3; i++)
    {
        c_axes.at<float> (i,i) = 1;
    }

    // initialize read/write paths

    triang_path = argv[READ_ARG];
    write_path = argv[WRITE_ARG];

    // read in triangulation data

    if(!readTriang())
        return false;

    // initialize attitude and trajectory MAtrices

    attitude = Mat::zeros(0,3,CV_32F);
    trajectory = Mat::zeros(0,3,CV_32F);

    return true;
}

void extract(Mat source, Mat dest, Mat *trans, Mat *rot, bool zero)
{
    int N = source.rows;

    Mat src(N,3,CV_32F);
    Mat dst(N,3,CV_32F);

    source.copyTo(src);
    dest.copyTo(dst);

    // compute centroid

    Mat cent1, cent2;
    reduce(dst,cent1,0,CV_REDUCE_AVG,CV_32F);
    reduce(src,cent2,0,CV_REDUCE_AVG,CV_32F);

    // translate to zero

    if(zero)
    {
        for(int i=0; i < N; i++)
        {
            dst.row(i) = dst.row(i) - cent1;
            src.row(i) = src.row(i) - cent2;
        }
    }

    // compute rotation matrix

    Mat x1,x2;

    Mat C = Mat::zeros(3,3,CV_32F);

    for(int i=0; i < N; i++)
    {

        x1 = src.row(i);
        x2 = dst.row(i).t();

        C += x2*x1;
    }

    C = C/N;

    Mat w, u, vt;
    SVD::compute(C, w, u, vt);

    Mat r1 = u*vt;

    Mat factor = Mat::eye(3,3,CV_32F);
    factor.at<float>(2,2) = determinant(r1);

    *rot = u*factor*vt;
    *trans = cent1-cent2;

//    //cout << "Trans: " << *trans << endl;

//    cout << "\n\n\nSrc: \n" << src << endl;
    cout << "\n\n\nDst: \n" << dst << endl;
    cout << "Rot: \n" << *rot << endl;
    cout << "Back: \n" << ((*rot)*src.t()).t() << endl;
}

bool readTriang()
{
    // load handler and check number of points

    FileStorage handle(triang_path, FileStorage::READ);

    if(!handle.isOpened())
    {
        cerr << "Failed to open file..." << triang_path;
        return false;
    }

    int num_points;
    handle["NumPoints"] >> num_points;

    if(num_points != NUM_POINTS)
    {
        cerr << "Given triangulation file has the incorrect number of tracking points...";
        return false;
    }

    // assume correct number of points tracked

    FileStorage fs[NUM_POINTS];
    string path;
    vector<Point3f> points[NUM_POINTS];

    // load triangulation files

    for(int i=0; i < NUM_POINTS; i++)
    {
        handle["file_"+toStr(i)] >> path;
        fs[i].open(path, FileStorage::READ);

        if(!fs[i].isOpened())
        {
            cerr << "Failed to open file..." << path;
            return false;
        }

        fs[i]["point"] >> points[i];
    }

    // load points into pos matrix

    int num_times = points[0].size(); // assume all points triangulated the same number of times

    for(int i=0; i < num_times; i++)
    {
        pos.push_back(Mat::zeros(NUM_POINTS,3,CV_32F));

        for(int j=0; j < NUM_POINTS; j++)
        {
            pos.at(i).at<float>(j,0) = points[j].at(i).x;
            pos.at(i).at<float>(j,1) = points[j].at(i).y;
            pos.at(i).at<float>(j,2) = points[j].at(i).z;
        }
    }

    return true;
}

Mat compAxes(Mat &zs)
{
    // take the average z orientation as up

    Mat axes = Mat::zeros(3,3,CV_32F);

    reduce(zs,axes.row(2),0,CV_REDUCE_AVG,CV_32F);

    // take the cross product of the z with camera x axis to get y

    axes.row(2).cross(c_axes.row(0)).copyTo(axes.row(1));
    axes.row(1) = axes.row(1)/lenMat(axes.row(1));

    axes.row(1).cross(axes.row(2)).copyTo(axes.row(0));
    axes.row(0) = axes.row(0)/lenMat(axes.row(0));

    return axes;
}

float lenMat(Mat src)
{
    float length=0;

    for(int y=0; y < src.rows; y++)
    {
        for(int x=0; x < src.cols; x++)
        {
            length += pow(src.at<float>(y,x),2);
        }
    }

    return sqrt(length);
}

Mat getAtt(Mat R)
{
    float st[3];
    Mat att = Mat::zeros(1,3, CV_32F);

    // compute yaw

    st[0] = atan2(R.at<float>(1,0), R.at<float>(0,0));

    // compute pitch

    st[1] = asin(-1*R.at<float>(2,0));

    // compute roll

    st[2] = atan2(R.at<float>(2,1), R.at<float>(2,2));

    att.at<float>(0,0) = st[0];
    att.at<float>(0,1)  = st[1];
    att.at<float>(0,2)  = st[2];

    return att;
}

void toXML()
{
    FileStorage fs(write_path, FileStorage::WRITE);

    if(!fs.isOpened())
    {
        cerr << "Failed to open file..." << write_path;
        return;
    }

    // write data

    fs << "yaw" << attitude.col(0);
    fs << "pitch" << attitude.col(0);
    fs << "roll" << attitude.col(0);

    fs << "x" << trajectory.col(0);
    fs << "y" << trajectory.col(0);
    fs << "z" << trajectory.col(0);
}

void toCSV()
{
    string path = write_path.substr(0, write_path.length()-4) + ".txt";

    ofstream file;
    file.open(path.c_str());

    for(int i=0; i < attitude.rows; i++)
    {
        for(int j=0; j<3; j++)
        {
            file << attitude.at<float>(i,j) << '\t';
        }

        for(int j=0; j<3; j++)
        {
            file << trajectory.at<float>(i,j) << '\t';
        }

        file << endl;
    }
}
