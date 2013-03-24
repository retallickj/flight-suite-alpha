#include "multitrack.h"
#include <sstream>

#define RECT_WIDTH 30
#define RECT_HEIGHT 30

#define RECT_MIN 20

#define PACK_ARG 1
#define WRITE_ARG 2
#define FEED_ARG_OFFSET 2

#define MEAN_WIDTH 5
#define MEAN_HEIGHT 5

const string EXPORT_ROOT = "../exports/";

using namespace cv;
using namespace std;

// GLOBALS

int num_feeds;

vector<string> filenames;
vector<string> winnames;
vector<double> offsets;
vector<bool> plays;
vector<MultiTrack*> mts;
vector<Scalar*> hsvs;

Point rect_start;
Point rect_end;

string write_folder;
bool fromPack;

int run_fps = -1;


// *** Function Declarations ***

void onMouseDown(int event, int x, int y, int flag, void* param);
bool initiate(int argc, char** argv);
bool cleanup();
void extractVPack(string readpath);
Scalar getColor(Mat* image, Point center) ;



// *** MAIN BODY ***

int main(int argc, char** argv)
{
    bool kill, pause;
    char c;
    string savename;

    kill = false;
    pause = false;

    if(!initiate(argc, argv)) return 0;

    while((char)waitKey(5) != 's');

    while(!kill)
    {
        kill = true;

        for(int i = 0; i < num_feeds; i++)
        {
            if(plays.at(i) && !pause)
                plays.at(i) = mts.at(i)->advance();

            if(plays.at(i)) kill = false;
        }

        c = (char) waitKey(1000/run_fps);

        switch(c)
        {
        case 'k':
            kill = true;
            break;
        case 's':
            pause = pause? false:true;
            break;
        case 'n':
            kill = true;

            cout << "Next frame...\n";

            for(int i = 0; i < num_feeds; i++)
            {
                if(plays.at(i))
                    plays.at(i) = mts.at(i)->advance();

                if(plays.at(i)) kill = false;
            }
            break;
        case 'c':
            for(int i = 0; i < num_feeds; i++)
                mts.at(i)->clearRectangles();
            break;
        case 'w':
            // write rectangles to file

            cout << "Writing data...\n\n";

            for(int i = 0; i < num_feeds; i++)
            {
                savename = write_folder + "/feed" + toStr(i) + ".xml";
                mts.at(i)->exportAll(savename);
            }
            break;
        default:
            break;
        }
    }

    cleanup();
    return 0;
}




// *** Function Definitions ***


void onMouseDown(int event, int x, int y, int flag, void* param)
{
    MultiTrack* p_mt;
    Mat* p_image;
    struct Rectangle* p_rect;
    Scalar color;
    Scalar color_hsv;
    Mat mat_hsv;

    // kill annoying -Wunused-parameter warning

    int dummy = flag;
    flag = dummy;

    // check cases

    switch(event)
    {
    case EVENT_RBUTTONDOWN:

        p_mt = (MultiTrack *) param;
        p_image = p_mt->getImageAddress();

        cout << "x: " << x << endl;
        cout << "y: " << y << endl << endl;

        cvtColor(*p_image, mat_hsv, CV_BGR2HSV);

        color = (Scalar) p_image->at<Vec3b>(y,x);
        color_hsv = (Scalar) mat_hsv.at<Vec3b>(y,x);

        cout << "Position:" << endl;
        cout << "\tx: " << x << endl;
        cout << "\ty: " << y << endl << endl;

        cout << "RGB:" << endl;
        cout << "\tR:" << color[2] << endl;
        cout << "\tG:" << color[1] << endl;
        cout << "\tB:" << color[0] << endl << endl;

        cout << "HSV:" << endl;
        cout << "\tH:" << color_hsv[0] << endl;
        cout << "\tS:" << color_hsv[1] << endl;
        cout << "\tV:" << color_hsv[2] << "\n\n\n";

        break;

    case EVENT_LBUTTONDOWN:

        rect_start.x = x;
        rect_start.y = y;

        break;

    case EVENT_LBUTTONUP:

        p_mt = (MultiTrack *) param;
        p_image = p_mt->getImageAddress();

        rect_end.x = x;
        rect_end.y = y;

        // create new rectangle

        p_rect = new struct Rectangle;

        p_rect->center = rect_start;
        p_rect->width = 2*abs(rect_end.x-rect_start.x);
        p_rect->height = 2*abs(rect_end.y-rect_start.y);

        // extract color from region

        //color = (Scalar) p_image->at<Vec3b>(rect_start.y, rect_start.x);
        color = getColor(p_image, rect_start);

        if( p_rect->width < RECT_MIN || p_rect->height < RECT_MIN)
        {
            delete p_rect;
            p_rect =0;
            return;
        }

        p_rect->color = color;

        p_mt->addRectangle(p_rect);
        p_mt->drawRectangles();

        break;
    case EVENT_MOUSEMOVE:

        p_mt = (MultiTrack *) param;
        p_image = p_mt->getImageAddress();

        cvtColor(*p_image, mat_hsv, CV_BGR2HSV);

        color = (Scalar) p_image->at<Vec3b>(y,x);
        color_hsv = (Scalar) mat_hsv.at<Vec3b>(y,x);

        // display text

        *(p_mt->show_color) = color_hsv;

        break;
    default:
        return;
    }
}



bool initiate(int argc, char** argv)
{
    num_feeds = argc-(1+FEED_ARG_OFFSET);

    if(num_feeds < 1)
        return false;

    for(int i = 0; i < argc; i++)
        cout << "argv[" << i << "]:  " << argv[i] << endl;

    // video pack or video feeds

    if(strcmp(argv[1],"1") == 0)  // Video pack
    {
        // extract filenames, and offsets
        extractVPack(argv[PACK_ARG]);
        fromPack = true;
    }
    else
        fromPack = false;

    // set write_folder

    write_folder = argv[WRITE_ARG];

    uINT feed_index;
    int fps;

    for(int i = 0; i < num_feeds; i++)
    {
        feed_index = 1 + FEED_ARG_OFFSET + i;
        hsvs.push_back(new Scalar(0,0,0));

        if(!fromPack)
        {
            filenames.push_back(argv[feed_index]);
        }

        winnames.push_back("Feed-"+toStr(i));
        mts.push_back(new MultiTrack(filenames.at(i), winnames.at(i), hsvs.at(i)));
        fps = mts.at(i)->start();
        setMouseCallback(winnames.at(i), onMouseDown, (void *) mts.at(i));
        plays.push_back(true);

        if(run_fps < fps)
            run_fps = fps;

        if(fromPack)
        {
            mts.at(i)->setOffset(offsets.at(i));
        }
    }

    return true;
}

bool cleanup()
{
    for(int i = 0; i < num_feeds; i++)
    {
        delete mts.at(i);
        mts.at(i) = 0;
    }

    //destroyAllWindows();
    return true;
}

void extractVPack(string readpath)
{
    FileStorage fs(readpath, FileStorage::READ);

    if(!fs.isOpened())
    {
        cerr << "Failed to open file...";
        return;
    }

    fs["num_feeds"] >> num_feeds;

    string filepath;
    double offset;

    for(int i = 0; i < num_feeds; i++)
    {
        fs["feed_"+toStr(i)+"_filepath"] >> filepath;
        fs["feed_"+toStr(i)+"_offset"] >> offset;

        filenames.push_back(filepath);
        offsets.push_back(offset);
    }
}

Scalar getColor(Mat *image, Point center)
{
    Scalar color;

    // create new Rectangle for cropping:
    //          needs center, width, height for cropping algorithm

    struct Rectangle temp;
    temp.center = center;
    temp.width = MEAN_WIDTH;
    temp.height = MEAN_HEIGHT;

    Mat* cropped = crop(image, &temp);

    color = mean(*cropped);

    delete(cropped);
    cropped = 0;

    return color;
}
