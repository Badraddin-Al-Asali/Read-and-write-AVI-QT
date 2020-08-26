#ifndef MYDISPLAYWIDGET_H
#define MYDISPLAYWIDGET_H

#include <QOpenGLWidget>
#include <QPainter>
class MyDisplayWidget :public QOpenGLWidget
{
Q_OBJECT
public:
    MyDisplayWidget(QWidget *parent = Q_NULLPTR);
    void display(const QImage &img);
protected:
    void paintEvent(QPaintEvent*);
private:
    QImage image;
};

#endif // MYDISPLAYWIDGET_H
