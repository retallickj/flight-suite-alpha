#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QProcess>
#include <QLineEdit>
#include <QTextEdit>
#include <QSettings>

#include <iostream>

const QString SETTINGS_PATH  = "../../bin/settings.ini";
const QString EXEC_DIR_PATH = "../../Source";
const QString DATA_DIR_PATH = "../../../Data";

const QString SELECT_FILE_STR = "Select File...";
const QString SELECT_DIR_STR = "Select Directory...";
const QString SELECT_FILES_STR = "Select Files...";

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:


    // Video Synchronization
    
    void on_vsync_pb_add_clicked();

    void on_vsync_pb_in_clicked();

    void on_vsync_pb_out_clicked();

    void on_vsync_pb_run_clicked();

    void on_vsync_line_in_returnPressed();



    // Camera Calibration

    void on_camcal_pb_v1in_clicked();

    void on_camcal_pb_v2in_clicked();

    void on_camcal_pb_vpin_clicked();

    void on_camcal_pb_out_clicked();

    void on_camcal_pb_run_clicked();

    void on_camcal_clb_prev_clicked();

    void on_camcal_clb_next_clicked();

    void on_camcal_pb_v1add_clicked();

    void on_camcal_pb_v2add_clicked();

    void on_camcal_pb_vpadd_clicked();


    // Tracking

    void on_track_pb_vpin_clicked();

    void on_track_pb_add_clicked();

    void on_track_pb_vin_clicked();

    void on_track_line_vin_returnPressed();

    void on_track_pb_out_clicked();

    void on_track_pb_run_clicked();


    // Triangulation

    void on_triang_pb_tpin_clicked();

    void on_traing_pb_cmin_clicked();

    void on_triang_pb_out_clicked();

    void on_triang_pb_run_clicked();


    // Processing

    void on_proc_pb_trin_clicked();

    void on_proc_pb_out_clicked();

    void on_proc_pb_run_clicked();


    // Settings

    void on_vsync_exec_pb_clicked();

    void on_camcal_exec_pb_clicked();

    void on_track_exec_pb_clicked();

    void on_triang_exec_pb_clicked();

    void on_proc_exec_pb_clicked();

    void on_asis_exec_pb_clicked();


    void on_vsync_exec_line_editingFinished();

    void on_camcal_exec_line_editingFinished();

    void on_track_exec_line_editingFinished();

    void on_triang_exec_line_editingFinished();

    void on_proc_exec_line_editingFinished();

    void on_asis_exec_line_editingFinished();




    void on_settings_clb_1_clicked();

    void on_settings_clb_2_clicked();

    void on_video_path_pb_clicked();

    void on_videopack_path_pb_clicked();

    void on_cammat_path_pb_clicked();

    void on_trackpack_path_pb_clicked();

    void on_triangpack_path_pb_clicked();

    void on_statepack_path_pb_clicked();

    void on_video_path_line_editingFinished();

    void on_videopack_path_line_editingFinished();

    void on_cammat_path_line_editingFinished();

    void on_trackpack_path_line_editingFinished();

    void on_triangpack_path_line_editingFinished();

    void on_statepack_path_line_editingFinished();

    void on_vsync_line_freq_editingFinished();

    void on_vsync_cb_freq_clicked();

private:

    // helper functions for clean build

    void getFile(QLineEdit* line, QString dir = "");
    void getDir(QLineEdit* line, QString dir = "");
    void getFiles(QLineEdit* line, QString dir = "");
    void addFiles(QTextEdit* text, QLineEdit* line);
    void setDefaults();

    Ui::MainWindow *ui;

    QProcess* qproc;
    QSettings* settings;
};

#endif // MAINWINDOW_H
