#include "mydisplaywidget.h"

MyDisplayWidget::MyDisplayWidget(QWidget *parent): QOpenGLWidget(parent)
{

}

void MyDisplayWidget::display(const QImage &img){
    image = img;
    this->update();
}

void MyDisplayWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), image);
}
