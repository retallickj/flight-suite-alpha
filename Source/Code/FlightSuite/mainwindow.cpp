#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // configure settings

    settings = new QSettings(SETTINGS_PATH,QSettings::IniFormat,this);

    // default gui displays

    this->setDefaults();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete settings;
}

void MainWindow::getFile(QLineEdit* line, QString dir)
{
    QString fname;

    if(dir.isEmpty())
        fname =  QFileDialog::getOpenFileName(this, SELECT_FILE_STR);
    else
        fname =  QFileDialog::getOpenFileName(this, SELECT_FILE_STR, dir);

    line->clear();
    line->insert(fname);
}

void MainWindow::getDir(QLineEdit* line, QString dir)
{

    QString dname;

    if(dir.isEmpty())
        dname =  QFileDialog::getExistingDirectory(this, SELECT_DIR_STR);
    else
        dname =  QFileDialog::getExistingDirectory(this, SELECT_DIR_STR, dir);

    line->clear();
    line->insert(dname);
}

void MainWindow::getFiles(QLineEdit* line, QString dir)
{
    QStringList list;

    if(dir.isEmpty())
        list =  QFileDialog::getOpenFileNames(this, SELECT_FILES_STR);
    else
        list =  QFileDialog::getOpenFileNames(this, SELECT_FILES_STR, dir);

    line->clear();
    line->insert(list.join("<br/>"));
}

void MainWindow::addFiles(QTextEdit *text, QLineEdit *line)
{
    QString input = line->text();

    if(!input.isEmpty())
    {
        // check formating

        // set newline
        input.replace(";", "<br/>");

        // add to text list

        text->append(input);

        // clear line

        line->clear();
    }
}


void MainWindow::setDefaults()
{
    // executable paths

    ui->vsync_exec_line->insert(settings->value("exec/vsync").toString());
    ui->camcal_exec_line->insert(settings->value("exec/camcal").toString());
    ui->track_exec_line->insert(settings->value("exec/track").toString());
    ui->triang_exec_line->insert(settings->value("exec/triang").toString());
    ui->proc_exec_line->insert(settings->value("exec/proc").toString());
    ui->asis_exec_line->insert(settings->value("exec/asis").toString());
    ui->plot_exec_line->insert(settings->value("exec/plot").toString());

    // directory paths

    ui->video_path_line->insert(settings->value("path/video").toString());
    ui->videopack_path_line->insert(settings->value("path/videopack").toString());
    ui->cammat_path_line->insert(settings->value("path/cammat").toString());
    ui->trackpack_path_line->insert(settings->value("path/trackpack").toString());
    ui->triangpack_path_line->insert(settings->value("path/triangpack").toString());
    ui->statepack_path_line->insert(settings->value("path/statepack").toString());

    ui->vsync_line_freq->insert(settings->value("vsync/freq").toString());
}




// Video Synchronization





void MainWindow::on_vsync_pb_add_clicked()
{
    addFiles(ui->vsync_text, ui->vsync_line_in);
}

void MainWindow::on_vsync_line_in_returnPressed()
{
    on_vsync_pb_add_clicked();
}

void MainWindow::on_vsync_pb_in_clicked()
{
    getFiles(ui->vsync_line_in, ui->video_path_line->text());
}

void MainWindow::on_vsync_pb_out_clicked()
{
    getFile(ui->vsync_line_out, ui->videopack_path_line->text());
}

void MainWindow::on_vsync_line_freq_editingFinished()
{
    if(ui->vsync_cb_freq->checkState())
    {
        settings->setValue("vsync/freq", ui->vsync_line_freq->text());
    }
}

void MainWindow::on_vsync_cb_freq_clicked()
{
    if(ui->vsync_cb_freq->checkState())
    {
        this->on_vsync_exec_line_editingFinished();
    }
}

void MainWindow::on_vsync_pb_run_clicked()
{
    QString path_string = ui->vsync_text->toPlainText();

    if(path_string.isEmpty())
        return;

    QString exec_path = settings->value("exec/vsync").toString();
    QStringList args;

    int status;

    QString write_path = ui->vsync_line_out->text();

    QString freq = ui->vsync_line_freq->text();
    if(freq.isEmpty())
        return;

    args.append(write_path);
    args.append(freq);
    args.append(path_string.split("\n"));
    status = qproc->execute(exec_path, args);

    std::cout << status << std::endl;

    //ui->vsync_text->clear();
    //ui->vsync_text->append(exec_path);
}





// Camera Calibration





void MainWindow::on_camcal_clb_prev_clicked()
{
    ui->camcal_stack->setCurrentIndex(0);
}

void MainWindow::on_camcal_clb_next_clicked()
{
    ui->camcal_stack->setCurrentIndex(1);
}

void MainWindow::on_camcal_pb_v1in_clicked()
{
    getFiles(ui->camcal_line_v1in, ui->video_path_line->text());
}

void MainWindow::on_camcal_pb_v2in_clicked()
{
    getFiles(ui->camcal_line_v2in, ui->video_path_line->text());
}

void MainWindow::on_camcal_pb_vpin_clicked()
{
    getFiles(ui->camcal_line_vpin, ui->videopack_path_line->text());
}

void MainWindow::on_camcal_pb_out_clicked()
{
    getDir(ui->camcal_line_out, ui->cammat_path_line->text());
}

void MainWindow::on_camcal_pb_v1add_clicked()
{
    addFiles(ui->camcal_v1text, ui->camcal_line_v1in);
}

void MainWindow::on_camcal_pb_v2add_clicked()
{
    addFiles(ui->camcal_v2text, ui->camcal_line_v2in);
}

void MainWindow::on_camcal_pb_vpadd_clicked()
{
    addFiles(ui->camcal_vptext, ui->camcal_line_vpin);
}


void MainWindow::on_camcal_pb_run_clicked()
{
    // run Camera Calibration code

    QString exec_path = settings->value("exec/camcal").toString();
    QStringList args;

    if(ui->camcal_vptext->toPlainText().isEmpty())
        return;

    QStringList sl1 = ui->camcal_v1text->toPlainText().split("\n");
    QStringList sl2 = ui->camcal_v2text->toPlainText().split("\n");
    QStringList sl3 = ui->camcal_vptext->toPlainText().split("\n");

    args.append(QString::number(sl1.size()));
    args.append(QString::number(sl2.size()));
    args.append(QString::number(sl3.size()));

    args.append(sl1);
    args.append(sl2);
    args.append(sl3);

    int status = qproc->execute(exec_path,args);

    std::cout << "Execution Status: " << status << std::endl;
}






// Blob Tracking






void MainWindow::on_track_pb_vpin_clicked()
{
    getFile(ui->track_line_vpin, ui->videopack_path_line->text());
}

void MainWindow::on_track_pb_add_clicked()
{
    addFiles(ui->track_text, ui->track_line_vin);
}

void MainWindow::on_track_pb_vin_clicked()
{
    getFiles(ui->track_line_vin, ui->video_path_line->text());
}

void MainWindow::on_track_line_vin_returnPressed()
{
    on_track_pb_add_clicked();
}

void MainWindow::on_track_pb_out_clicked()
{
    getDir(ui->track_line_out, ui->trackpack_path_line->text());
}

void MainWindow::on_track_pb_run_clicked()
{
    QString exec_path = settings->value("exec/track").toString();
    QStringList args;

    int status;

    QString write_path = ui->track_line_out->text();

    if(ui->track_ckbx->isChecked())
    {
        args.append("1");
        args.append(write_path);

        args.append((QStringList) ui->track_line_vpin->text());
        status = qproc->execute(exec_path, args);
    }
    else
    {
        args.append("0");
        args.append(write_path);
        args.append(ui->track_text->toPlainText().split("\n"));
        status = qproc->execute(exec_path, args);
    }

    std::cout << status << std::endl;
}





// Triangulation





void MainWindow::on_triang_pb_tpin_clicked()
{
    getFile(ui->triang_line_tpin, ui->trackpack_path_line->text());
}

void MainWindow::on_traing_pb_cmin_clicked()
{
    getFile(ui->triang_line_cmin, ui->cammat_path_line->text());
}

void MainWindow::on_triang_pb_out_clicked()
{
    getFile(ui->triang_line_out, ui->triangpack_path_line->text());
}

void MainWindow::on_triang_pb_run_clicked()
{
    // run Triangulation code

    QString exec_path = settings->value("exec/triang").toString();
    QStringList args;

    QString write_path = ui->triang_line_out->text();

    if(write_path.isEmpty())
        write_path = "./output.xml";


}




// Processing






void MainWindow::on_proc_pb_trin_clicked()
{
    getFile(ui->proc_line_trin, ui->triangpack_path_line->text());
}

void MainWindow::on_proc_pb_out_clicked()
{
    getFile(ui->proc_line_out, ui->statepack_path_line->text());
}

void MainWindow::on_proc_pb_run_clicked()
{
    // Run Processing code

    QString exec_path = settings->value("exec/proc").toString();
    QStringList args;

    if(ui->proc_line_trin->text().isEmpty())
        return;

    QString write_path;

    if(ui->proc_line_out->text().isEmpty())
        write_path = "./output.xml";
    else
        write_path = ui->proc_line_out->text();


    args.append(write_path);
    args.append(ui->proc_line_trin->text());

    int status = qproc->execute(exec_path,args);

    std::cout << "Execution Status: " << status << std::endl;


}




// Analysis












// Settings tab




void MainWindow::on_vsync_exec_pb_clicked()
{
    getFile(ui->vsync_exec_line, EXEC_DIR_PATH);
    this->on_vsync_exec_line_editingFinished();
}

void MainWindow::on_camcal_exec_pb_clicked()
{
    getFile(ui->camcal_exec_line, EXEC_DIR_PATH);
    this->on_camcal_exec_line_editingFinished();
}

void MainWindow::on_track_exec_pb_clicked()
{
    getFile(ui->track_exec_line, EXEC_DIR_PATH);
    this->on_track_exec_line_editingFinished();
}

void MainWindow::on_triang_exec_pb_clicked()
{
    getFile(ui->triang_exec_line, EXEC_DIR_PATH);
    this->on_triang_exec_line_editingFinished();
}

void MainWindow::on_proc_exec_pb_clicked()
{
    getFile(ui->proc_exec_line, EXEC_DIR_PATH);
    this->on_proc_exec_line_editingFinished();
}

void MainWindow::on_asis_exec_pb_clicked()
{
    getFile(ui->asis_exec_line, EXEC_DIR_PATH);
    this->on_asis_exec_line_editingFinished();
}

void MainWindow::on_plot_exec_pb_clicked()
{
    getFile(ui->plot_exec_line, EXEC_DIR_PATH);
    this->on_plot_exec_line_editingFinished();
}


void MainWindow::on_vsync_exec_line_editingFinished()
{
    settings->setValue("exec/vsync",ui->vsync_exec_line->text());
}

void MainWindow::on_camcal_exec_line_editingFinished()
{
    settings->setValue("exec/camcal",ui->camcal_exec_line->text());
}

void MainWindow::on_track_exec_line_editingFinished()
{
    settings->setValue("exec/track",ui->track_exec_line->text());
}

void MainWindow::on_triang_exec_line_editingFinished()
{
    settings->setValue("exec/triang",ui->triang_exec_line->text());
}

void MainWindow::on_proc_exec_line_editingFinished()
{
    settings->setValue("exec/proc",ui->proc_exec_line->text());
}

void MainWindow::on_asis_exec_line_editingFinished()
{
    settings->setValue("exec/asis",ui->asis_exec_line->text());
}

void MainWindow::on_plot_exec_line_editingFinished()
{
    settings->setValue("exec/plot",ui->plot_exec_line->text());
}






void MainWindow::on_settings_clb_1_clicked()
{
    ui->settings_stack->setCurrentIndex(1);
}

void MainWindow::on_settings_clb_2_clicked()
{
    ui->settings_stack->setCurrentIndex(0);
}




void MainWindow::on_video_path_pb_clicked()
{
    getDir(ui->video_path_line, DATA_DIR_PATH);
    this->on_video_path_line_editingFinished();
}

void MainWindow::on_videopack_path_pb_clicked()
{
    getDir(ui->videopack_path_line, DATA_DIR_PATH);
    this->on_videopack_path_line_editingFinished();
}

void MainWindow::on_cammat_path_pb_clicked()
{
    getDir(ui->cammat_path_line, DATA_DIR_PATH);
    this->on_cammat_path_line_editingFinished();
}

void MainWindow::on_trackpack_path_pb_clicked()
{
    getDir(ui->trackpack_path_line, DATA_DIR_PATH);
    this->on_trackpack_path_line_editingFinished();
}

void MainWindow::on_triangpack_path_pb_clicked()
{
    getDir(ui->triangpack_path_line, DATA_DIR_PATH);
    this->on_triangpack_path_line_editingFinished();
}

void MainWindow::on_statepack_path_pb_clicked()
{
    getDir(ui->statepack_path_line, DATA_DIR_PATH);
    this->on_statepack_path_line_editingFinished();
}



void MainWindow::on_video_path_line_editingFinished()
{
    settings->setValue("path/video",ui->video_path_line->text());
}

void MainWindow::on_videopack_path_line_editingFinished()
{
    settings->setValue("path/videopack",ui->videopack_path_line->text());
}

void MainWindow::on_cammat_path_line_editingFinished()
{
    settings->setValue("path/camcat",ui->cammat_path_line->text());
}

void MainWindow::on_trackpack_path_line_editingFinished()
{
    settings->setValue("path/trackpack",ui->trackpack_path_line->text());
}

void MainWindow::on_triangpack_path_line_editingFinished()
{
    settings->setValue("path/triangpack",ui->triangpack_path_line->text());
}

void MainWindow::on_statepack_path_line_editingFinished()
{
    settings->setValue("path/statepack",ui->statepack_path_line->text());
}




// Analysis Tab


void MainWindow::on_plot_pb_clicked()
{
    getFile(ui->plot_line, ui->statepack_path_line->text());
}

void MainWindow::on_plot_run_clicked()
{
    QString exec_path = ui->plot_exec_line->text();
    QStringList args;

    if(ui->plot_line->text().isEmpty())
        return;

    QString file_path = ui->plot_line->text();

    args.append(exec_path);
    args.append(file_path);

    int status = qproc->execute("python",args);

    std::cout << "Execution Status: " << status << std::endl;


}
