#ifndef MYVIDEOPLAY_H
#define MYVIDEOPLAY_H

#include <QImage>
#include <QThread>
#include "readrawavi.h"
class MyVideoPlay: public QObject
{
Q_OBJECT
public:
    MyVideoPlay(QObject *parent = Q_NULLPTR);
    void setDelay(const int delay);
    int getDelay() const;
signals:
    void frameDisplayReady(QImage *frame);
public slots:
    void on_frame_Ready(QImage *frame);
private:
    int delay;
};

#endif // MYVIDEOPLAY_H
