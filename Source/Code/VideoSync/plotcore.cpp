#include "plotcore.h"

PlotCore::PlotCore(string winname, int width, int height, int *l1, bool cycleSeries)
{
    this->winname = winname;

    this->dims.x = width;
    this->dims.y = height;

    this->ll = new Point2i;
    this->rr = new Point2i;

    this->ll->x = (int) width*PCG::PLOT_OFFSET;
    this->ll->y = (int) height*(1-PCG::PLOT_OFFSET);
    this->rr->x = (int) width*(1-PCG::PLOT_OFFSET);
    this->rr->y = (int) height*PCG::PLOT_OFFSET;

    this->l1 = -1;
    this->l2 = -1;

    this->p_l1=l1;

    this->maxX = 0;

    this->cycleSeries = cycleSeries;

    this->currentSeries = 0;
    this->currentPlot = 0;
}

PlotCore::~PlotCore()
{
    // Plot object dynamically allocated... delete

    for(uInt i = 0; i < this->plots.size(); i++)
    {
        delete this->plots.at(i);
    }
}


void PlotCore::addPlot()
{
    Plot *p = new struct Plot;

    p->image = Mat(this->dims.y,this->dims.x,CV_8UC3,PCG::WHITE);

    p->title = "Plot: " + PCG::toStr(this->plots.size());
    p->xlabel = "";
    p->ylabel = "";

    p->ll_act = new Point2d;
    p->rr_act = new Point2d;

    p->ll_act->x = 1;
    p->rr_act->x = -1;

    p->ll_act->y = 0;
    p->rr_act->y = 0;

    this->plots.push_back(p);
}

void PlotCore::changePlot(int plot_index, bool showflag)
{
    this->currentPlot = plot_index;

    if(this->plots.at(plot_index)->series.size() < (this->currentSeries-1))
        this->currentSeries = 0;

    // replot

    this->plot(plot_index);

    if(showflag)
        imshow(this->winname, this->plots.at(plot_index)->image);
}

void PlotCore::shiftPlot(bool right, bool showflag)
{
    int maxindex = (int) this->plots.size()-1;

    if(right && this->currentPlot == maxindex)
    {
        this->changePlot(0, showflag);
    }
    else if(!right && this->currentPlot == 0)
        this->changePlot(maxindex, showflag);
    else
        this->changePlot(this->currentPlot + (right ? 1:-1), showflag);
}

void PlotCore::changeSeries(int series_index, bool showflag)
{
    this->currentSeries = series_index;

    this->plot(this->currentPlot);

    if(showflag)
        imshow(this->winname, this->plots.at(this->currentPlot)->image);
}

void PlotCore::shiftSeries(bool right, bool showflag)
{
    int maxindex = (int) this->plots.at(this->currentPlot)->series.size() - 1;

    if(right && this->currentSeries == maxindex)
    {
        this->changeSeries(0, showflag);
    }
    else if(!right && this->currentSeries == 0)
        this->changeSeries(maxindex, showflag);
    else
        this->changeSeries(this->currentSeries + (right ? 1:-1), showflag);
}

void PlotCore::plot(int plot_index)
{
    // clear image

    this->clear(plot_index);

    // draw bounding box

    this->drawBoundBox(plot_index);

    // check for zeros: if so draw cross-axis

    this->drawAxes(plot_index);

    // draw points

    struct Plot *p = this->plots.at(plot_index);

    if(this->cycleSeries)
    {
        this->drawSeries(p, &p->series.at(this->currentSeries));
    }
    else
    {
        for (uInt i=0; i < p->series.size(); i++)
        {
            this->drawSeries(p, &(p->series.at(i)));
        }
    }

    // draw labels

    this->drawLabels(plot_index);
}

void PlotCore::hide()
{
    destroyWindow(this->winname);
}

void PlotCore::show(vector<int*> *val, int maxVal, int* maxX, int maxX_Lim)
{
    namedWindow(this->winname,0);
    resizeWindow(this->winname, this->dims.x, this->dims.y);

    if(maxVal > 0)
    {
        for(uInt i=0; i < this->plots.size(); i++)
            createTrackbar(this->trackname+PCG::toStr(i),this->winname,val->at(i),maxVal);
    }

    if(maxX_Lim > 0)
    {
        this->maxX = maxX;
        createTrackbar(this->trackname,this->winname,maxX,maxX_Lim);
    }

    setMouseCallback(this->winname, PCG::mouseCallback,(void *) this);
}

void PlotCore::update(int plot_index)
{
    struct Plot *p = this->plots.at(plot_index);

    // clear plot

    this->clear(plot_index);

    // recompute bounds

    for(uInt i=0; i < p->series.size(); i++)
    {
        checkBounds(p, &p->series.at(i));
    }

    // replot

    this->plot(plot_index);
}

void PlotCore::updateAll(bool showflag)
{
    for(uInt i=0; i < this->plots.size(); i++)
    {
        this->update(i);
    }

    // draw shared vlines

    if(this->l1 >= 0)
    {
        this->addVLine(this->l1);
    }
    if(this->l2 >= 0)
    {
        this->addVLine(this->l2);
    }

    // draw hlines

    this->drawHLines();

    if(showflag)
        imshow(this->winname, this->plots.at(this->currentPlot)->image);
}

void PlotCore::addSeries(int plot_index, vector<Point2d> *src, string title,  Scalar color, PCG::STYLE style)
{
    struct Plot *p = this->plots.at(plot_index);

    // create new Series

    struct Series temp;

    temp.color = color;
    temp.style = style;
    temp.data = src;
    temp.title = title;

    temp.ll_act.x = 1;
    temp.rr_act.x = -1;
    temp.ll_act.y = 1;
    temp.rr_act.y = -1;

    // check for boundary changes

    checkBounds(p, &temp);

    // append series to Plot

    p->series.push_back(temp);

    // replot

    plot(plot_index);
}

void PlotCore::addVLine(int x, Scalar color, int thickness)
{
    Point2i bottom, top;

    bottom.x = x;
    top.x = x;

    struct Plot* p;

    for(uInt i=0; i <this->plots.size(); i++)
    {
        p = this->plots.at(i);

        // x in plot region ?

        if(x < this->ll->x || x > this->rr->x)
            return;

        // go ahead

        bottom.y = this->ll->y;
        top.y = this->rr->y;

        line(p->image, bottom, top, color, thickness);
    }
}

void PlotCore::clear(int plot_index)
{
    this->plots.at(plot_index)->image.setTo(PCG::WHITE);
}

void PlotCore::clearAll()
{
    for(uInt i=0; i < this->plots.size(); i++)
    {
        this->clear(i);
    }
}

void PlotCore::emptyPlot(int plot_index)
{
    struct Plot *p = this->plots.at(plot_index);

    this->clear(plot_index);
    p->series.clear();

    p->ll_act = 0;
    p->rr_act = 0;

    p->title = "";
    p->xlabel = "";
    p->ylabel = "";
}

void PlotCore::emptyAllPlots()
{
    for(uInt i=0; i < this->plots.size(); i++)
    {
        this->emptyPlot(i);
    }
}

Point2d PlotCore::getPoint(int plot_index, int x, int y)
{
    struct Plot *p = this->plots.at(plot_index);

    if(x < this->ll->x) x = this->ll->x;
    else if(x > this->rr->x) x = this->rr->x;

    if(y < this->ll->y) y = this->ll->y;
    else if(y > this->rr->y) y = this->rr->y;

    int dx = x - this->ll->x;
    int dy = this->ll->y - y;

    Point2d temp;

    temp.x = p->ll_act->x + dx*p->xscale;
    temp.y = p->ll_act->y + dy*p->yscale;

    return temp;
}

void PlotCore::setLabels(string title, string xlabel, string ylabel)
{
    Plot *p;

    for(uInt i=0; i < this->plots.size(); i++)
    {
        p = this->plots.at(i);

        p->title = title;
        p->xlabel = xlabel;
        p->ylabel = ylabel;
    }
}

void PlotCore::addHLine(int *y)
{
    this->hlines.push_back(y);
}

void PlotCore::drawBoundBox(int plot_index)
{
    struct Plot *p = this->plots.at(plot_index);

    rectangle(p->image, *(this->ll), *(this->rr), PCG::BOUND_WIDTH);
}

void PlotCore::drawAxes(int plot_index)
{
    struct Plot *p = this->plots.at(plot_index);

    // plot zeros if in plot region

    Point2i p1;
    Point2i p2;

    if(p->ll_act->x < 0 && p->rr_act->x > 0) // 0 in x
    {
        p1.x = (int) -1*p->ll_act->x/p->xscale;
        p2.x = p1.x;

        p1.y = this->ll->y;
        p2.y = this->rr->y;

        drawDash(p->image, p1, p2);
    }

    if(p->ll_act->y < 0 && p->rr_act->y > 0) // 0 in y
    {

        p1.y = (int) this->ll->y + p->ll_act->y/p->yscale;

        p2.y = p1.y;

        p1.x = this->ll->x;
        p2.x = this->rr->x;

        drawDash(p->image, p1, p2);
    }
}

void PlotCore::drawLabels(int plot_index)
{
    Plot *p = this->plots.at(plot_index);

    int baseline = 0;

    // draw title

    Point2i title_p;
    string title_str = p->title;

    if(this->cycleSeries)
        title_str += " :  " + this->plots.at(this->currentPlot)->series.at(this->currentSeries).title;

    Size title_sz = getTextSize(title_str, PCG::TITLE_FONT, PCG::TITLE_SCALE,\
                                PCG::TITLE_THICK,&baseline);


    title_p.x = (this->ll->x + this->rr->x - title_sz.width)/2;
    title_p.y = (int) this->rr->y - PCG::TITLE_OFFSET;

    putText(p->image,title_str,title_p,PCG::TITLE_FONT, PCG::TITLE_SCALE, \
            PCG::TITLE_COLOR,PCG::TITLE_THICK);

    // draw x bounds

    Point2i x_p;
    string x_str;

    if(this->cycleSeries)
        x_str = PCG::toStr(this->plots.at(this->currentPlot)->series.at\
                           (this->currentSeries).ll_act.x);
    else
        x_str= PCG::toStr(p->ll_act->x);

    Size x_sz = getTextSize(x_str, PCG::LABEL_FONT, PCG::LABEL_SCALE,\
                            PCG::LABEL_THICK,&baseline);

    x_p.x = this->ll->x - x_sz.width/2;
    x_p.y = this->ll->y + x_sz.height + PCG::LABEL_OFFSET;

    putText(p->image,x_str,x_p,PCG::LABEL_FONT, PCG::LABEL_SCALE, \
            PCG::LABEL_COLOR,PCG::LABEL_THICK);

    if(this->cycleSeries)
        x_str = PCG::toStr(this->plots.at(this->currentPlot)->series.at\
                           (this->currentSeries).rr_act.x);
    else
        x_str = PCG::toStr(p->rr_act->x);

    x_sz = getTextSize(x_str, PCG::LABEL_FONT, PCG::LABEL_SCALE,\
                            PCG::LABEL_THICK,&baseline);

    x_p.x = this->rr->x - x_sz.width/2;
    x_p.y = this->ll->y + x_sz.height + PCG::LABEL_OFFSET;

    putText(p->image,x_str,x_p,PCG::LABEL_FONT, PCG::LABEL_SCALE, \
            PCG::LABEL_COLOR,PCG::LABEL_THICK);

    // draw y bounds

    Point2i y_p;
    string y_str;

    if(this->cycleSeries)
        y_str = PCG::toStr(this->plots.at(this->currentPlot)->series.at\
                           (this->currentSeries).ll_act.y);
    else
        y_str = PCG::toStr(p->ll_act->y);

    Size y_sz = getTextSize(y_str, PCG::LABEL_FONT, PCG::LABEL_SCALE,\
                            PCG::LABEL_THICK,&baseline);

    y_p.x = this->ll->x - y_sz.width - PCG::LABEL_OFFSET;
    y_p.y = this->ll->y + y_sz.height/2;

    putText(p->image,y_str,y_p,PCG::LABEL_FONT, PCG::LABEL_SCALE, \
            PCG::LABEL_COLOR,PCG::LABEL_THICK);

    if(this->cycleSeries)
        y_str = PCG::toStr(this->plots.at(this->currentPlot)->series.at\
                           (this->currentSeries).rr_act.y);
    else
        y_str = PCG::toStr(p->rr_act->y);

    y_sz = getTextSize(y_str, PCG::LABEL_FONT, PCG::LABEL_SCALE,\
                            PCG::LABEL_THICK,&baseline);

    y_p.x = this->ll->x - y_sz.width - PCG::LABEL_OFFSET;
    y_p.y = this->rr->y + y_sz.height/2;

    putText(p->image,y_str,y_p,PCG::LABEL_FONT, PCG::LABEL_SCALE, \
            PCG::LABEL_COLOR,PCG::LABEL_THICK);

    // draw x labels

    // draw y labels

}

void PlotCore::drawDash(Mat &img, Point2i p1, Point2i p2)
{
    Point2d delta = p2-p1;
    double length = sqrt(pow(delta.x,2)+pow(delta.y,2));
    double dashlength = PCG::AXIS_DASH;

    Point2d dash = (dashlength/length)*delta;

    bool draw = true;

    Point2d p3 = p1;
    Point2d p4;

    for(int i = 0; i*dashlength < length; i++)
    {
        if(draw)
        {
            if((i+1)*dashlength >= length)
                p4 = p2;
            else
                p4 = p3+dash;

            line(img, p3, p4, PCG::BLACK,PCG::DASH_WIDTH);
        }

        p3 += dash;
        draw = draw ? false:true;
    }
}

void PlotCore::drawSeries(Plot *plt, Series *src)
{
    Point2i pt;
    Point2i last;

    Point2d temp;

    Point2d ll_act;
    Point2d rr_act;

    float xscale;
    float yscale;

    if(this->cycleSeries)
    {
        ll_act = src->ll_act;
        rr_act = src->rr_act;

        xscale = src->xscale;
        yscale = src->yscale;
    }
    else
    {
        ll_act = *plt->ll_act;
        rr_act = *plt->rr_act;

        xscale = plt->xscale;
        yscale = plt->yscale;
    }

    for(uInt i=0; i < src->data->size(); i++)
    {
        temp = src->data->at(i);

        if(this->maxX != 0 && temp.x > *this->maxX) continue;

        pt.x = (int) (this->ll->x + (temp.x-ll_act.x)/xscale);


        pt.y = (int) (this->ll->y - (temp.y-ll_act.y)/yscale);

        // draw pt depending on style

        Point2i px;

        px.x = pt.x;

        px.y = (int) this->ll->y + ll_act.y/yscale - \
                PCG::LINE_WIDTH * (temp.y > 0 ? .5: -.5);

        switch(src->style)
        {
        case PCG::STEM:
            // draw circle at point
            circle(plt->image,pt,PCG::CIRC_RAD, src->color);
            // draw stem
            pt.y += PCG::CIRC_RAD + 1;
            line(plt->image, px,pt,src->color);
            break;
        case PCG::CROSS:
            break;
        case PCG::CIRC:
            circle(plt->image,pt,PCG::CIRC_RAD, src->color);
            break;
        case PCG::PLUS:
            break;
        case PCG::LINE:
            break;
        default:
            break;
        }
    }
}

void PlotCore::drawHLines()
{
    Point2i p1;
    Point2i p2;

    Plot *p;
    int line_y;

    p1.x = this->ll->x;
    p2.x = this->rr->x;

    for(uInt i=0; i < this->plots.size(); i++)
    {
        p = this->plots.at(i);

        for(uInt j=0; j < this->hlines.size(); j++)
        {
            line_y = *this->hlines.at(j);

            if(line_y <= p->ll_act->y || line_y >= p->rr_act->y)
                continue;

            p1.y = (int) (this->ll->y - (line_y-p->ll_act->y)/p->yscale);
            p2.y = p1.y;

            this->drawDash(p->image,p1,p2);
        }
    }
}


void PlotCore::checkBounds(Plot *p, struct Series *ser)
{
    vector<Point2d> *src = ser->data;

    if(src->size()<2)
        return;

    // check for boundary changes

    double xmin = src->at(0).x;
    double xmax = xmin;

    double ymin = src->at(0).y;
    double ymax = ymin;

    float x, y;

    for(vector<Point2d>::iterator iter = src->begin();
        iter != src->end(); iter++)
    {
        x = iter->x;
        y = iter->y;

        if(xmin > x) xmin = x;
        else if(xmax < x) xmax = x;

        if(ymin > y) ymin = y;
        else if(ymax < y) ymax = y;
    }

    // -- xmax, xmin, ymax, ymin now set

    if(this->maxX != 0)
    {
        if(ser->rr_act.x > *this->maxX)
            ser->rr_act.x = *this->maxX;

        xmax = xmax < *this->maxX ? xmax : *this->maxX;
    }

    if(ser->ll_act.x > ser->rr_act.x) // no assignment yet
    {
        ser->ll_act.x = xmin;
        ser->rr_act.x = xmax;
    }
    else
    {
        ser->ll_act.x = ser->ll_act.x < xmin ? ser->ll_act.x: xmin;
        ser->rr_act.x = ser->rr_act.x > xmax ? ser->rr_act.x: xmax;
    }

    if(ser->ll_act.y > ser->rr_act.y) // no assignment yet
    {
        ser->ll_act.y = ymin;
        ser->rr_act.y = ymax;
    }
    else
    {
        ser->ll_act.y = ser->ll_act.y < ymin ? ser->ll_act.y: ymin;
        ser->rr_act.y = ser->rr_act.y > ymax ? ser->rr_act.y: ymax;
    }

    // reset scaling factors

    ser->xscale = (ser->rr_act.x-ser->ll_act.x)/(this->rr->x-this->ll->x);
    ser->yscale = (ser->rr_act.y-ser->ll_act.y)/(this->ll->y-this->rr->y);


    // reset absolute bounds

    if(ser->ll_act.y < p->ll_act->y)
        p->ll_act = &ser->ll_act;
    if(ser->rr_act.y > p->rr_act->y)
        p->rr_act = &ser->rr_act;

    p->xscale = (p->rr_act->x-p->ll_act->x)/(this->rr->x-this->ll->x);
    p->yscale = (p->rr_act->y-p->ll_act->y)/(this->ll->y-this->rr->y);
}


void PCG::mouseCallback(int event, int x, int y, int flag, void *param)
{
    y = flag;
    flag = y;

    PlotCore *pc = (PlotCore *) param;

    switch(event)
    {
    case EVENT_LBUTTONDOWN:
        cout << "LBD" << endl;
        pc->l1 = x;
        *pc->p_l1 = (x-pc->ll)*pc->plots.at(pc->currentPlot)->xscale;
        break;
    case EVENT_LBUTTONUP:
        cout << "LBU" << endl;
        pc->l2 = x;
        // assert order
        if(pc->l1 > x)
        {
            pc->l2 = pc->l1;
            pc->l1 = x;
        }
        break;
    }
}
