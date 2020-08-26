#ifndef WRITERAWAVI_H
#define WRITERAWAVI_H

#include "readrawavi.h"

#define OLDINDEXCHUNK  0x31786469
class WriteRawAVI :public QObject
{
Q_OBJECT
public:
    WriteRawAVI(QObject *parent = Q_NULLPTR);

    struct AVIALLHEADERS {
        const ReadRAWAVI::AVIMAINHEADERSTR *mainHeader;
        const ReadRAWAVI::AVISTREAMHEADERSTR *videoStreamHeader;
        const ReadRAWAVI::AVISTREAMHEADERSTR *audioStreamHeader;
        const ReadRAWAVI::BITMAPINFOHEADERSTR *bitMapInfoHeader;
        const ReadRAWAVI::WAVEFORMATEXSTR *waveFormateEx;
    };

    void openFile(const QString &filename);
    void writeHeader(const AVIALLHEADERS &header);
    void close();
public slots:
    void writeVideoFrame(QImage *frame);
    void writeVideoFrameWeb(QImage frame);
    void writeEncodedFrame(const QByteArray frame);
    void writeAudioFrame(const QByteArray sample);
private:
    struct AVIOLDINDEX {
      int fcc; // 4
      int  cb; // 8
      struct {
        int dwChunkId; // 12
        int dwFlags; // 16
        int dwOffset; // 20
        int dwSize; // 24
      };
    };
    void writeMainHeader(const ReadRAWAVI::AVIMAINHEADERSTR *mainHeader);
    void writeStreamHeader(const ReadRAWAVI::AVISTREAMHEADERSTR *streamHeader);
    void writeBitMapInfoHeader(const ReadRAWAVI::BITMAPINFOHEADERSTR *bitMapInfoHeader);
    void writeWaveFormateEx(const ReadRAWAVI::WAVEFORMATEXSTR *waveFormateEx);
    void createAVI(const int &fileSize);
    void createList(const int &size);
    void createStreamHeaderList();
    void createStreamFormate(const int &size);
    void createMOVI();
    void pushStream(const ReadRAWAVI::STREAMSTR &stream, const int &size);
    void encodeFrameI420(const QImage &inFrame);
    void writeOldIndexChunk();
    int calculateHeaderListSize(const ReadRAWAVI::AVISTREAMHEADERSTR *videoStreamHeader, const ReadRAWAVI::AVISTREAMHEADERSTR *audioStreamHeader, const ReadRAWAVI::WAVEFORMATEXSTR *waveFormateEx);
    void writeSize();
    void reverseImageTable(QImage *frame);
    enum AVIOLDINDEXFLAGS{
        LIST = 0x00000001,
        KEYFRAME = 0x00000010,
        NO_TIME = 0x00000100
    };
    enum AVIMAINHEADERFLAGS{
        HASINDEX = 0x00000010,
        MUSTUSEINDEX = 0x00000020,
        ISINTERLEAVED = 0x00000100,
        TRUSTCKTYPE = 0x00000800,
        WASCAPTUREFILE = 0x00010000,
        COPYRIGHTED = 0x00020000,
        MAX_RIFF_SIZE = 0x40000000LL,
        MASTER_INDEX_SIZE = 256
    };
    enum Compression
    {
       BI_RGB = 0x0000,
       BI_RLE8 = 0x0001,
       BI_RLE4 = 0x0002,
       BI_BITFIELDS = 0x0003,
       BI_JPEG = 0x0004,
       BI_PNG = 0x0005,
       BI_CMYK = 0x000B,
       BI_CMYKRLE8 = 0x000C,
       BI_CMYKRLE4 = 0x000D
     };
    QFile outputFile;
    ReadRAWAVI::STREAMSTR streams[4];
    qint64 seekBackPositions[5];
    QByteArray encodedFrame;
    int videoFrameWritten;
    int videoFrameByteCount;
    int audioSamplesWritten;
    int audioSamplesWrittenCounter;
    int headerListSize;
    int tempSeekPostion;
};

#endif // WRITERAWAVI_H
