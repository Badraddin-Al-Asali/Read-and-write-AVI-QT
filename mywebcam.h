#ifndef MYWEBCAM_H
#define MYWEBCAM_H

#include <QAbstractVideoSurface>
#include <QImage>
#include <QDebug>
class MyWebcam : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    MyWebcam(QObject * parent = Q_NULLPTR);
public slots:
    void processFrame(const QVideoFrame &frame);
signals:
    void imageReady(QImage img);
    void imageReadyYUV(const QByteArray img);
protected:
    QList<QVideoFrame::PixelFormat>supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const;
    bool present(const QVideoFrame& frame);
private:
    void YUV422toRGB888(unsigned char *src);
    void YUV422toRGB888_2(unsigned char *src, unsigned char *dst);
    void flipImg(const QImage &img);
    QImage img;
    unsigned char *data1, *data2, *startData, *endData, *startDataMirror, *endDataStatic, *endDataYUV;
    int line, linex2, height, mirrorStartPos, rbgSize;
    QByteArray YUV422;
};

#endif // MYWEBCAM_H
