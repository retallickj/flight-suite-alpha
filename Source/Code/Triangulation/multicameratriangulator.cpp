#include "multicameratriangulator.h"

/**
 From "Triangulation", Hartley, R.I. and Sturm, P., Computer vision and image understanding, 1997
 */
multiCameraTriangulator::multiCameraTriangulator(int noOfCameras,vector<string> paramFiles)
{
    Mat R,T;
    cameras.resize(noOfCameras);
    projections.resize(noOfCameras);
    distCoefs.resize(noOfCameras);

    for(int i = 1; i < noOfCameras; i++)
    {

        FileStorage parameters;
        parameters.open(paramFiles[i-1],FileStorage::READ);
        parameters["Camera_Matrix_One"] >> cameras[0];
        parameters["Camera_Matrix_Two"] >> cameras[1];
        parameters["R"] >> R;
        parameters["T"] >> T;
        parameters["Distortion_Coefficients_One"] >> distCoefs[0];
        parameters["Distortion_Coefficients_Two"] >> distCoefs[1];

        parameters.release();

        Mat Trans = Mat::zeros(3, 4, CV_64F);

        if(i == 1)
        {
            projections[0] = Mat(3, 4, CV_64F,Scalar(0));
            for(int i = 0; i < 3; i++)
                for(int j = 0; j < 3; j++)
                    projections[0].at<double>(i,j) = cameras[0].at<double>(i,j);
        }

        projections[i] = Mat(3, 4, CV_64F,Scalar(0));

        for(int j = 0; j < 3; j++)
        {
            for(int k = 0; k < 3; k++)
            {
                Trans.at<double>(j,k) = R.at<double>(j,k);
            }
            Trans.at<double>(j,3) = T.at<double>(j);
        }

        projections[i] = cameras[i]*Trans;
    }
}

Point3f multiCameraTriangulator::LinearLSTriangulation(vector<Point3f> points,vector<double>& weightings)

{
    //build matrix A for homogenous equation system Ax = 0
    //assume X = (x,y,z,1), for Linear-LS method
    //which turns it into a AX = B system, where A is 4x3, X is 3x1 and B is 4x1
    Mat A(points.size()*2,3,CV_64F);
    Mat B(points.size()*2,1,CV_64F);
    for(unsigned int i = 0; i < points.size(); i++)
    {
        //double test = projections[i].at<float>(2,0);
        A.at<double>(i*2,0) = (points[i].x*projections[i].at<double>(2,0)-projections[i].at<double>(0,0))/weightings[i];
        A.at<double>(i*2,1) = (points[i].x*projections[i].at<double>(2,1)-projections[i].at<double>(0,1))/weightings[i];
        A.at<double>(i*2,2) = (points[i].x*projections[i].at<double>(2,2)-projections[i].at<double>(0,2))/weightings[i];

        A.at<double>(i*2+1,0) = (points[i].y*projections[i].at<double>(2,0)-projections[i].at<double>(1,0))/weightings[i];
        A.at<double>(i*2+1,1) = (points[i].y*projections[i].at<double>(2,1)-projections[i].at<double>(1,1))/weightings[i];
        A.at<double>(i*2+1,2) = (points[i].y*projections[i].at<double>(2,2)-projections[i].at<double>(1,2))/weightings[i];

        B.at<double>(i*2) = -(points[i].x*projections[i].at<double>(2,3)-projections[i].at<double>(0,3))/weightings[i];
        B.at<double>(i*2+1) = -(points[i].y*projections[i].at<double>(2,3)-projections[i].at<double>(1,3))/weightings[i];
    }

    Mat X;
    int test = solve(A,B,X,DECOMP_SVD);

    Point3f result;
    result.x = X.at<double>(0);
    result.y = X.at<double>(1);
    result.z = X.at<double>(2);

    return result;
}

Point3f multiCameraTriangulator::iterativeLinearLSTriangulation(vector<Point3f> points,double epsilon,int iterations)
{
    vector<double> weightings(points.size(),1);
    vector<double> prevWeightings;
    Point3f X;

    for(int i = 0; i < iterations; i++)
    {
        bool quit = true;
        X = LinearLSTriangulation(points,weightings);
        prevWeightings = weightings;

        for(unsigned int j = 0; j < points.size(); j++)
        {
            weightings[j] = projections[j].at<double>(2,0)*X.x+projections[j].at<double>(2,1)*X.y+projections[j].at<double>(2,2)*X.z+projections[j].at<double>(2,3);
            if(abs(weightings[j] - prevWeightings[j]) > epsilon)
                quit = false;
        }
        if(quit == true)
            break;
    }
    return X;
}
