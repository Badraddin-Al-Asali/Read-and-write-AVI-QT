#include "myvideoplay.h"

MyVideoPlay::MyVideoPlay(QObject *parent):QObject(parent), delay(0)
{

}

void MyVideoPlay::setDelay(const int delay) {
    this->delay = delay;
}

int MyVideoPlay::getDelay() const {
    return delay;
}

void MyVideoPlay::on_frame_Ready(QImage *frame) {
    emit frameDisplayReady(frame);
    QThread::msleep(delay);
}
