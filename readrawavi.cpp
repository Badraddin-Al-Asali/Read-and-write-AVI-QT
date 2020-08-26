#include "readrawavi.h"

int ReadRAWAVI::bufferSize(0);
int ReadRAWAVI::FIRST50Frames(50);
const unsigned char ReadRAWAVI::RIFF[4] = { 'R', 'I', 'F', 'F' };
const unsigned char ReadRAWAVI::RIFF_Childs[11][4]={
                                      { 'A', 'V', 'I', ' ' },
                                      { 'L', 'I', 'S', 'T' },
                                      { 'H', 'D', 'R', 'L' },
                                      { 'h', 'd', 'r', 'l' },
                                      { 'S', 'T', 'R', 'L' },
                                      { 's', 't', 'r', 'l' },
                                      { 'S', 'T', 'R', 'F' },
                                      { 's', 't', 'r', 'f' },
                                      { 'M', 'O', 'V', 'I' },
                                      { 'm', 'o', 'v', 'i' },
                                      { 'J', 'U', 'N', 'K' },};
ReadRAWAVI::ReadRAWAVI(QObject *parent) :QObject(parent), errors(0), index(0), bytePositon(0), rawAVIFile("C:\\Users\\Andy BadMan\\Desktop\\mp4\\myAVIWriteFile1VideoFrameTEST.avi"),
    mainHeader(Q_NULLPTR),
    streamHeader(Q_NULLPTR),
    bitmapInfoHeader(Q_NULLPTR),
    waveFormateEx(Q_NULLPTR)
{
    if (!rawAVIFile.open(QIODevice::ReadOnly)) {
        errors = -1;
        return ;
    }
    maxSize = rawAVIFile.size();
    aviData = rawAVIFile.read(maxSize);
    bytePositon = (unsigned int*)aviData.data();
}

QString ReadRAWAVI::readNextByte() {
    if(!errors) {
        const int numberOfBytes(4);
        int bytesRead(0);
        unsigned char values[numberOfBytes];

        QString valueDecimals[numberOfBytes];

        QString valueHex[numberOfBytes];

        QString indexes[numberOfBytes];

        QString output;
        for(; bytesRead < numberOfBytes && index < maxSize; ++index, ++bytesRead) {
            qDebug() << index;
            values[bytesRead] = aviData.at(index);
            QString hexadecimal;
            QString charecter(values[bytesRead]);
            output += (charecter != ' ' || values[bytesRead] != 0) ? charecter : QString('.');
            valueHex[bytesRead] = hexadecimal.setNum(values[bytesRead], 16);
            if(valueHex[bytesRead].size() < 2)
                valueHex[bytesRead].push_front('0');
            valueDecimals[bytesRead] = QString::number((int)values[bytesRead]);
            indexes[bytesRead] = QString::number(index);
            //qDebug() << output;
        }
        int i(0);
        output += '[';
        for(; i < bytesRead; ++i){
            output += + " " + valueDecimals[i];
        }
        output += ']';
        output += '[';
        QString hexadecimal;
        for(i = 0; i < bytesRead; ++i){
            hexadecimal += valueHex[bytesRead - 1 - i];
            output += + " " + valueHex[i];
        }
        output += ']';
        bool bStatus(false);
        //std::reverse(hexadecimal.begin(), hexadecimal.end());
        uint number4Bytes(hexadecimal.toUInt(&bStatus, 16));
        if(bStatus)
            output += '{' + QString::number(number4Bytes) + '}';
        output += + " " + indexes[bytesRead-1];
        return output;
    }
    return NULL;
}

QString ReadRAWAVI::readNextChunk()
{
    if(!errors) {
        QString output;
        int chunkSize(0);

        const int *RIFF_ptr = (const int*)ReadRAWAVI::RIFF;
        const int *RIFF_Childs_ptr = (const int*)ReadRAWAVI::RIFF_Childs[0];


        if(*bytePositon == *RIFF_ptr) {
            ++bytePositon;
            chunkSize = *bytePositon;
            aviSize = chunkSize;
            output = "RIFF " + QString::number(chunkSize);
            ++bytePositon;
        }

        if(*bytePositon == RIFF_Childs_ptr[0]) {
            output += " AVI ";
            ++bytePositon;
        }
        qDebug() << output;
        readHeaderList();

        //qDebug() << output;
        //++bytePositon;
    }
    return NULL;
}
void ReadRAWAVI::readHeaderList() {
    //const int *RIFF_ptr = (const int*)RIFF;
    const unsigned int *RIFF_Childs_ptr = (const unsigned int*)ReadRAWAVI::RIFF_Childs[0];

    mainHeader = Q_NULLPTR;
    streamHeader = Q_NULLPTR;
    bitmapInfoHeader = Q_NULLPTR;
    waveFormateEx = Q_NULLPTR;
    for(int i(-1); i < 4; streamHeaders[++i] = Q_NULLPTR);
    moviChunkSize = 0;
    //QString output;
    int chunkSize(0);

    if(*bytePositon == RIFF_Childs_ptr[1]) {
        //------------Get List Size-------------
        ++bytePositon;
        chunkSize = *bytePositon;
        //--------------------------------------
        //----Get Position of The Next List-----
        unsigned char *nextListPos((unsigned char *)bytePositon);
        //--------------------------------------
        ++bytePositon;
        if(*bytePositon == RIFF_Childs_ptr[2] || *bytePositon == RIFF_Childs_ptr[3]) {
            //--------Get Main Header Stream--------
            ++bytePositon;
            mainHeader = (AVIMAINHEADERSTR*)bytePositon;
            AVIMAINHEADER(mainHeader);
            //--------------------------------------
            qDebug() << "==========";
            //-----------Go To Next Chunk-----------
            bytePositon += 16;
            //--------------------------------------
            //-------Read All Stream Headers--------
            for(int i(0); i < mainHeader->dwStreams; ++ i){
                readStreamHeaderList();
            }
            //--------------------------------------
        }

        bool moviFound(false);
        //---------Find the MOVI Chunk----------
        do {
            //--------Go To Next List Chunk---------
            bytePositon = (unsigned int*)(nextListPos + chunkSize + sizeof(chunkSize));
            //--------------------------------------
            if(*bytePositon == RIFF_Childs_ptr[1] || RIFF_Childs_ptr[10]) {
                //------------Get List Size-------------
                ++bytePositon;
                chunkSize = *bytePositon;
                //--------------------------------------
                //----Get Position of The Next List-----
                nextListPos = (unsigned char *)bytePositon;
                //--------------------------------------
                //-----------Go to Next Byte------------
                ++bytePositon;
                //--------------------------------------
                //----Check Byte Value Equals to MOVI---
                moviFound = *bytePositon == RIFF_Childs_ptr[8] || *bytePositon == RIFF_Childs_ptr[9];
                //--------------------------------------
            }
        } while (!moviFound);
        //--------------------------------------
        if(moviFound) {
            moviChunkSize = chunkSize;
            emit videoReady();
            //readMovi();
        }
    }
}

void ReadRAWAVI::readStreamHeaderList() {

    const unsigned int *RIFF_Childs_ptr = (const unsigned int*)ReadRAWAVI::RIFF_Childs[0];
    int chunkSize(0);
    QString output;


    if(*bytePositon == RIFF_Childs_ptr[1]) {
        //------------Get List Size-------------
        ++bytePositon;
        chunkSize = *bytePositon + sizeof(chunkSize);
        //--------------------------------------
        //----Get Position of The Next List-----
        unsigned char *nextListPos((unsigned char *)bytePositon);
        nextListPos += chunkSize;
        //--------------------------------------
        //-----------Go to Next Byte------------
        ++bytePositon;
        //--------------------------------------
        if(*bytePositon == RIFF_Childs_ptr[4] || *bytePositon == RIFF_Childs_ptr[5]) {
            //------Go to Stream Header Chunk------
            ++bytePositon;
            //--------------------------------------
            streamHeader = (AVISTREAMHEADERSTR*)bytePositon;
            AVISTREAMHEADER(streamHeader);
            //-----------Go to Next Chunk-----------
            bytePositon += 16;
            //--------------------------------------
            if(*bytePositon == RIFF_Childs_ptr[6] || *bytePositon == RIFF_Childs_ptr[7]) {
                //------------Get Chunk Size------------
                ++bytePositon;
                int streamFormatechunkSize = *bytePositon;
                //--------------------------------------
                //---Get Position of The Next Chunk-----
                unsigned char *jumpPtr((unsigned char *)bytePositon);
                jumpPtr += streamFormatechunkSize;
                //--------------------------------------
                //------Go to Stream Formate Chunk------
                ++bytePositon;
                //--------------------------------------
                output += " STRF " + QString::number(chunkSize);
                //-----------Get Stream Type------------
                switch (streamHeader->fccType) {
                //--------------------------------------
                    case STREAMTYPE::VIDEO:{
                        streamHeaders[0] = streamHeader;
                        bitmapInfoHeader = (BITMAPINFOHEADERSTR*)bytePositon;
                        BITMAPINFOHEADER(bitmapInfoHeader);
                    }
                    break;
                    case STREAMTYPE::AUDIO:{
                        streamHeaders[1] = streamHeader;
                        waveFormateEx = (WAVEFORMATEXSTR*)bytePositon;
                        WAVEFORMATEX(waveFormateEx);
                    }
                    break;
                    default:
                        qDebug() << "Stream type is not audio or video";
                    break;
                }
            }
        }
        //--------Go To Next List Chunk---------
        //qDebug() << "==============";
        bytePositon = (unsigned int*)nextListPos;
        //qDebug() << *bytePositon << QString(QByteArray((char*) bytePositon, 4));
        //--------------------------------------
    }
}

void ReadRAWAVI::reverseImageTable(unsigned char *pkt, const int &pktSize) {
    QImage *frame = &frameBuffer[++bufferIndex%10];
    int line(frame->bytesPerLine());
    int lineX2((line*2));
    unsigned char *startD(pkt), *endD(pkt + line * (frame->height() - 1));
    unsigned char *start(frame->bits());
    unsigned char *endX(start + line);
    unsigned char *end(start + line * (frame->height() - 1));
    do {
        do{
            *start = endD[2];
            *end = startD[2];
            ++start;
            ++end;
            *start = endD[1];
            *end = startD[1];
            ++start;
            ++end;
            *start = *endD;
            *end = *startD;
            ++start;
            ++end;
            startD += 3;
            endD += 3;
        }while (start < endX);
        endX += line;
        end -= lineX2;
        endD -= lineX2;
    }while (start < end);
    while (ReadRAWAVI::bufferSize >= 9) {
        QThread::msleep(40);
    }
    ++ReadRAWAVI::bufferSize;
    emit frameReady(frame);
}
//#include <QThread>
void ReadRAWAVI::readMovi() {
    unsigned int pktSize(0), cumulativePktSize(0);
    STREAMSTR *streamPktType;
    //----Go to Stream-----
    ++bytePositon;
    //---------------------
    qDebug() << "======";
    int frameCounter(0), numberofsamples(0);
    do {
        //qDebug() << bytePositon << QString(QByteArray((char*) bytePositon, 4));
        // Get Stream Type
        streamPktType = (STREAMSTR*)bytePositon;
        //---------------------
        // Get Stream Packet Size
        ++bytePositon;
        pktSize = *bytePositon;
        //---------------------
        // Get Stream Packet Data
        ++bytePositon;
        unsigned char *mByte((unsigned char *)bytePositon);
        //---------------------
        switch(streamPktType->streamType) {
            case STREAMPKTTYPE::UNCOMPRESSEDVIDEO:
                //--bytePositon;
                //qDebug() << QString(QByteArray((char*)bytePositon, 4));
                //--bytePositon;
                //qDebug() << QString(QByteArray((char*)bytePositon, 4));
                //qDebug() << "Uncompressed Video Frame" << pktSize << bitmapInfoHeader->biWidth * bitmapInfoHeader->biHeight * 3;
                //decodeFrameI420(mByte, pktSize);
                switch(bitmapInfoHeader->biCompression){
                    case BITMAPCOMPRESION::RAW:
                        reverseImageTable(mByte, pktSize);
                    break;
                    case BITMAPCOMPRESION::YUYV:{
                        decodeFrameYUYV(mByte, pktSize);
                    }
                    break;
                    case BITMAPCOMPRESION::I420:{
                        decodeFrameI420(mByte, pktSize);
                    }
                    break;
                }
                //qDebug() << "ReadRAWAVI::FIRST50Frames" << ReadRAWAVI::FIRST50Frames;
                //if(ReadRAWAVI::FIRST50Frames <= 0)
                //    return;
                ++frameCounter;

            break;
            case STREAMPKTTYPE::CPMPRESSEDVIDEO:
                switch(bitmapInfoHeader->biCompression){
                    case BITMAPCOMPRESION::RAW:
                    reverseImageTable(mByte, pktSize);
                    break;
                    case BITMAPCOMPRESION::YUYV:{
                        decodeFrameYUYV(mByte, pktSize);
                    }
                    break;
                    case BITMAPCOMPRESION::I420:{
                        decodeFrameI420(mByte, pktSize);
                    }
                    break;
                }
                //decodeFrameI420(mByte, pktSize);
                //decodeFrameYUYV(mByte, pktSize);
                //qDebug() << "ReadRAWAVI::FIRST50Frames" << ReadRAWAVI::FIRST50Frames << (ReadRAWAVI::FIRST50Frames <= 0);
                //if(ReadRAWAVI::FIRST50Frames <= 0)
                //    return;
                ++frameCounter;
            break;
            case STREAMPKTTYPE::PALETTECHANGE:
                qDebug() << "Palette Change";
            break;
            case STREAMPKTTYPE::AUDIODATA:
                numberofsamples += pktSize;
                emit audioSampleReady(mByte, pktSize);
                //qDebug() << "Audio Data" << pktSize;
                //QThread::msleep(100);
            break;
            default:
                qDebug() << "MOVI STREAM UNKNOWN" << cumulativePktSize << moviChunkSize << frameCounter << (numberofsamples / 48000.0) / 2 << QString(QByteArray((char*)&streamPktType->streamType, 2)) << pktSize;
        }
        //Go to Next Packet
        bytePositon = (unsigned int*)(mByte + pktSize);
        cumulativePktSize += pktSize;
        //----------------------
    } while (cumulativePktSize < moviChunkSize);
    //qDebug() << index;
    //qDebug() << ((unsigned char*)bytePositon) - ((unsigned char*)aviData.data());
    //qDebug() << cumulativePktSize << moviChunkSize * 2 << (numberofsamples / 48000.0) / 2 << QString(QByteArray((char*)&streamPktType->streamType, 2)) << pktSize;
    qDebug() << "Total Number of Frames:" << frameCounter;
}

void ReadRAWAVI::AVIMAINHEADER(AVIMAINHEADERSTR *mainHeader){
    qDebug() << "mainHeader->fcc" << mainHeader->fcc << QString(QByteArray((char*)&mainHeader->fcc, 4));
    qDebug() << "mainHeader->cb" << mainHeader->cb;
    qDebug() << "mainHeader->dwMicroSecPerFrame" << mainHeader->dwMicroSecPerFrame;
    qDebug() << "mainHeader->dwMaxBytesPerSec" << mainHeader->dwMaxBytesPerSec;
    qDebug() << "mainHeader->dwPaddingGranularity" << mainHeader->dwPaddingGranularity;
    qDebug() << "mainHeader->dwFlags" << mainHeader->dwFlags;
    qDebug() << "mainHeader->dwTotalFrames" << mainHeader->dwTotalFrames;
    qDebug() << "mainHeader->dwInitialFrames" << mainHeader->dwInitialFrames;
    qDebug() << "mainHeader->dwStreams" << mainHeader->dwStreams;
    qDebug() << "mainHeader->dwSuggestedBufferSize" << mainHeader->dwSuggestedBufferSize;
    qDebug() << "mainHeader->dwWidth" << mainHeader->dwWidth;
    qDebug() << "mainHeader->dwHeight" << mainHeader->dwHeight;
    qDebug() << "mainHeader->dwReserved[0]" << mainHeader->dwReserved[0];
    qDebug() << "mainHeader->dwReserved[1]" << mainHeader->dwReserved[1];
    qDebug() << "mainHeader->dwReserved[2]" << mainHeader->dwReserved[2];
    qDebug() << "mainHeader->dwReserved[3]" << mainHeader->dwReserved[3];
}

void ReadRAWAVI::AVISTREAMHEADER(AVISTREAMHEADERSTR *streamHeader) {
    qDebug() << "fcc" << streamHeader->fcc << QString(QByteArray((char*)&streamHeader->fcc, 4));
    qDebug() << "cb" << streamHeader->cb;
    qDebug() << "fccType" << streamHeader->fccType << QString(QByteArray((char*)&streamHeader->fccType, 4));
    qDebug() << "fccHandler" << streamHeader->fccHandler << QString(QByteArray((char*)&streamHeader->fccHandler, 4));
    qDebug() << "dwFlags" << streamHeader->dwFlags;
    qDebug() << "wPriority" << streamHeader->wPriority;
    qDebug() << "wLanguage" << streamHeader->wLanguage;
    qDebug() << "dwInitialFrames" << streamHeader->dwInitialFrames;
    qDebug() << "dwScale" << streamHeader->dwScale;
    qDebug() << "dwRate" << streamHeader->dwRate;
    qDebug() << "dwStart" << streamHeader->dwStart;
    qDebug() << "dwLength" << streamHeader->dwLength;
    qDebug() << "dwSuggestedBufferSize" << streamHeader->dwSuggestedBufferSize;
    qDebug() << "dwQuality" << streamHeader->dwQuality;
    qDebug() << "dwSampleSize" << streamHeader->dwSampleSize;
    qDebug() << "rcFrame.left" << streamHeader->rcFrame.left;
    qDebug() << "rcFrame.top" << streamHeader->rcFrame.top;
    qDebug() << "rcFrame.right" << streamHeader->rcFrame.right;
    qDebug() << "rcFrame.bottom" << streamHeader->rcFrame.bottom;
}

void ReadRAWAVI::BITMAPINFOHEADER(BITMAPINFOHEADERSTR *bitmapInfoHeader) {
    qDebug() << "biSize" << bitmapInfoHeader->biSize;
    qDebug() << "biWidth" << bitmapInfoHeader->biWidth;
    qDebug() << "biHeight" << bitmapInfoHeader->biHeight;
    qDebug() << "biPlanes" << bitmapInfoHeader->biPlanes;
    qDebug() << "biBitCount" << bitmapInfoHeader->biBitCount;
    qDebug() << "biCompression" << bitmapInfoHeader->biCompression << QString(QByteArray((char*)&bitmapInfoHeader->biCompression, 4));
    qDebug() << "biSizeImage" << bitmapInfoHeader->biSizeImage;
    qDebug() << "biXPelsPerMeter" << bitmapInfoHeader->biXPelsPerMeter;
    qDebug() << "biYPelsPerMeter" << bitmapInfoHeader->biYPelsPerMeter;
    qDebug() << "biClrUsed" << bitmapInfoHeader->biClrUsed;
    qDebug() << "biClrImportant" << bitmapInfoHeader->biClrImportant;
}

void ReadRAWAVI::WAVEFORMATEX(WAVEFORMATEXSTR *waveFormateEx) {
    qDebug() << "wFormatTag" << waveFormateEx->wFormatTag;
    qDebug() << "nChannels" << waveFormateEx->nChannels;
    qDebug() << "nSamplesPerSec" << waveFormateEx->nSamplesPerSec;
    qDebug() << "nAvgBytesPerSec" << waveFormateEx->nAvgBytesPerSec;
    qDebug() << "nBlockAlign" << waveFormateEx->nBlockAlign;
    qDebug() << "wBitsPerSample" << waveFormateEx->wBitsPerSample;
    qDebug() << "cbSize" << waveFormateEx->cbSize;
}

void ReadRAWAVI::decodeFrameI420(unsigned char *pkt, const int &pktSize) {
    //QThread::msleep(100);
    //--ReadRAWAVI::FIRST50Frames;
    //removeMe = QByteArray((char*)pkt, pktSize);

    QImage *frame = &frameBuffer[++bufferIndex%10];
    int rgbBytesPerLine(frame->bytesPerLine()), yPannelSize(bitmapInfoHeader->biWidth * bitmapInfoHeader->biHeight);
    unsigned char *y1 = pkt, *y2 = pkt + bitmapInfoHeader->biWidth, *u = pkt + yPannelSize, *rgbDataL1 = frame->bits(), *rgbDataL2 = frame->bits() + rgbBytesPerLine;
    unsigned char *y1End, *y2End = pkt + yPannelSize;
    unsigned char *v = yPannelSize / 4 + u;
    int R1, R2, G1, G2, B1, B2, v128, u128,vr, vg, vb, ug;
    do
    {
        y1End = y2;
        do
        {
            v128 = *v - 128;
            u128 = *u - 128;
            vr = (v128 * 1370705) / 1000000;
            vg = (v128 * 698001) / 1000000;
            ug = (u128 * 337633) / 1000000;
            vg -= ug;
            vb = (u128 * 1732446) /1000000;

            R1 = *y1 + vr;
            G1 = *y1 - vg;
            B1 = *y1 + vb;

            if (R1 < 0)
            { R1 = 0; }
            else if (R1 > 255 )
            { R1 = 255; }
            if (G1 < 0)
            { G1 = 0; }
            else if (G1 > 255)
            { G1 = 255; }
            if (B1 < 0)
            { B1 = 0; }
            else if (B1 > 255)
            { B1 = 255; }

            *(rgbDataL1++) = R1;
            *(rgbDataL1++) = G1;
            *(rgbDataL1++) = B1;

            ++y1;

            R1 = *y1 + vr;
            G1 = *y1 - vg;
            B1 = *y1 + vb;

            if (R1 < 0)
            { R1 = 0; }
            else if (R1 > 255 )
            { R1 = 255; }
            if (G1 < 0)
            { G1 = 0; }
            else if (G1 > 255)
            { G1 = 255; }
            if (B1 < 0)
            { B1 = 0; }
            else if (B1 > 255)
            { B1 = 255; }

            *(rgbDataL1++) = R1;
            *(rgbDataL1++) = G1;
            *(rgbDataL1++) = B1;

            ++y1;

            R2 = *y2 + vr;
            G2 = *y2 - vg;
            B2 = *y2 + vb;

            if (R2 < 0)
            { R2 = 0; }
            else if (R2 > 255 )
            { R2 = 255; }
            if (G2 < 0)
            { G2 = 0; }
            else if (G2 > 255)
            { G2 = 255; }
            if (B2 < 0)
            { B2 = 0; }
            else if (B2 > 255)
            { B2 = 255; }



            *(rgbDataL2++) = R2;
            *(rgbDataL2++) = G2;
            *(rgbDataL2++) = B2;
            ++y2;

            R2 = *y2 + vr;
            G2 = *y2 - vg;
            B2 = *y2 + vb;

            if (R2 < 0)
            { R2 = 0; }
            else if (R2 > 255 )
            { R2 = 255; }
            if (G2 < 0)
            { G2 = 0; }
            else if (G2 > 255)
            { G2 = 255; }
            if (B2 < 0)
            { B2 = 0; }
            else if (B2 > 255)
            { B2 = 255; }

            *(rgbDataL2++) = R2;
            *(rgbDataL2++) = G2;
            *(rgbDataL2++) = B2;
            ++y2;
            ++u;
            ++v;
        } while(y1 < y1End);
        y1 += bitmapInfoHeader->biWidth;
        y2 += bitmapInfoHeader->biWidth;
        rgbDataL1 += rgbBytesPerLine;
        rgbDataL2 += rgbBytesPerLine;
    } while(y2 < y2End);
    while (ReadRAWAVI::bufferSize >= 9) {
        QThread::msleep(40);
    }
    ++ReadRAWAVI::bufferSize;
    emit frameReady(frame);
}

// Clamp out of range values
#define CLAMP(t) (((t)>255)?255:(((t)<0)?0:(t)))
// Color space conversion for RGB
#define GET_R_FROM_YUV2(y, u, v) ((y+409*v+128)>>8)
#define GET_G_FROM_YUV2(y, u, v) ((y-100*u-208*v+128)>>8)
#define GET_B_FROM_YUV2(y, u, v) ((y+516*u+128)>>8)

void ReadRAWAVI::decodeFrameYUYV(unsigned char *pkt, const int &pktSize) {
    //QThread::msleep(100);
    //--ReadRAWAVI::FIRST50Frames;
    //removeMe = QByteArray((char*)pkt, pktSize);

    QImage *frame = &frameBuffer[++bufferIndex%10];

    unsigned char *data = frame->bits(), *endData(frame->bits() + frame->byteCount()), *src = pkt;
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

    } while(data < endData);

    while (ReadRAWAVI::bufferSize >= 9) {
        QThread::msleep(40);
    }
    ++ReadRAWAVI::bufferSize;
    emit frameReady(frame);
}


QImage ReadRAWAVI::getImage() {
    QImage ret(img->copy());
    delete img;
    return ret;
}

void ReadRAWAVI::openAVI(){
    readNextChunk();
}

void ReadRAWAVI::playAVI(){
    if(moviChunkSize > 0){
        for(bufferIndex = 0; bufferIndex < 10; ++bufferIndex)
            frameBuffer[bufferIndex]=QImage(bitmapInfoHeader->biWidth, bitmapInfoHeader->biHeight, QImage::Format_RGB888);
        bufferIndex = 0;
        ReadRAWAVI::bufferSize = 0;
        readMovi();
    }
}

const ReadRAWAVI::AVIMAINHEADERSTR *ReadRAWAVI::getMainHeader() const {
    return mainHeader;
}

const ReadRAWAVI::AVISTREAMHEADERSTR *ReadRAWAVI::getVideoStreamHeader() const {
    return streamHeaders[0];
}

const ReadRAWAVI::AVISTREAMHEADERSTR *ReadRAWAVI::getAudioStreamHeader() const {
    return streamHeaders[1];
}

const ReadRAWAVI::BITMAPINFOHEADERSTR *ReadRAWAVI::getBitmapInfoHeader() const {
    return bitmapInfoHeader;
}

const ReadRAWAVI::WAVEFORMATEXSTR *ReadRAWAVI::getWaveFormateHeader() const {
    return waveFormateEx;
}
