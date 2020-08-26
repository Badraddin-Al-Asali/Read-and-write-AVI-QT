#ifndef MYAUDIOPLAY_H
#define MYAUDIOPLAY_H

#include <QAudioOutput>
#include "readrawavi.h"
class MyAudioPlay: public QObject
{
Q_OBJECT
public:
    MyAudioPlay(QObject *parent = Q_NULLPTR);
    void setAudioDevice(const ReadRAWAVI::WAVEFORMATEXSTR *audioSettings);
signals:
    void saveAudioSample(const QByteArray audipSample);
public slots:
    void on_audioSample_Ready(unsigned char *audioData, const int& dataSize);
private:
    QAudioOutput *audioOutput; // class member.
    QAudioFormat audioFormat;
    QIODevice *audioDevice;
};

#endif // MYAUDIOPLAY_H
