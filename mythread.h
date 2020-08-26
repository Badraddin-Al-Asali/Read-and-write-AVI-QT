#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
class MyThread:public QThread
{
    Q_OBJECT
public:
    MyThread(QThread *parent = Q_NULLPTR);
protected:
    void run();
};

#endif // MYTHREAD_H
