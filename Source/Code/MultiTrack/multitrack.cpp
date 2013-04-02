#include "multitrack.h"

#include <sstream>



// *** General Functions ***

void drawCross(Mat& image, Point pos, Scalar color)
{
    line(image, pos - L*Point(1,1), pos + L*Point(1,1), color, 2);
    line(image, pos - L*Point(1,-1), pos + L*Point(1,-1), color, 2);
}

void genHSVbounds(int H, int &S_low, int &S_high, int &V_low, int &V_high)
{
    S_low=S_LOW;
    V_low=V_LOW;s
    S_high=S_MAX;
    V_high=V_MAX;
}

Mat* crop(Mat* src, Rectangle* p_rect)
{
    Point2f p1 = p_rect->center - .5*(p_rect->width*Point(1,0) + p_rect->height*Point(0,1));
    Point2f p2 = p_rect->center + .5*(p_rect->width*Point(1,0) + p_rect->height*Point(0,1));

    // bounding and satisfying conditions

    if(p1.x < 0) p1.x = 0;
    if(p2.x >= src->cols) p2.x = src->cols-1;
    if(p1.y < 0) p1.y = 0;
    if(p2.y >= src->rows) p2.y = src->rows-1;
    if(p1.y > p2.y) p2.y = p1.y+1;
    if(p1.x > p2.x) p2.x = p1.x+1;

    Rect roi(p1, p2);

    Mat* cropped = new Mat(*src, roi);
    return cropped;
}





// *** MultiTrack Functions ***

MultiTrack::MultiTrack(string filename, string winname, Scalar* show_color)
{
    if(filename == "webcam")
    {
        this->cap.open(0);
        this->frames = LIM_FRAMES;
        this->fps = WCAM_FPS;
        this->live_feed = true;
    }
    else
    {
        this->cap.open(filename);
        this->frames = this->cap.get(CV_CAP_PROP_FRAME_COUNT);
        this->fps = this->cap.get(CV_CAP_PROP_FPS);
        this->live_feed = false;
    }

    cout << "FPS: " << this->fps << endl;
    cout << "Period: " << 1./this->fps << endl;

    this->winname = winname;
    this->show_color = show_color;

    // Set first image
    this->cap >> this->image;
    namedWindow(winname,0);
    resizeWindow(winname, WIN_X, WIN_Y);
    imshow(winname, this->image);

    this->currentFrame = 1;
    this->currentTime = 0;
    this->offsetTime = -1;
}

MultiTrack::~MultiTrack()
{
    destroyWindow(this->winname);
    this->cap.release();
    for(uINT i = 0; i < this->p_rects.size(); i++)
    {
        delete(this->p_rects.at(i));
    }
}

double MultiTrack::start()
{
    cout << "Successfully running..." << this->winname << endl;
    waitKey(START_PAUSE);

    return this->fps;
}

bool MultiTrack::advance()
{
    int temp_frame;

    if(this->live_feed)
    {
        this->currentFrame++;
        this->currentTime += 1000./this->fps;
    }
    else
    {
        temp_frame = this->cap.get(CV_CAP_PROP_POS_FRAMES);

        if(temp_frame >= this->frames)
            return false;
        else
        {
            this->currentFrame = temp_frame;
            this->currentTime = this->cap.get(CV_CAP_PROP_POS_MSEC);
            if(this->offsetTime >= 0)
                this->currentTime += this->offsetTime;
        }
    }

    this->cap >> this->image;
    this->trackAll();
    this->drawRectangles();

    double t_scale = pow(.1, TIME_DISPLAY_DIGS);
    putText(image, toStr(floor((this->currentTime/1000)/t_scale)*t_scale), Point(0,TIME_DISPLAY_Y), FONT_HERSHEY_PLAIN, 1.5, Scalar(0,255,0), 2);

    this->showColor();

    imshow(this->winname, this->image);

    return true;
}

MultiTrack* MultiTrack::getAddress()
{
    return this;
}

Mat* MultiTrack::getImageAddress()
{
    return &(this->image);
}

void MultiTrack::addRectangle(struct Rectangle* p_rect)
{
    Mat mat_hsv;
    Vec3b temp;
    Point3f center;
    center.x = p_rect->center.x;
    center.y = p_rect->center.y;
    center.z = this->currentTime;

    cvtColor(this->image, mat_hsv, CV_BGR2HSV);
    temp = mat_hsv.at<Vec3b>(center.y, center.x);
    p_rect->color_hsv = (Scalar) temp;

    p_rect->logging = true;
    p_rect->start_frame = this->currentFrame;
    p_rect->log.push_back(center);
    p_rect->lastWeight = 1;

    this->p_rects.push_back(p_rect);

    cout << "Position:" << endl;
    cout << "\tx: " << center.x << endl;
    cout << "\ty: " << center.y << endl << endl;

    cout << "RGB:" << endl;
    cout << "\tR:" << p_rect->color[2] << endl;
    cout << "\tG:" << p_rect->color[1] << endl;
    cout << "\tB:" << p_rect->color[0] << endl << endl;

    cout << "HSV:" << endl;
    cout << "\tH:" << p_rect->color_hsv[0] << endl;
    cout << "\tS:" << p_rect->color_hsv[1] << endl;
    cout << "\tV:" << p_rect->color_hsv[2] << "\n\n\n";
}

void MultiTrack::drawRectangle(int index)
{
    struct Rectangle* p_rect = this->p_rects.at(index);

    Point p1 = p_rect->center + .5*(p_rect->width*Point(1,0) + p_rect->height*Point(0,1));
    Point p2 = p_rect->center - .5*(p_rect->width*Point(1,0) + p_rect->height*Point(0,1));

    rectangle(this->image, p1, p2, p_rect->color, RECT_THICK);
}

void MultiTrack::drawRectangles()
{
    for(uINT i = 0; i < this->p_rects.size(); i++)
    {
        if(p_rects.at(i)->logging)
            this->drawRectangle(i);
    }

    imshow(this->winname, this->image);
}

void MultiTrack::clearRectangles()
{
    for(uINT i = 0; i < this->p_rects.size(); i++)
        destroyWindow(this->winname + ": thresh " + toStr(i));
    this->p_rects.clear();
}

void MultiTrack::track(int index, bool logflag)
{
    // Crop image

    struct Rectangle* p_rect = this->p_rects.at(index);

    Mat* cropped = crop(this->getImageAddress(),p_rect);

    // Generate lower and upper thresholds

    Scalar hsv = p_rect->color_hsv;

    Scalar low_bounds[2];
    Scalar high_bounds[2];

    int S_low, S_high;
    int V_low, V_high;

    genHSVbounds(hsv[0],S_low,S_high,V_low,V_high);

    bool red = (hsv[0] > H_MAX - H_VAR || hsv[0] < H_VAR) ? true : false;

    if(!red)
    {
        low_bounds[0] = Scalar(hsv[0] - H_VAR, S_low, V_low);
        high_bounds[0] = Scalar(hsv[0] + H_VAR, S_high, V_high);
    }
    else
    {
        // high H partition

        low_bounds[0] = Scalar(hsv[0] - H_VAR + (hsv[0] < H_VAR ? H_MAX:0), S_low, V_low);
        high_bounds[0] = Scalar(H_MAX - 1 ,S_high,V_high);

        // low H partition

        low_bounds[1] = Scalar(0, S_low, V_low);
        high_bounds[1] = Scalar(hsv[0] + H_VAR - (hsv[0] < H_VAR ? 0:H_MAX) ,S_high,V_high);
    }

    // Thresh image

    Mat hsv_image;

    cvtColor(*cropped, hsv_image, CV_BGR2HSV);

    delete(cropped);
    cropped = 0;

    Mat threshed;

    if(!red)
        inRange(hsv_image, low_bounds[0], high_bounds[0], threshed);
    else
    {
        Mat temp1, temp2;
        inRange(hsv_image, low_bounds[0], high_bounds[0], temp1);
        inRange(hsv_image, low_bounds[1], high_bounds[1], temp2);
        bitwise_or(temp1, temp2, threshed);
    }

    string winName = this->winname + ": thresh " + toStr(index);

    namedWindow(winName, 0);
    resizeWindow(winName, THRESHWIN_X, THRESHWIN_Y);

    // calculate centroid

    Moments mnts = moments(threshed);

    Point2f centroid;

    float cent_x;
    float cent_y;

//    cout << "Moments:" << endl;
//    cout << "m10: " << mnts.m10 << endl;
//    cout << "m01: " << mnts.m01 << endl;
//    cout << "m00: " << mnts.m00 << endl;

    if(mnts.m00 < 1)
    {
        p_rect->logging = false;
        destroyWindow(winName);
    }
    else
    {
        cent_x = double(mnts.m10)/mnts.m00;
        cent_y = double(mnts.m01)/mnts.m00;

//        cout << "cx: " << cent_x << endl;
//        cout << "cy: " << cent_y << "\n\n\n";

        centroid.x = cent_x;
        centroid.y = cent_y;

        Point2f p1 = p_rect->center - .5*(p_rect->width*Point(1,0) + p_rect->height*Point(0,1));
        p_rect->center = centroid + p1;

        if(logflag)
        {
            Point3f temp_p;
            temp_p.x = cent_x + p1.x;
            temp_p.y = cent_y + p1.y;
            temp_p.z = this->currentTime;

            p_rect->log.push_back(temp_p);

            string writeN = toStr(index);
            string writeX = "x: " + toStr(cent_x + p1.x);
            string writeY = "y: " + toStr(cent_y + p1.y);

            Point N_org = p1 + Point2f(0,-5);
            //Point X_org = p_rect->center + .5*(p_rect->width*Point(1,0)- p_rect->height*Point(0,1));
            Point Y_org = p_rect->center + .5*(p_rect->width*Point(1,0)+ p_rect->height*Point(0,1));
            Point X_org = Y_org + Point(0, -30);

            putText(image, writeN, N_org, FONT_HERSHEY_PLAIN, 1.5, Scalar(0,255,0), 2);
            putText(image, writeX, X_org, FONT_HERSHEY_PLAIN, 1.5, Scalar(0,255,0), 2);
            putText(image, writeY, Y_org, FONT_HERSHEY_PLAIN, 1.5, Scalar(0,255,0), 2);

            p_rect->lastWeight = mnts.m00;

            imshow(winName, threshed);
            //drawCross(this->image, p_rect->center, p_rect->color);
        }
    }
}

void MultiTrack::trackAll()
{
    for(uINT i = 0; i < this->p_rects.size(); i++)
    {
        if(p_rects.at(i)->logging)
        {
            this->track(i, false);
            this->track(i, true);
        }
    }
}

void MultiTrack::setOffset(double offset)
{
    this->offsetTime = offset;
}

void MultiTrack::exportAll(string filename)
{
    Mat points;
    struct Rectangle* p_rect;
    bool isOffset;

    FileStorage fs(filename, FileStorage::WRITE);

    if(!fs.isOpened())
    {
        cerr << "Failed to open file...";
        return;
    }

    cout << "Destination: " << filename << endl << endl;

    string offset_flag;
    string points_name;

    cout << "Writing: " << this->winname << endl;

    for(uINT i = 0; i < this->p_rects.size(); i++)
    {
        cout << "\tRectangle: " << i << endl;

        p_rect = this->p_rects.at(i);

        offset_flag = "offset";
        isOffset = (this->offsetTime >= 0)? true:false;
        fs << offset_flag << isOffset;

        points_name = "points_" + toStr(i);
        fs << points_name << p_rect->log;
    }

    cout << "Done Writing...\n\n";
}

void MultiTrack::showColor()
{
    Point2i loc;
    string str[3];
    int baseline=0;
    Size sz;

    str[0] = "H : " + toStr((*this->show_color)[0]);
    str[1] = "S : " + toStr((*this->show_color)[1]);
    str[2] = "V : " + toStr((*this->show_color)[2]);

    loc.x = this->image.cols;

    for(int i=0; i < 3; i++)
    {
        sz = getTextSize(str[i], FONT_HERSHEY_PLAIN, 1.5, 2, &baseline);
        if(sz.width > this->image.cols-loc.x)
            loc.x = this->image.cols - sz.width;
    }

    // offset x

    loc.x -= 5;
    loc.y = 5;

    for(int i=0; i < 3; i++)
    {
        loc.y += sz.height+5;
        putText(this->image,str[i],loc, FONT_HERSHEY_PLAIN, 1.5, Scalar(255,0,0),2);
    }


}

int MultiTrack::numPoints()
{
    return this->p_rects.size();
}
