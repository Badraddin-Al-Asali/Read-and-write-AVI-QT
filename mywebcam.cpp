#include "mywebcam.h"

MyWebcam::MyWebcam(QObject * parent) : QAbstractVideoSurface(parent), img(1280, 720, QImage::Format_RGB888)
{
    startData = img.bits();
    data1 = startData;
    line = img.bytesPerLine();
    linex2 = line + line;
    height = img.height() - 1;
    mirrorStartPos = line * height;
    startDataMirror = data1 + mirrorStartPos;
    data2 = startDataMirror;
    endDataStatic = data1 + line;
    endData = endDataStatic;
    endDataYUV = startData + img.byteCount();
    rbgSize = (img.height() * img.width()) / 2;
    int YuvSize(img.height() * img.width());
    YuvSize = YuvSize + (YuvSize / 2) + (YuvSize / 2);
    YUV422 = QByteArray(YuvSize, '/0');
}

QList<QVideoFrame::PixelFormat> MyWebcam::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const {
    return QList<QVideoFrame::PixelFormat>() << /*QVideoFrame::Format_RGB32;*/QVideoFrame::Format_YUYV; // here return whatever formats you will handle
    //return QList<QVideoFrame::PixelFormat>();
}
#include <QThread>
bool MyWebcam::present(const QVideoFrame& frame) {
    if (frame.isValid()) {
        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        //qDebug() << "Test QAbstractVideoSurface";
        //qDebug() << cloneFrame.pixelFormat();
        //YUV422toRGB888(cloneFrame.bits());
        //YUV422toRGB888_2(cloneFrame.bits(), startData);
        //qDebug() << "TTTTTTTTTTTTTTTTTT";
        //QThread::msleep(10000);
        memcpy(YUV422.data(), cloneFrame.bits(), YUV422.size());
        cloneFrame.unmap();
        emit imageReadyYUV(YUV422);
        //const QImage img(cloneFrame.bits(),
        //             cloneFrame.width(),
        //             cloneFrame.height(),
        //            QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat()));
        //flipImg(img);
        //emit imageReady(this->img.copy());
        //cloneFrame.unmap();
       // do something with the image ...



        return true;
    }
    return false;
}

void MyWebcam::processFrame(const QVideoFrame &frame) {
    if (frame.isValid()) {
        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        QThread::msleep(10000);
        //qDebug() << "Test QVideoProb";
        cloneFrame.unmap();
    }
}

void MyWebcam::flipImg(const QImage &img) {
    int line = this->line;
    int linex2 = this->linex2;
    int srcLine = img.bytesPerLine();
    int srcLinex2 = srcLine + srcLine;
    unsigned char *dstData1 = startData;
    unsigned char *dstdata2 = startDataMirror;
    const unsigned char *temp(img.bits());
    const unsigned char *srcData1 = temp;
    const unsigned char *srcData2 = temp + (img.height() - 1) * srcLine;
    unsigned char *dstEndData1 = endDataStatic;
    //int countY(0), countX(0), countLoop(0);
    do{
        //++countY;
        //countX = 0;
        do{
            //++countX;
            //++countLoop;


            *dstData1 = srcData2[2]; //R
            ++dstData1;
            *dstData1 = srcData2[1]; //G
            ++dstData1;
            *dstData1 = *srcData2; //B
            ++dstData1;
            srcData2 += 4; // skip alpha

            *dstdata2 = srcData1[2]; //R
            ++dstdata2;
            *dstdata2 = srcData1[1]; //G
            ++dstdata2;
            *dstdata2 = *srcData1; //B
            ++dstdata2;
            srcData1 += 4; // skip alpha

        }while (dstData1 < dstEndData1);
        dstEndData1 += line;
        dstdata2 -= linex2;
        srcData2 -= srcLinex2;
    }while (dstData1 < dstdata2);
    //qDebug() << countY << countX << countLoop;
}

// Clamp out of range values
#define CLAMP(t) (((t)>255)?255:(((t)<0)?0:(t)))
// Color space conversion for RGB
#define GET_R_FROM_YUV2(y, u, v) ((y+409*v+128)>>8)
#define GET_G_FROM_YUV2(y, u, v) ((y-100*u-208*v+128)>>8)
#define GET_B_FROM_YUV2(y, u, v) ((y+516*u+128)>>8)

void MyWebcam::YUV422toRGB888(unsigned char *src)
{
    unsigned char *data = startData;
    int y0, u, y2, v;
    do
    {
        y0 = (*src - 16) * 298;
        ++src;
        u = *src - 128;
        ++src;
        y2 = (*src - 16) * 298;
        ++src;
        v = *src - 128;
        ++src;
        // RGB
        *data = CLAMP(GET_R_FROM_YUV2(y0, u, v));
        ++data;
        *data = CLAMP(GET_G_FROM_YUV2(y0, u, v));
        ++data;
        *data = CLAMP(GET_B_FROM_YUV2(y0, u, v));
        ++data;

        // RGB
        *data = CLAMP(GET_R_FROM_YUV2(y2, u, v));
        ++data;
        *data = CLAMP(GET_G_FROM_YUV2(y2, u, v));
        ++data;
        *data = CLAMP(GET_B_FROM_YUV2(y2, u, v));
        ++data;

    } while(data < endDataYUV);
}

// Color space conversion for RGB
#define GET_R_FROM_YUV(y, u, v) ((298*y+409*v+128)>>8)
#define GET_G_FROM_YUV(y, u, v) ((298*y-100*u-208*v+128)>>8)
#define GET_B_FROM_YUV(y, u, v) ((298*y+516*u+128)>>8)

void MyWebcam::YUV422toRGB888_2(unsigned char *src, unsigned char *dst)
{
    for (unsigned int i = 0; i < rbgSize; ++i)
        {
            int y0 = *src++ - 16;
            int u0 = *src++ - 128;
            int y2 = *src++ - 16;
            int v = *src++ - 128;

            // RGB
            *dst++ = CLAMP(GET_R_FROM_YUV(y0, u0, v));
            *dst++ = CLAMP(GET_G_FROM_YUV(y0, u0, v));
            *dst++ = CLAMP(GET_B_FROM_YUV(y0, u0, v));


            // RGB
            *dst++ = CLAMP(GET_R_FROM_YUV(y2, u0, v));
            *dst++ = CLAMP(GET_G_FROM_YUV(y2, u0, v));
            *dst++ = CLAMP(GET_B_FROM_YUV(y2, u0, v));

        }
}
