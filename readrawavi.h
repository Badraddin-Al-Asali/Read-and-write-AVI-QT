#ifndef READRAWAVI_H
#define READRAWAVI_H

#include <QFile>
#include <QDebug>
#include <QThread>
#include <QImage>

class ReadRAWAVI: public QObject
{
Q_OBJECT
public:
    QByteArray removeMe;
    static const unsigned char RIFF[4];
    static const unsigned char RIFF_Childs[11][4];
    static int FIRST50Frames;

    enum STREAMPKTTYPE{
        UNCOMPRESSEDVIDEO = 0x6264,
        CPMPRESSEDVIDEO   = 0x6364,
        PALETTECHANGE     = 0x6370,
        AUDIODATA         = 0x6277
    };

    enum STREAMTYPE {
        VIDEO = 0x73646976,
        AUDIO = 0x73647561,
        MIDI  = 0x736d6964,
        TEXT  = 0x73747874
    };

    enum WAVEFORMATETYPE {
        PCM        = 0x0001,
        EXTENSIBLE = 0xFFFE
    };

    enum BITMAPCOMPRESION {
        RAW        = 0x00,
        YUYV       = '2YUY',
        I420       = '024I'
    };

    struct STREAMSTR
    {
       short streamNumber;
       short streamType;
    };

    ReadRAWAVI(QObject *parent = Q_NULLPTR);
    QString readNextByte();
    QString readNextChunk();
    QImage getImage();

    struct AVIMAINHEADERSTR {
      int fcc; // 4
      int cb; // 8
      int dwMicroSecPerFrame; // 12
      int dwMaxBytesPerSec; // 16
      int dwPaddingGranularity; //20
      int dwFlags; // 24
      int dwTotalFrames; // 28
      int dwInitialFrames; // 32
      int dwStreams; // 36
      int dwSuggestedBufferSize; // 40
      int dwWidth; // 44
      int dwHeight; // 48
      int dwReserved[4]; // 64
    };
    struct AVISTREAMHEADERSTR {
      int fcc; // 4
      int cb; // 8
      int fccType; // 12
      int fccHandler; // 16
      int  dwFlags; // 20
      short wPriority; // 22
      short wLanguage; // 24
      int dwInitialFrames; // 28
      int dwScale; // 32
      int dwRate; // 36
      int dwStart; // 40
      int dwLength; // 44
      int dwSuggestedBufferSize; // 48
      int dwQuality; // 52
      int dwSampleSize; // 56
      struct {
          short int left; // 58
          short int top; // 60
          short int right; // 62
          short int bottom; // 64
      } rcFrame;
    };

    struct BITMAPINFOHEADERSTR {
      int biSize; // 4
      int biWidth; // 8
      int biHeight; // 12
      short biPlanes; // 14
      short biBitCount; // 16
      int biCompression; // 20
      int biSizeImage; // 24
      int biXPelsPerMeter; // 28
      int biYPelsPerMeter; // 32
      int biClrUsed; // 36
      int biClrImportant; // 40
    };

    struct WAVEFORMATEXSTR {
      short  wFormatTag; // 2
      short  nChannels; // 4
      int nSamplesPerSec; // 8
      int nAvgBytesPerSec; // 12
      short  nBlockAlign; // 14
      short  wBitsPerSample; // 16
      short  cbSize; // 18
    };

    const AVIMAINHEADERSTR *getMainHeader() const;
    const AVISTREAMHEADERSTR *getVideoStreamHeader() const;
    const AVISTREAMHEADERSTR *getAudioStreamHeader() const;
    const BITMAPINFOHEADERSTR *getBitmapInfoHeader() const;
    const WAVEFORMATEXSTR *getWaveFormateHeader() const;

    static int bufferSize;
signals:
    void videoReady();
    void frameReady(QImage *frame);
    void audioSampleReady(unsigned char *audioData, const int& dataSize);
public slots:
    void openAVI();
    void playAVI();
private:

    void AVIMAINHEADER(AVIMAINHEADERSTR *mainHeader);
    void AVISTREAMHEADER(AVISTREAMHEADERSTR *streamHeader);
    void BITMAPINFOHEADER(BITMAPINFOHEADERSTR *bitmapInfoHeader);
    void WAVEFORMATEX(WAVEFORMATEXSTR *waveFormateEx);
    void readHeaderList();
    void readStreamHeaderList();
    void readMovi();

    void decodeFrameI420(unsigned char *pkt, const int &pktSize);
    void decodeFrameYUYV(unsigned char *pkt, const int &pktSize);
    void reverseImageTable(unsigned char *pkt, const int &pktSize);


    qint8 errors;
    qint64 index, maxSize;
    unsigned int *bytePositon, aviSize;
    QFile rawAVIFile;
    QByteArray aviData;

    AVIMAINHEADERSTR *mainHeader;
    AVISTREAMHEADERSTR *streamHeaders[4];
    AVISTREAMHEADERSTR *streamHeader;
    BITMAPINFOHEADERSTR *bitmapInfoHeader;
    WAVEFORMATEXSTR *waveFormateEx;
    QImage *img;
    QImage frameBuffer[10];
    int bufferIndex;
    unsigned int moviChunkSize;
};

#endif // READRAWAVI_H
