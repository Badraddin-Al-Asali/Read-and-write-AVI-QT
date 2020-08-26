#include "myaudioplay.h"

MyAudioPlay::MyAudioPlay(QObject *parent) :QObject(parent),
    audioOutput(Q_NULLPTR),
    audioDevice(Q_NULLPTR)
{

}

void MyAudioPlay::setAudioDevice(const ReadRAWAVI::WAVEFORMATEXSTR *audioSettings){
    if(audioSettings == Q_NULLPTR)
        return;
    if(audioOutput != Q_NULLPTR) {
        audioOutput->stop();
        delete audioOutput;
        audioOutput = Q_NULLPTR;
    }
    //qDebug() << "audioFormat->sampleRate()" << audioFormat->sampleRate();
    // Set up the format, eg.
    audioFormat.setSampleRate(audioSettings->nSamplesPerSec);
    audioFormat.setChannelCount(audioSettings->nChannels);
    audioFormat.setSampleSize(audioSettings->wBitsPerSample);
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::UnSignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(audioFormat)) {
        qWarning() << "Raw audio format not supported by backend, cannot play audio.";
        return;
    }

    audioOutput = new QAudioOutput(audioFormat, this);
    audioDevice = audioOutput->start();
}

void MyAudioPlay::on_audioSample_Ready(unsigned char *audioData, const int& dataSize){
    auto state = audioOutput->state();
    if (state == QAudio::ActiveState || state == QAudio::IdleState) {
        //emit saveAudioSample(QByteArray((char*) audioData, dataSize));
        while(audioOutput->bytesFree() < dataSize) {
            QThread::msleep(40);
        }
        audioDevice->write((char*)audioData, dataSize);
    }
}
