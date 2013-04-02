// multicamera vision.cpp : Defines the entry point for the console application.
//


#include "multicameratriangulator.h"

#include <iostream>
#include <fstream>


#define EPSILON 0.1
#define ITERATIONS 10
#define FEED_ARG_OFFSET 2
#define POINTS_ARG 2
#define PARAM_ARG 3
#define WRITE_ARG 1
#define LARGE_NUMBER 99999999999999999999999.0

using namespace cv;
using namespace std;

int strToInt(string myString)
{
    stringstream ss(myString);
    int myInt;
    ss >> myInt;
    return myInt;
}

template <class TYPE>
string toStr(TYPE in)
{
    stringstream ss;
    ss << in;
    return ss.str();
}


int main(int argc, char** argv)
{
    if(argc < PARAM_ARG+1)
        return -1;

    vector<string> pointFiles;
    vector<string> paramFiles;

    string pnt_path = argv[POINTS_ARG];
    string param_path = argv[PARAM_ARG];

    // load handlers

    FileStorage fs1(pnt_path,FileStorage::READ);
    FileStorage fs2(param_path,FileStorage::READ);

    if(!fs1.isOpened())
    {
        cerr << "Failed to open file..." << pnt_path;
        return -1;
    }

    if(!fs2.isOpened())
    {
        cerr << "Failed to open file..." << param_path;
        return -1;
    }

    // load points

    int numFeeds;
    int numPoints;
    int numParams;

    fs1["num_feeds"] >> numFeeds;
    fs1["num_points"] >> numPoints;
    fs2["num_params"] >> numParams;

    if(numParams != numFeeds - 1)
    {
        cerr << "Incorrect number of parameter files...";
        return -1;
    }

    string name, path;

    for(int i=0; i < numFeeds; i++)
    {
        name = "feed_path_" + toStr(i);
        fs1[name] >> path;
        pointFiles.push_back(path);
    }

    for(int i=0; i < numParams; i++)
    {
        name = "param_path_" + toStr(i);
        fs2[name] >> path;
        paramFiles.push_back(path);
    }

    // handle write filename

    string write_path = argv[WRITE_ARG];

    // release handlers

    fs1.release();
    fs2.release();

    multiCameraTriangulator triangulator(numFeeds,paramFiles);

    vector<FileStorage> feeds(numFeeds);
    vector<FileStorage> points(numPoints);
    vector<vector<vector<Point3f> > > inPoints;
    vector<vector<vector<Point3f> > > inPointsUndist;
    vector<vector<Point3f> > outPoints;
    vector<Point3f> triangulatePoints;
    vector<vector<float> > timeStamps;
    vector<vector<unsigned int> > indexes;
    string fileName;
    string pointName;

    inPoints.resize(numFeeds);
    inPointsUndist.resize(numFeeds);
    indexes.resize(numFeeds);
    triangulatePoints.resize(numFeeds);
    outPoints.resize(numPoints);
    timeStamps.resize(numPoints);

    for(int i = 0; i < numFeeds; i++)
    {
        fileName = pointFiles[i];
        //bool temp =feeds[i].open(fileName,FileStorage::READ);
        feeds[i].open(fileName,FileStorage::READ);
        inPoints[i].resize(numPoints);
        indexes[i].resize(numPoints);

        for(int j = 0; j < numPoints; j++)
        {
            pointName = "points_" + format("%d",j);
            feeds[i][pointName] >> inPoints[i][j];
            indexes[i][j] = 1;
        }
    }

    bool loop = true;

    //loop which interpolates between points and triangulates positions
    while(loop)
    {
        float minTime = LARGE_NUMBER;
        int feedNum;

        //loop through feeds and determine which feed has its second point at the earliest time
        for(int j = 0; j < numFeeds; j++)
        {
            for(int k = 0; k < numPoints; k++)
            {
                if(inPoints[j][k][indexes[j][k]].z < minTime)
                {
                    minTime = inPoints[j][k][indexes[j][k]].z;
                    feedNum = j;
                }
            }
        }

        //loop through points and interpolate at the 2nd point of the earliest feed between the 1st and 2nd points of all other feeds
        for(int k = 0; k < numPoints; k++)
        {
            bool triangulate = true;
            //loop through feeds for given point
             for(int j = 0; j < numFeeds; j++)
             {
                                  //if the 1st point of any other feed is after the second point of the earliest feed dont triangulate
                if(inPoints[j][k][indexes[j][k]-1].z > inPoints[j][k][indexes[feedNum][k]].z)
                {
                         triangulate = false;
                }
                else
                {
                     vector<Point2f> undistortMat;
                     undistortMat.resize(1);
                     vector<Point2f> outMat;
                     outMat.resize(1);

                     //if the feed is the one with the earliest 2nd point no interpolation necessary
                     if(feedNum == j)
                     {
                         undistortMat[0].x = inPoints[j][k][indexes[j][k]].x;
                         undistortMat[0].y = inPoints[j][k][indexes[j][k]].y;
                     }
                     //for all other feeds interpolate between 1st and 2nd points
                     else
                     {

                         double interpFact = (minTime - inPoints[j][k][indexes[j][k]-1].z)/(inPoints[j][k][indexes[j][k]].z - inPoints[j][k][indexes[j][k]-1].z);
                         undistortMat[0].x = inPoints[j][k][indexes[j][k]-1].x + (inPoints[j][k][indexes[j][k]].x - inPoints[j][k][indexes[j][k]-1].x)*interpFact;
                         undistortMat[0].y = inPoints[j][k][indexes[j][k]-1].y + (inPoints[j][k][indexes[j][k]].y - inPoints[j][k][indexes[j][k]-1].y)*interpFact;
                     }

                     //undistort the interpolated point
                     undistortPoints(undistortMat,outMat,triangulator.cameras[j],triangulator.distCoefs[j]);

                     //undistort normalizes the points, so we have to multiply by the camera matrix to return it to pixel coordinates, haven't found a way to avoid this
                     triangulatePoints[j].x = outMat[0].x*triangulator.cameras[j].at<double>(0,0)+triangulator.cameras[j].at<double>(0,2);
                     triangulatePoints[j].y = outMat[0].y*triangulator.cameras[j].at<double>(1,1)+triangulator.cameras[j].at<double>(1,2);
                     triangulatePoints[j].z = 1;
                }
             }

             //triangulate 3d point
             if(triangulate)
             {
                outPoints[k].push_back(triangulator.iterativeLinearLSTriangulation(triangulatePoints,EPSILON,ITERATIONS));
                timeStamps[k].push_back(minTime);
             }

             //increase index of the earliest feed
             indexes[feedNum][k] ++;
        }

        //check and see if the end of any feed has been reached
        for(int j = 0; j < numFeeds; j++)
        {
            for(int k = 0; k < numPoints; k++)
            {
                if(inPoints[j][k].size() <= indexes[j][k])
                {
                    loop = false;
                }
            }
        }
    }

    //output data to xml and tab seperated files
    string fileFolder = write_path;
    for(int i = 0; i < numPoints; i++)
    {
        //bool temp = points[i].open(fileFolder + "point" + format("%d.xml",i),FileStorage::WRITE);
        points[i].open(fileFolder + "point" + format("%d.xml",i),FileStorage::WRITE);
        points[i] << "point" << outPoints[i];
        points[i] << "timeStamps" << timeStamps[i];

        ofstream myfile;
        string filename = fileFolder + "point" + format("%d.txt",i);
        myfile.open(filename.c_str());
        for(unsigned int j = 0; j < outPoints[0].size();j++)
        {
            myfile << outPoints[i][j].x << "\t" << outPoints[i][j].y << "\t" << outPoints[i][j].z <<"\n";
        }
        myfile.close();
    }
    return 0;


}
