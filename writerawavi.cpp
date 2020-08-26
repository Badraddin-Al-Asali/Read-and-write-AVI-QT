#include "writerawavi.h"

WriteRawAVI::WriteRawAVI(QObject *parent): QObject(parent)
{

}

void WriteRawAVI::openFile(const QString &filename){
    outputFile.setFileName(filename);
    if(!outputFile.open(QIODevice::WriteOnly)) {
        qDebug() << outputFile.errorString();
        return;
    }

}

void WriteRawAVI::createAVI(const int &fileSize) {
    outputFile.write((char*)ReadRAWAVI::RIFF, 4);
    seekBackPositions[0] = outputFile.pos();
    outputFile.write((char*)&fileSize, 4); // Seek Back
    outputFile.write((char*)ReadRAWAVI::RIFF_Childs[0], 4);
}

void WriteRawAVI::createList(const int &size) {
    outputFile.write((char*)ReadRAWAVI::RIFF_Childs[1], 4);
    outputFile.write((char*)&size, 4);
}

void WriteRawAVI::createStreamHeaderList() {
    outputFile.write((char*)ReadRAWAVI::RIFF_Childs[5], 4);
}

void WriteRawAVI::createStreamFormate(const int &size) {
    outputFile.write((char*)ReadRAWAVI::RIFF_Childs[7], 4);
    outputFile.write((char*)&size, 4);
}

void WriteRawAVI::createMOVI() {
    outputFile.write((char*)ReadRAWAVI::RIFF_Childs[9], 4);
    videoFrameWritten = 0;
    audioSamplesWritten = 0;
    audioSamplesWrittenCounter = 0;
}

void WriteRawAVI::pushStream(const ReadRAWAVI::STREAMSTR &stream, const int &size) {
    outputFile.write((char*)&stream.streamNumber, 2); // Stream Header List
    outputFile.write((char*)&stream.streamType, 2); // Stream Header List
    outputFile.write((char*)&size, 4);
}

int WriteRawAVI::calculateHeaderListSize(const ReadRAWAVI::AVISTREAMHEADERSTR *videoStreamHeader, const ReadRAWAVI::AVISTREAMHEADERSTR *audioStreamHeader, const ReadRAWAVI::WAVEFORMATEXSTR *waveFormateEx) {
    int returnValue(68); //Main Header(64) + hdrl(4) tag
    if(videoStreamHeader != Q_NULLPTR) {
        returnValue += 8;  // Sub List Size
        returnValue += 4;  // Stream Header List tag
        returnValue += 64; // Stream Header Size
        returnValue += 8;  // Stream Formate Tag + Size
        returnValue += 40; // Bit map Info Header Size
    }
    if(audioStreamHeader != Q_NULLPTR) {
        returnValue += 8;  // Sub List Size
        returnValue += 4;  // Stream Header List tag
        returnValue += 64; // Stream Header Size
        returnValue += 8;  // Stream Formate Tag + Size
        if(waveFormateEx != Q_NULLPTR) {
            if(waveFormateEx->wFormatTag != ReadRAWAVI::WAVEFORMATETYPE::PCM){
                returnValue += 18;
            }
            else{
                returnValue += 16;
            }
        }
        else {
            return -1;
        }
    }
    return returnValue;
}

void WriteRawAVI::writeHeader(const AVIALLHEADERS &header) {
    if(outputFile.isOpen()) {
        createAVI(0/*610784*/ /*610808*/ /*610872*/); //Total File Size 4 + 4 + 4
            headerListSize = calculateHeaderListSize(header.videoStreamHeader, header.audioStreamHeader, header.waveFormateEx);
            if(headerListSize == -1) {
                qDebug() << "waveFormateEx" << NULL;
                return;
            }
            if(header.bitMapInfoHeader == Q_NULLPTR) {
                qDebug() << "bitMapInfoHeader" << NULL;
                return;
            }
            streams[0].streamNumber = -1;
            streams[1].streamNumber = -1;
            //MAIN HEADER
            createList(headerListSize); // 8
                writeMainHeader(header.mainHeader); // 64 + hdrl(4)
                ReadRAWAVI::STREAMSTR aStream;
                //VIDEO HEADER Total Size 124
                if(header.videoStreamHeader != Q_NULLPTR) {
                    createList(116); // 8

                        createStreamHeaderList(); // 4
                            writeStreamHeader(header.videoStreamHeader); // 64
                            seekBackPositions[2] = tempSeekPostion;
                        createStreamFormate(40); // 8
                            writeBitMapInfoHeader(header.bitMapInfoHeader); // 40
                            //videoFrameByteCount = header.bitMapInfoHeader->biSizeImage; // ((width * height) / 2) + (width * height)
                            videoFrameByteCount = header.bitMapInfoHeader->biWidth * header.bitMapInfoHeader->biHeight * 3;
                            aStream.streamNumber = 0x3030;
                            if(header.bitMapInfoHeader->biCompression != 0x00)
                                aStream.streamType = ReadRAWAVI::STREAMPKTTYPE::CPMPRESSEDVIDEO;
                            else
                                aStream.streamType = ReadRAWAVI::STREAMPKTTYPE::UNCOMPRESSEDVIDEO;
                            streams[0] = aStream;
                            //encodedFrame = QByteArray(header.bitMapInfoHeader->biSizeImage, 0);
                }

                //AUDIO HEADER Total Size 100 bytes
                if(header.audioStreamHeader != Q_NULLPTR) {
                    int audioStreamAdd(0);
                    if(header.waveFormateEx->wFormatTag != ReadRAWAVI::WAVEFORMATETYPE::PCM) {
                        audioStreamAdd += 2;
                    }
                    int audioHeaderListSize = 92 + audioStreamAdd;
                    createList(audioHeaderListSize); // 8
                        createStreamHeaderList(); // 4
                            writeStreamHeader(header.audioStreamHeader); // 64
                            seekBackPositions[3] = tempSeekPostion;
                        createStreamFormate(16 + audioStreamAdd); // 8
                            writeWaveFormateEx(header.waveFormateEx); // 16
                            aStream.streamNumber = 0x3130;
                            aStream.streamType = ReadRAWAVI::STREAMPKTTYPE::AUDIODATA;
                            streams[1] = aStream;
                }

            //DATA
            const int MoviListSize(0);
            createList(MoviListSize); // 8 Seek Back to adjust size
            seekBackPositions[4] = outputFile.pos() - sizeof(MoviListSize);
                createMOVI(); // 4
    }
    else {
        qDebug() << "writeHeader outputFile \'" << outputFile.fileName() << "\' is not opened";
    }

}

void WriteRawAVI::writeMainHeader(const ReadRAWAVI::AVIMAINHEADERSTR *mainHeader) {
    outputFile.write((char*)ReadRAWAVI::RIFF_Childs[3], 4); //Header List hdrl
    outputFile.write((char*)&mainHeader->fcc, sizeof(mainHeader->fcc));
    outputFile.write((char*)&mainHeader->cb, sizeof(mainHeader->cb));
    outputFile.write((char*)&mainHeader->dwMicroSecPerFrame, sizeof(mainHeader->dwMicroSecPerFrame));
    outputFile.write((char*)&mainHeader->dwMaxBytesPerSec, sizeof(mainHeader->dwMaxBytesPerSec));
    outputFile.write((char*)&mainHeader->dwPaddingGranularity, sizeof(mainHeader->dwPaddingGranularity));
    outputFile.write((char*)&mainHeader->dwFlags, sizeof(mainHeader->dwFlags));
    //int flags(0);
    //if(mainHeader->dwStreams > 1)
    //    flags = AVIMAINHEADERFLAGS::ISINTERLEAVED;
    //outputFile.write((char*)&flags, sizeof(flags));
    seekBackPositions[1] = outputFile.pos();
    outputFile.write((char*)&mainHeader->dwTotalFrames, sizeof(mainHeader->dwTotalFrames)); // Seek Back
    outputFile.write((char*)&mainHeader->dwInitialFrames, sizeof(mainHeader->dwInitialFrames));
    outputFile.write((char*)&mainHeader->dwStreams, sizeof(mainHeader->dwStreams));
    outputFile.write((char*)&mainHeader->dwSuggestedBufferSize, sizeof(mainHeader->dwSuggestedBufferSize));
    outputFile.write((char*)&mainHeader->dwWidth, sizeof(mainHeader->dwWidth));
    outputFile.write((char*)&mainHeader->dwHeight, sizeof(mainHeader->dwHeight));
    outputFile.write((char*)&mainHeader->dwReserved[0], sizeof(mainHeader->dwReserved[0]));
    outputFile.write((char*)&mainHeader->dwReserved[1], sizeof(mainHeader->dwReserved[1]));
    outputFile.write((char*)&mainHeader->dwReserved[2], sizeof(mainHeader->dwReserved[2]));
    outputFile.write((char*)&mainHeader->dwReserved[3], sizeof(mainHeader->dwReserved[3]));
}

void WriteRawAVI::writeStreamHeader(const ReadRAWAVI::AVISTREAMHEADERSTR *streamHeader) {
    outputFile.write((char*)&streamHeader->fcc, sizeof(streamHeader->fcc));
    outputFile.write((char*)&streamHeader->cb, sizeof(streamHeader->cb));
    outputFile.write((char*)&streamHeader->fccType, sizeof(streamHeader->fccType));
    /*if(streamHeader->fccType == ReadRAWAVI::STREAMTYPE::VIDEO) {
        int rgbFlag(Compression::BI_RGB);
        outputFile.write((char*)&rgbFlag, sizeof(rgbFlag));
    }
    else*/
        outputFile.write((char*)&streamHeader->fccHandler, sizeof(streamHeader->fccHandler));
    outputFile.write((char*)&streamHeader->dwFlags, sizeof(streamHeader->dwFlags));
    outputFile.write((char*)&streamHeader->wPriority, sizeof(streamHeader->wPriority));
    outputFile.write((char*)&streamHeader->wLanguage, sizeof(streamHeader->wLanguage));
    outputFile.write((char*)&streamHeader->dwInitialFrames, sizeof(streamHeader->dwInitialFrames));
    outputFile.write((char*)&streamHeader->dwScale, sizeof(streamHeader->dwScale));
    outputFile.write((char*)&streamHeader->dwRate, sizeof(streamHeader->dwRate));
    outputFile.write((char*)&streamHeader->dwStart, sizeof(streamHeader->dwStart));
    tempSeekPostion = outputFile.pos();
    outputFile.write((char*)&streamHeader->dwLength, sizeof(streamHeader->dwLength)); // Seek Back
    outputFile.write((char*)&streamHeader->dwSuggestedBufferSize, sizeof(streamHeader->dwSuggestedBufferSize));
    outputFile.write((char*)&streamHeader->dwQuality, sizeof(streamHeader->dwQuality)); //Set This to -1 for drivers to use the default quality value.
    outputFile.write((char*)&streamHeader->dwSampleSize, sizeof(streamHeader->dwSampleSize));
    outputFile.write((char*)&streamHeader->rcFrame.left, sizeof(streamHeader->rcFrame.left));
    outputFile.write((char*)&streamHeader->rcFrame.top, sizeof(streamHeader->rcFrame.top));
    outputFile.write((char*)&streamHeader->rcFrame.right, sizeof(streamHeader->rcFrame.right));
    outputFile.write((char*)&streamHeader->rcFrame.bottom, sizeof(streamHeader->rcFrame.bottom));
}

void WriteRawAVI::writeBitMapInfoHeader(const ReadRAWAVI::BITMAPINFOHEADERSTR *bitMapInfoHeader) {
    outputFile.write((char*)&bitMapInfoHeader->biSize, sizeof(bitMapInfoHeader->biSize));
    outputFile.write((char*)&bitMapInfoHeader->biWidth, sizeof(bitMapInfoHeader->biWidth));
    outputFile.write((char*)&bitMapInfoHeader->biHeight, sizeof(bitMapInfoHeader->biHeight));
    outputFile.write((char*)&bitMapInfoHeader->biPlanes, sizeof(bitMapInfoHeader->biPlanes));
    //short bitCount(24);
    outputFile.write((char*)&bitMapInfoHeader->biBitCount, sizeof(bitMapInfoHeader->biBitCount));
    //outputFile.write((char*)&bitCount, sizeof(bitCount));
    //int rgbFlag(Compression::BI_RGB);
    //outputFile.write((char*)&rgbFlag, sizeof(rgbFlag));
    outputFile.write((char*)&bitMapInfoHeader->biCompression, sizeof(bitMapInfoHeader->biCompression));
    outputFile.write((char*)&bitMapInfoHeader->biSizeImage, sizeof(bitMapInfoHeader->biSizeImage));
    //int imageSize(bitMapInfoHeader->biWidth * bitMapInfoHeader->biHeight * 3);
    //outputFile.write((char*)&imageSize, sizeof(imageSize));
    outputFile.write((char*)&bitMapInfoHeader->biXPelsPerMeter, sizeof(bitMapInfoHeader->biXPelsPerMeter));
    outputFile.write((char*)&bitMapInfoHeader->biYPelsPerMeter, sizeof(bitMapInfoHeader->biYPelsPerMeter));
    outputFile.write((char*)&bitMapInfoHeader->biClrUsed, sizeof(bitMapInfoHeader->biClrUsed));
    outputFile.write((char*)&bitMapInfoHeader->biClrImportant, sizeof(bitMapInfoHeader->biClrImportant));
}

void WriteRawAVI::writeWaveFormateEx(const ReadRAWAVI::WAVEFORMATEXSTR *waveFormateEx) {
    outputFile.write((char*)&waveFormateEx->wFormatTag, sizeof(waveFormateEx->wFormatTag));
    outputFile.write((char*)&waveFormateEx->nChannels, sizeof(waveFormateEx->nChannels));
    outputFile.write((char*)&waveFormateEx->nSamplesPerSec, sizeof(waveFormateEx->nSamplesPerSec));
    outputFile.write((char*)&waveFormateEx->nAvgBytesPerSec, sizeof(waveFormateEx->nAvgBytesPerSec)); // nSamplesPerSec * nChannels * (wBitsPerSample / 8)
    outputFile.write((char*)&waveFormateEx->nBlockAlign, sizeof(waveFormateEx->nBlockAlign)); // nChannels * (wBitsPerSample / 8)
    outputFile.write((char*)&waveFormateEx->wBitsPerSample, sizeof(waveFormateEx->wBitsPerSample));
    //outputFile.write((char*)&waveFormateEx->cbSize, sizeof(waveFormateEx->cbSize));
}

void WriteRawAVI::encodeFrameI420(const QImage &inFrame) {
    const unsigned char *rgb(inFrame.bits());
    unsigned char *destination((unsigned char*)encodedFrame.data());
    int width(inFrame.width()), height(inFrame.height());

    size_t image_size = width * height;
    size_t upos = image_size;
    size_t vpos = upos + upos / 4;
    size_t i = 0;

    for(size_t line = 0; line < height; ++line )
    {
        if(!(line % 2))
        {
            for( size_t x = 0; x < width; x += 2 )
            {
                uint8_t r = rgb[3 * i];
                uint8_t g = rgb[3 * i + 1];
                uint8_t b = rgb[3 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;

                destination[upos++] = ((-38*r + -74*g + 112*b) >> 8) + 128;
                destination[vpos++] = ((112*r + -94*g + -18*b) >> 8) + 128;

                r = rgb[3 * i];
                g = rgb[3 * i + 1];
                b = rgb[3 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        }
        else
        {
            for( size_t x = 0; x < width; x += 1 )
            {
                uint8_t r = rgb[3 * i];
                uint8_t g = rgb[3 * i + 1];
                uint8_t b = rgb[3 * i + 2];

                destination[i++] = ((66*r + 129*g + 25*b) >> 8) + 16;
            }
        }
    }
}

void WriteRawAVI::reverseImageTable(QImage *frame) {
    int line(frame->bytesPerLine());
    int lineX2((line*2));
    unsigned char *start(frame->bits());
    unsigned char *endX(start + line);
    unsigned char *end(start + line * (frame->height() - 1));
    do {
        do{
            std::swap(*start, end[2]);
            ++start;
            std::swap(*start, end[1]);
            ++start;
            std::swap(*start, *end);
            ++start;
            end += 3;
        }while (start < endX);
        endX += line;
        end -= lineX2;
    }while (start < end);
}

void WriteRawAVI::writeVideoFrame(QImage *frame) {
    //if(outputFile.isOpen()) {
        //encodeFrameI420(frame);

        pushStream(streams[0], videoFrameByteCount);
        //outputFile.write(encodedFrame);
        reverseImageTable(frame);
        outputFile.write((char*)frame->bits(), videoFrameByteCount);
        --ReadRAWAVI::bufferSize;
        ++videoFrameWritten;
    //}
    //else{
        //qDebug() << "writeVideoFrame outputFile \'" << outputFile.fileName() << "\' is not opened";
    //}
}

void WriteRawAVI::writeVideoFrameWeb(QImage frame) {
    pushStream(streams[0], videoFrameByteCount);
    reverseImageTable(&frame);
    outputFile.write((char*)frame.bits(), videoFrameByteCount);
    ++videoFrameWritten;
}

void WriteRawAVI::writeEncodedFrame(const QByteArray frame) {
    if(outputFile.isOpen()){
        pushStream(streams[0], frame.size()); // 8
        outputFile.write(frame); // 610560 1008
        ++videoFrameWritten;
    }
    else{
        qDebug() << "writeEncodedFrame outputFile \'" << outputFile.fileName() << "\' is not opened";
    }
}

void WriteRawAVI::writeAudioFrame(const QByteArray sample) {
    if(outputFile.isOpen()){
        int size(sample.size());
        pushStream(streams[1], sample.size());
        outputFile.write(sample);
        //qDebug() << "Writting audio";
        audioSamplesWritten += size;
    }
    else{
        qDebug() << "writeAudioFrame outputFile \'" << outputFile.fileName() << "\' is not opened";
    }
}

void WriteRawAVI::writeOldIndexChunk() {
    AVIOLDINDEX index;
    index.fcc = OLDINDEXCHUNK;
    index.cb = 16;
    index.dwChunkId = *((int*)&streams[0]);
    index.dwFlags = AVIOLDINDEXFLAGS::LIST;
    index.dwOffset = 4;
    index.dwSize = 1008;
    outputFile.write((char*)&index.fcc, sizeof(index.fcc));
    outputFile.write((char*)&index.cb, sizeof(index.cb));
    outputFile.write((char*)&index.dwChunkId, sizeof(index.dwChunkId));
    outputFile.write((char*)&index.dwFlags, sizeof(index.dwFlags));
    outputFile.write((char*)&index.dwOffset, sizeof(index.dwOffset));
    outputFile.write((char*)&index.dwSize, sizeof(index.dwSize));
}

void WriteRawAVI::writeSize() {
    int totalMoviListSize(4)/* Movi Tag*/, totalAVISize(headerListSize + 4/*AVI Tag*/ + 8/*Movi List Tag*/ + 8 /*Header List Tag*/);

    totalMoviListSize += videoFrameWritten * videoFrameByteCount; //Video Frames
    totalMoviListSize += videoFrameWritten * 2 * sizeof(int); // Video Stream Tags

    totalMoviListSize += audioSamplesWritten;
    totalMoviListSize += audioSamplesWrittenCounter * 2 * sizeof(int); // Audio Stream Tags

    totalAVISize += totalMoviListSize;

    outputFile.seek(seekBackPositions[4]);
    outputFile.write((char*)&totalMoviListSize, sizeof(totalMoviListSize));
    if(streams[1].streamNumber != -1) {
        outputFile.seek(seekBackPositions[3]);
        outputFile.write((char*)&audioSamplesWritten, sizeof(audioSamplesWritten));
        qDebug() << "audioSamplesWritten" << audioSamplesWritten;
    }
    if(streams[0].streamNumber != -1) {
        outputFile.seek(seekBackPositions[2]);
        outputFile.write((char*)&videoFrameWritten, sizeof(videoFrameWritten));
        outputFile.seek(seekBackPositions[1]);
        outputFile.write((char*)&videoFrameWritten, sizeof(videoFrameWritten));
        qDebug() << "videoFrameWritten" << videoFrameWritten;
    }
    outputFile.seek(seekBackPositions[0]);
    outputFile.write((char*)&totalAVISize, sizeof(totalAVISize));
}

void WriteRawAVI::close() {
    if(outputFile.isOpen()) {
        //writeOldIndexChunk(); // 24
        writeSize();
        outputFile.close();
    }
    else{
        qDebug() << "close outputFile \'" << outputFile.fileName() << "\' is not opened";
    }
}
