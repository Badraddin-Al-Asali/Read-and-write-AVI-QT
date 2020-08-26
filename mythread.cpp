#include "mythread.h"

MyThread::MyThread(QThread *parent): QThread(parent)
{

}

void MyThread::run(){
    exec();
}
