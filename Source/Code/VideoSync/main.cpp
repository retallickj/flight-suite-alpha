#include "feed.h"
#include "plotcore.h"

#define WRITE_ARG 1
#define FEED_ARG_OFFSET 1

#define RECT_MIN 0

#define TRIGPLOT_NAME "trigger-plotting"
#define SIGPLOT_NAME "signal-plotting"
#define FFTPLOT_NAME "fft-plotting"

#define PLOT_WIDTH 800
#define PLOT_HEIGHT 600

#define PI 3.1415926535897

#define RIGHTKEY        '.'
#define LEFTKEY         ','
#define SER_RIGHTKEY    '['
#define SER_LEFTKEY     ']'
#define START_STOPKEY   's'
#define KILLKEY         'k'
#define NEXTKEY         'n'
#define WRITEKEY        'w'
#define RESETKEY        'r'
#define PLOT_TRIGKEY    '1'
#define PLOT_SIGKEY     '2'
#define PLOT_FFTKEY     '3'
#define TRIGGERKEY      't'
#define OFFSETKEY       'o'

using namespace std;
using namespace cv;

vector<string> filepaths;
vector<string> winnames;
vector<struct Feed*> feeds;
vector<bool> plays;

string write_name;

int num_feeds;
int delay_time = 33;

Point rect_start;
Point rect_end;

PlotCore *pc_trig;
PlotCore *pc_sig;
PlotCore *pc_fft;

bool triggerSelect;

vector<int*> thresh_ints;
int* max_fft = new int(FFT_MAX_DEFAULT);

// *** GENERAL FUNCTIONS ***

bool initiate(int argc, char** argv);
bool cleanup();

void writePack();

void mouseCallback(int event, int x, int y, int flag, void* param);

void computeOffsets();
float computePhaseDiff(Feed *f1, Feed *f2);
float valueAt(vector<Point2f> *src, float x);


// *** MAIN LOOP ***

int main(int argc, char** argv)
{
    bool kill, pause;
    bool showTrig, showSig, showFFT;
    char option;

    kill = false;
    pause = false;

    showTrig = false;
    showSig = false;
    showFFT = false;

    cout << "FileStorage: " << FileStorage::WRITE << endl << endl;

    if(!initiate(argc, argv))
        return -1;

    // wait for start

    while((char) waitKey(5) != 's');

    // loop body

    while(!kill)
    {
        kill = true;

        // chores

        for(int i = 0; i < num_feeds; i++)
        {
            if(plays.at(i) && !pause)
                plays.at(i) = (feeds.at(i)->advance() == 1) ? true: false;

            if(plays.at(i)) kill = false;
        }

        pc_trig->updateAll(showTrig);
        pc_sig->updateAll(showSig);
        pc_fft->updateAll(showFFT);

        option = (char) waitKey(delay_time);

        switch(option)
        {
        case START_STOPKEY:
            pause = pause ? false:true;
            triggerSelect = false;
            break;
        case KILLKEY: // Kill
            kill = true;
            break;
        case NEXTKEY: // Next Frame
            kill = true;

            for(int i = 0; i < num_feeds; i++)
            {
                if(plays.at(i))
                    plays.at(i) = (feeds.at(i)->advance() == 1) ? true: false;

                if(plays.at(i)) kill = false;
            }

            pc_trig->updateAll(showTrig);
            pc_sig->updateAll(showSig);
            pc_fft->updateAll(showFFT);

            break;
        case WRITEKEY: // Write and kill
            writePack();
            kill = true;
            break;
        case PLOT_TRIGKEY: // set-unset plotting
            if(showTrig)
            {
                pc_trig->hide();
                cout << "Disabling Trigger Plotting...\n";
                showTrig= false;
            }
            else
            {
                pc_trig->show(&thresh_ints, TRIGGER_WEIGHT_MAX);
                cout << "Enabling Trigger Plotting...\n";
                showTrig=true;
                pc_trig->updateAll(showTrig);
                pc_trig->changePlot(0,showTrig);
            }
            break;
        case PLOT_SIGKEY: // set-unset plotting
            if(showSig)
            {
                pc_sig->hide();
                cout << "Disabling Signal Plotting...\n";
                showSig= false;
            }
            else
            {
                pc_sig->show();
                cout << "Enabling Signal Plotting...\n";
                showSig=true;
                pc_sig->updateAll(showSig);
                pc_sig->changePlot(0,showSig);
            }
            break;
        case PLOT_FFTKEY: // set-unset plotting
            if(showFFT)
            {
                pc_fft->hide();
                cout << "Disabling FFT Plotting...\n";
                showFFT= false;
            }
            else
            {
                pc_fft->show(0,0,max_fft, FFT_MAX_LIM);
                cout << "Enabling FFT Plotting...\n";
                showFFT=true;
                pc_fft->updateAll(showFFT);
                pc_fft->changePlot(0, showFFT);
            }
            break;
        case RIGHTKEY:
            pc_trig->shiftPlot(true, showTrig);
            pc_sig->shiftPlot(true, showSig);
            pc_fft->shiftPlot(true, showFFT);
            break;
        case LEFTKEY:
            pc_trig->shiftPlot(false, showTrig);
            pc_sig->shiftPlot(false, showSig);
            pc_fft->shiftPlot(false, showFFT);
            break;
        case RESETKEY:
            for(uINT i=0; i < feeds.size(); i++)
            {
                feeds.at(i)->reset();
            }
            break;
        case SER_RIGHTKEY:
            pc_trig->shiftSeries(true, showTrig);
            pc_sig->shiftSeries(true, showSig);
            pc_fft->shiftSeries(true, showFFT);
            break;
        case SER_LEFTKEY:
            pc_trig->shiftSeries(false, showTrig);
            pc_sig->shiftSeries(false, showSig);
            pc_fft->shiftSeries(false, showFFT);
            break;
        case TRIGGERKEY:
            triggerSelect = true;
            pause = true;
            break;
        case OFFSETKEY:
            computeOffsets();
            break;
        default:
            break;
        }
    }

    cleanup();
    return 0;
}



// *** Function Definitions ***

bool initiate(int argc, char** argv)
{
    num_feeds = argc - (1+FEED_ARG_OFFSET);
    triggerSelect = false;

    cout << "num_feeds: " << num_feeds << endl << endl;

    if(num_feeds < 1)
        return false;

    for(int i = 0; i < argc; i++)
        cout << "argv[" << i << "]:  " << argv[i] << endl;

    // set write_folder

    write_name = argv[WRITE_ARG];

    // set PlotCore

    pc_trig = new PlotCore(TRIGPLOT_NAME, PLOT_WIDTH, PLOT_HEIGHT);
    pc_sig = new PlotCore(SIGPLOT_NAME, PLOT_WIDTH, PLOT_HEIGHT);
    pc_fft = new PlotCore(FFTPLOT_NAME, PLOT_WIDTH, PLOT_HEIGHT, true);

    int feed_index;
    int fps;

    for(int i = 0; i < num_feeds; i++)
    {
        thresh_ints.push_back(new int(TRIGGER_WEIGHT_DEFAULT));

        feed_index = 1+FEED_ARG_OFFSET+i;
        filepaths.push_back(argv[feed_index]);
        winnames.push_back(argv[feed_index]);

        feeds.push_back(new Feed(filepaths.at(i), winnames.at(i), thresh_ints.at(i)));
        fps = feeds.at(i)->start();
        setMouseCallback(winnames.at(i), mouseCallback, (void *) feeds.at(i));
        plays.push_back(true);

        pc_trig->addPlot();
        pc_sig->addPlot();
        pc_fft->addPlot();

        pc_trig->addSeries(i,feeds.at(i)->getTrigLog());
        pc_sig->addSeries(i,feeds.at(i)->getSigLog());

        pc_fft->addSeries(i,feeds.at(i)->getfft_amp(), "amplitude");
        pc_fft->addSeries(i,feeds.at(i)->getfft_arg(), "phase", PCG::BLUE);

        pc_trig->addHLine(thresh_ints.at(i));

        if(delay_time > 1000/fps)
            delay_time = 1000/fps;
    }
    return true;
}

bool cleanup()
{
    for(int i = 0; i < num_feeds; i++)
    {
        delete feeds.at(i);
        feeds.at(i) = 0;
    }
    destroyAllWindows();
    delete pc_trig;
    delete pc_sig;
    delete pc_fft;
    for(uINT i=0; i < thresh_ints.size(); i++)
        delete thresh_ints.at(i);
    delete max_fft;
    return true;
}

void writePack()
{
    FileStorage fs;

    fs.open(write_name, FileStorage::WRITE );

    if(!fs.isOpened())
    {
        cerr << "Failed to open file...";
        return;
    }

    cout << "Destination: " << write_name << endl << endl;

    string ID;

    // number of feeds

    fs << "num_feeds" << num_feeds;

    for(int i = 0; i < num_feeds; i++)
    {
        ID = "feed_" + toStr(i);
        feeds.at(i)->write(fs, ID);
    }
}

void mouseCallback(int event, int x, int y, int flag, void* param)
{
    Feed* p_feed;
    Mat* p_image;

    Point delta, p1, p2;
    Rect roi;

    int dummy = flag;
    flag = dummy;

    // check cases

    switch(event)
    {
    case EVENT_LBUTTONDOWN:
        rect_start.x = x;
        rect_start.y = y;
        break;
    case EVENT_LBUTTONUP:
        p_feed = (Feed *) param;
        p_image = p_feed->getImageAddress();

        rect_end.x = x;
        rect_end.y = y;

        delta = rect_end - rect_start;

        delta.x = abs(delta.x);
        delta.y = abs(delta.y);

        if(delta.x < RECT_MIN || delta.y < RECT_MIN) return;

        p1 = rect_start-delta;
        p2 = rect_start+delta;

        // assert bounds

        if(p1.x < 0) p1.x = 0;
        if(p2.x >= p_image->cols) p2.x = p_image->cols-1;
        if(p1.y < 0) p1.y = 0;
        if(p2.y >= p_image->rows) p2.y = p_image->rows-1;

        roi = Rect(p1, p2);

        if(triggerSelect)
        {
            p_feed->setTrigROI(roi);
            triggerSelect = false;
        }
        else
            p_feed->setSigROI(roi);

        break;

    default:
        break;
    }
}

void computeOffsets()
{
    // compute relative phases

    cout << "\nComputing Offsets...\n\n";

    vector<float> phases;

    Feed *f0, *f1;
    float T = 2*PI/F_0;

    f0 = feeds.at(0);

    for(uINT i=1; i < feeds.size(); i++)
    {
        f1 = feeds.at(i);

        phases.push_back(computePhaseDiff(f0, f1));
    }

    // compute offsets: fft taken on end of trigger so assume times
    // within a few frames

    vector<float> offsets;

    float offset;
    int N, N1;

    N = f0->getSigLog()->size();
    offsets.push_back(0); // assume first feed has no offset

    for(uINT i = 1; i < feeds.size(); i++)
    {
        f1 = feeds.at(i);
        N1 = f1->getSigLog()->size();

        offset = f0->getSigLog()->at(N-1).x - f1->getSigLog()->at(N1-1).x;
        offset -= T*phases.at(i-1)/(2*PI);

        offsets.push_back(offset);
    }

    // determine the lowest offset, set to zero

    float offset_offset = 0;

    for(uINT i = 0; i < offsets.size(); i++)
    {
        if(offsets.at(i) < -offset_offset)
            offset_offset = -offsets.at(i);
    }

    for(uINT i = 0; i < offsets.size(); i++)
    {
        cout << "Feed " + toStr(i) + " :\tPhase : " + toStr(i ? phases.at(i-1):0);
        cout << "\tOffset : " + toStr(offsets.at(i));

        offsets.at(i) += offset_offset;
        feeds.at(i)->setOffset(offsets.at(i));

        cout << "\tShifted : " + toStr(offsets.at(i)) << endl;
    }


}

float computePhaseDiff(Feed *f1, Feed *f2)
{
    // access fourier data

    vector<Point2d> *fft1 = f1->getfft_arg();
    vector<Point2d> *fft2 = f2->getfft_arg();

    // compute resonance index

    float res1, res2;
    float df1, df2;
    float ds1, ds2;

    df1 = fft1->at(1).x - fft1->at(0).x;
    df2 = fft2->at(1).x - fft2->at(0).x;

    ds1 = FFT_PHASE_BOUND/df1;
    ds2 = FFT_PHASE_BOUND/df2;

    res1 = F_0/df1;
    res2 = F_0/df2;

    // assess the points around resonance

    vector<Point2f> v_p1,  v_p2;

    for(int i = res1 - ds1; i <= res1+ds1; i++)
    {
        v_p1.push_back(fft1->at(i));
    }


    for(int i = res2 - ds2; i <= res2+ds2; i++)
    {
        v_p2.push_back(fft2->at(i));
    }

    // assert ordering by number of elements

    vector<Point2f> *p_v1, *p_v2;

    if(v_p1.size() <= v_p2.size())
    {
        p_v1 = &v_p1;
        p_v2 = &v_p2;
    }
    else
    {
        p_v1 = &v_p2;
        p_v2 = &v_p1;
    }

    // iterate through comparisons

    float ph_diff;
    float _sum=0;

    for(uINT i=0; i < p_v1->size(); i++)
    {
        ph_diff = p_v1->at(i).y-valueAt(p_v2,p_v1->at(i).x);

        if(ph_diff < -PI)
            ph_diff += PI;

        _sum += ph_diff;
    }

    float avg = _sum / p_v1->size();

    return avg;
}

float valueAt(vector<Point2f> *src, float x)
{
    int ilow=0, ihigh=0;

    for(uINT i=0; i < src->size(); i++)
    {
        if (src->at(i).x == x)
            return src->at(i).y;
        else if(src->at(i).x > x)
        {
            ihigh = i;
            break;
        }
        else
            ilow = i;
    }

    Point2f p1 = src->at(ilow);
    Point2f p2 = src->at(ihigh);

    float slope = (p2.y-p1.y)/(p2.x-p1.x);

    return p1.y + slope*(x-p1.x);
}
