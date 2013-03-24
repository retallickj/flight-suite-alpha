#include "feed.h"


Feed::Feed(string filename, string winname, int* thresh)
{
    this->cap.open(filename);
    this->frames = this->cap.get(CV_CAP_PROP_FRAME_COUNT);
    this->fps = this->cap.get(CV_CAP_PROP_FPS);
    this->filepath = filename;
    this->winname = winname;

    this->trig_plotted = false;
    this->thresh_plotted = false;

    // set first image
    this->cap >> this->image;
    namedWindow(winname,0);
    resizeWindow(winname, WIN_X, WIN_Y);
    imshow(winname, this->image);

    this->currentFrame = this->cap.get(CV_CAP_PROP_POS_FRAMES);
    this->currentTime = this->cap.get(CV_CAP_PROP_POS_MSEC);
    this->offsetTime = 0;

    this->trigRoiSet = false;
    this->sigRoiSet = false;

    this->fft_amp = new vector<Point2d>;
    this->fft_arg = new vector<Point2d>;

    this->thresh = thresh;
}

Feed::~Feed()
{
    delete this->fft_amp;
    delete this->fft_arg;
}

int Feed::start()
{
    cout << "Successfully running..." << this->winname << endl;
    waitKey(START_PAUSE);

    return this->fps;
}

int Feed::advance()
{
    int temp_frame = this->cap.get(CV_CAP_PROP_POS_FRAMES);

    if(temp_frame >= this->frames)
        return -1;

    this->currentFrame = temp_frame;
    this->currentTime = this->cap.get(CV_CAP_PROP_POS_MSEC);

    // set new image

    this->cap >> this->image;

    // calculate roi intensities

    float trig_int, sig_int;

    if(this->trigRoiSet)
    {
        string trig_string = "";
        string sig_string = "";

        trig_int = this->calcInt(true, true);

        trig_string = "T: " + toStr(trig_int);

        Point2d temp;

        temp.x = this->currentTime;
        temp.y = trig_int > TRIGGER_WEIGHT_MAX ? TRIGGER_WEIGHT_MAX: trig_int;

//        if(this->triglog.size()>L_SMOOTH_T-1)
//            temp.y = this->localSmooth(temp.y);

        this->triglog.push_back(temp);

        if(SUB_PLOT_FLAG)
        {
            resizeWindow(this->winname + " : Trig", SUB_X, SUB_Y);
            imshow(this->winname + " : Trig", this->trig_image);
        }

        if(this->sigRoiSet)
        {
            if(trig_int >= *thresh)
                this->logging = true;
            else if(this->logging) // end of signal
            {
                this->logging = false;

                if(this->siglog.size() < MIN_SIG_SIZE)
                    this->siglog.clear();

                return 1;
            }

            if(this->logging)
            {
                sig_int = this->calcInt(false,  true);

                temp.y = sig_int;

                // write to log

                this->siglog.push_back(temp);
                sig_string = "S: " + toStr(sig_int);

                // update fft

                this->fft(this->fft_amp, this->fft_arg);

                if(SUB_PLOT_FLAG)
                {
                    resizeWindow(this->winname + " : Thresh", SUB_X, SUB_Y);
                    imshow(this->winname + " : Thresh",this->thresh_image);
                }
            }
            else
                sig_string = "S: ---";
        }

        // write intensities to screen

        putText(image,trig_string, Point(0,this->image.rows - TRIG_DISPLAY_OFFSET), \
                FONT_HERSHEY_PLAIN, 1.5, Scalar(255,0,0), 2);
        putText(image,sig_string, Point(0,this->image.rows - SIG_DISPLAY_OFFSET), \
                FONT_HERSHEY_PLAIN, 1.5, Scalar(255,0,0), 2);

        // draw rectangle around roi

        rectangle(this->image, this->trig_roi ,Scalar(0,0,255),R_THICK);
        rectangle(this->image, this->sig_roi ,Scalar(0,255,0),R_THICK);
    }

    // display current time

    double t_scale = pow(.1, TIME_DISPLAY_DIGS);
    putText(image,toStr(floor((this->currentTime/1000)/t_scale)*t_scale), \
            Point(0,TIME_DISPLAY_Y), FONT_HERSHEY_PLAIN, 1.5, Scalar(0,255,0), 2);

    imshow(this->winname, this->image);

    return 1;
}

void Feed::setTrigROI(Rect &roi)
{
    cout << "\n\nsetting trig roi ...";
    this->trigRoiSet = true;
    this->trig_roi = roi;

    if(!this->trig_plotted && SUB_PLOT_FLAG)
    {
        this->trig_plotted = true;
        namedWindow(this->winname + " : Trig", 0);
        resizeWindow(this->winname + " : Trig", SUB_X, SUB_Y);
    }

    cout << "added";
}

void Feed::setSigROI(Rect &roi)
{
    this->sigRoiSet = true;
    this->sig_roi = roi;

    if(!this->thresh_plotted && SUB_PLOT_FLAG)
    {
        this->thresh_plotted = true;
        namedWindow(this->winname + " : Thresh", 0);
        resizeWindow(this->winname + " : Thresh", SUB_X, SUB_Y);
    }
}

void Feed::reset()
{
    this->cap.set(CV_CAP_PROP_POS_FRAMES,0);

    this->currentFrame = 0;
    this->currentTime = 0;
    this->offsetTime = 0;

    this->start();
    this->advance();

    this->triglog.clear();
    this->siglog.clear();

    this->fft_amp->clear();
    this->fft_arg->clear();

    this->logging = false;

}

void Feed::write(FileStorage fs, string ID)
{
    string filepath_name = ID + "_filepath";
    string offset_name = ID + "_offset";

    fs << filepath_name << this->filepath;
    fs << offset_name << this->offsetTime;
}

Mat* Feed::getImageAddress()
{
    return &(this->image);
}

vector<Point2d>* Feed::getTrigLog()
{
    return &(this->triglog);
}

vector<Point2d>* Feed::getSigLog()
{
    return &(this->siglog);
}

vector<Point2d>* Feed::getfft_amp()
{
    return this->fft_amp;
}

vector<Point2d>* Feed::getfft_arg()
{
    return this->fft_arg;
}


void Feed::fft(vector<Point2d> *amp, vector<Point2d> *angle)
{
    fftw_complex *in, *out;
    fftw_plan p;

    uINT N = this->siglog.size();

    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    p = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // initialize input

    for(uINT i = 0; i < N; i++)
    {
        in[i][0] = this->siglog.at(i).y;
        in[i][1] = 0;
    }

    // execute fft

    fftw_execute(p); // out now loaded with fft coefs (2D length N array)


    // format output

    // -- clear

    amp->clear();
    angle->clear();

    // -- port out to dest

    Point2d amp_p;
    Point2d arg_p;

    complex<double> coef;

    float freq_scale = this->fps/N;

    for(uINT i = 0; i < N/2; i++)
    {
        coef = complex<double>(out[i][0], out[i][1]);

        amp_p.x = (float) i*freq_scale; // freq: scale by fps/N
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

        amp->push_back(amp_p);
        angle->push_back(arg_p);
    }
}



// *** Private Methods ***

float Feed::calcInt(bool trig, bool display)
{
    // create new Mat

    Rect roi = trig ? this->trig_roi: this->sig_roi;

    Mat crop(this->image, roi);

    // create thresholded image

    Mat grey;

    cvtColor(crop, grey, CV_BGR2GRAY);

    Moments mnts = moments(grey);

    // display threshed image

    if(display)
    {
        if(trig)
            this->trig_image = grey;
        else
            this->thresh_image = grey;
    }

    return mnts.m00;
}

float Feed::localSmooth(float nextVal)
{
    float _sum = 0;
    uINT N = this->triglog.size();

    for(uINT i = N+1-L_SMOOTH_T; i < N; i++)
    {
        _sum += this->triglog.at(i).y;
    }

    _sum += nextVal;

    return _sum/L_SMOOTH_T;
}

void Feed::setOffset(double offset)
{
    this->offsetTime = offset;
}

double Feed::getOffset()
{
    return this->offsetTime;
}
