#include "mymicrophone.h"

MyMicrophone::MyMicrophone(QObject *parent) : QObject(parent),
    m_audioInput(Q_NULLPTR),
    m_audioInputIODevice(Q_NULLPTR)
{
}

void MyMicrophone::setup() {
    if(m_audioInput == Q_NULLPTR) {

        // Select the device that I want
        QList<QAudioDeviceInfo> availableDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

        for(int i=0;i<availableDevices.size();i++){
            qDebug() << availableDevices.at(i).deviceName();
        }
        m_audioInputDevice = QAudioDeviceInfo::defaultInputDevice();
        auto channelsList = m_audioInputDevice.supportedChannelCounts();
        int maxChannelCount(0);
        for(int i(0); i < channelsList.size(); ++i){
            if(channelsList.at(i) > maxChannelCount) {
                maxChannelCount = channelsList.at(i);
            }
        }
        m_format.setChannelCount(2);
        m_format.setCodec("audio/pcm");
        auto sampleSizesList = m_audioInputDevice.supportedSampleSizes();
        int maxSampleSizes(0);
        for(auto i(0); i < sampleSizesList.size(); ++i){
            if(sampleSizesList.at(i) > maxSampleSizes) {
                maxSampleSizes = sampleSizesList.at(i);
            }
        }
        m_format.setSampleSize(16);
        auto sampleRatesList = m_audioInputDevice.supportedSampleRates();
        int maxSampleRate(0);
        for(auto i(0); i < sampleRatesList.size(); ++i){
            if(sampleRatesList.at(i) > maxSampleRate) {
                maxSampleRate = sampleRatesList.at(i);
            }
        }
        m_format.setSampleRate(48000);
        m_format.setByteOrder(QAudioFormat::LittleEndian);
        m_format.setSampleType(QAudioFormat::UnSignedInt);
        // Check format is OK
        if(!m_audioInputDevice.isFormatSupported(m_format)) {
            qWarning() << "Default format not supported, trying to use the nearest.";
            m_format = m_audioInputDevice.nearestFormat(m_format);
            qDebug() << "Codec:" << m_format.codec();
            qDebug() << "Channel count:" << m_format.channelCount();
            qDebug() << "Sample size:" << m_format.sampleSize();
            qDebug() << "Sample rate:" << m_format.sampleRate();
        }

        if(m_format.isValid()){
            qDebug() << "Codec:" << m_format.codec();
            qDebug() << "Max Channel count:" << m_format.channelCount();
            qDebug() << "Max Sample size:" << m_format.sampleSize();
            qDebug() << "Max Sample rate:" << m_format.sampleRate();
            // Initialisation du micro
            m_audioInput = new QAudioInput(m_audioInputDevice,m_format,this);
        }
        else {
            qCritical() << "Problem audio format 2";
        }
    }
}

void MyMicrophone::startListening(){
    if(m_audioInput != Q_NULLPTR) {

        readMicrophone = true;
        m_audioInputIODevice = m_audioInput->start();
        qDebug() << m_audioInput->error();
        connect(m_audioInputIODevice, &QIODevice::readyRead, this, &MyMicrophone::audioDataReady);

        qDebug() << "Start listening event" << m_audioInputIODevice << m_audioInput;
    }
    else {
        qCritical() << "Problem audio format 1";
    }
}

void MyMicrophone::audioDataReady() {
    if(readMicrophone) {
        int len = m_audioInput->bytesReady();
        QByteArray audioData = m_audioInputIODevice->read(len);
        emit saveAudioSample(audioData);
        //qDebug() << audioData.size() << len;
    }
}

void MyMicrophone::stopListening() {
    readMicrophone = false;
    disconnect(m_audioInputIODevice, &QIODevice::readyRead, this, &MyMicrophone::audioDataReady);
    m_audioInput->stop();
    while(m_audioInput->state() != QAudio::StoppedState)
    {
        QThread::msleep(40);
    }
    delete m_audioInput;
    m_audioInput = Q_NULLPTR;
}

const QAudioFormat *MyMicrophone::getAudioFormate() {
    return &m_format;
}
