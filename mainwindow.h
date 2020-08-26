#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include <QMessageBox>
#include <QTimer>
#include "mythread.h"
#include "myaudioplay.h"
#include "myvideoplay.h"
#include "writerawavi.h"
#include "mywebcam.h"
#include "mymicrophone.h"
#include <QCamera>
#include <QCameraViewfinderSettings>
#include <QCameraInfo>
#include <QVideoProbe>

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
    void on_nextFrameButton_clicked();

    void on_saveOutput_clicked();

    void on_nextFrameButton_pressed();

    void on_nextFrameButton_hold();

    void on_nextFrameButton_released();

    void on_readNBytes_clicked();

    void on_readChunk_clicked();

    void on_showFrame_clicked();

    void on_videoReady();

    void on_frameDisplay_Ready(QImage *frame);
    void on_frameDisplayFromCamera_Ready(QImage frame);
    void on_StopSave_clicked();

private:
    Ui::MainWindow *ui;
    QStringListModel *model;
    QTimer *timer;
    ReadRAWAVI rawAVI;
    WriteRawAVI writeRawAVI;
    MyAudioPlay myAudioPlay;
    MyVideoPlay myVideoPlay;
    MyThread aviThread, audioPlayThread, videoPlayThread;//, saveThread;
    QMessageBox msgBoxSaved;
    QCamera *camera;
    MyWebcam webcam;
    MyMicrophone myMicrophne;
    QCameraViewfinderSettings cameraSettings;
    QVideoProbe probe;
    bool on_nextFrameButton_hold_active;
    void setWebcamSave();
signals:
    void playVideo();
    void openVideo();
    void saveEncodedFrame(const QByteArray frame);
    void saveFrame(QImage *frame);
    void saveFrameWeb(QImage frame);
};

#endif // MAINWINDOW_H
