#ifndef MYMICROPHONE_H
#define MYMICROPHONE_H

#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QAudioInput>
#include <QFile>
#include <QDebug>
#include <QThread>
class MyMicrophone : public QObject
{
Q_OBJECT
public:
    MyMicrophone(QObject *parent = Q_NULLPTR);
    void setup();
    void startListening();
    void stopListening();
    const QAudioFormat *getAudioFormate();
signals:
    void saveAudioSample(QByteArray audioSample);
private slots:
    void audioDataReady();
private:
    QAudioFormat m_format;
    QAudioDeviceInfo m_audioInputDevice;
    QAudioInput *m_audioInput;
    QIODevice *m_audioInputIODevice;
    bool readMicrophone;
};

#endif // MYMICROPHONE_H
