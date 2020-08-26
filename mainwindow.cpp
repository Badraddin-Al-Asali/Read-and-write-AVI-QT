#include "mainwindow.h"
#include "ui_mainwindow.h"
int cccc(0);
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    on_nextFrameButton_hold_active(false)
{
    ui->setupUi(this);
    ui->display->setScaledContents(true);
    //ui->display->setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored);
    model = new QStringListModel(this);
    model->setStringList(QStringList());
    ui->output->setModel(model);
    msgBoxSaved.setText("OutputSaved");
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::on_nextFrameButton_hold);

    connect(&rawAVI, &ReadRAWAVI::frameReady, &myVideoPlay, &MyVideoPlay::on_frame_Ready);
    connect(&myVideoPlay, &MyVideoPlay::frameDisplayReady, this, &MainWindow::on_frameDisplay_Ready);
    connect(this, &MainWindow::openVideo, &rawAVI, &ReadRAWAVI::openAVI);
    connect(&rawAVI, &ReadRAWAVI::videoReady, this, &MainWindow::on_videoReady);
    connect(this, &MainWindow::playVideo, &rawAVI, &ReadRAWAVI::playAVI);
    connect(&rawAVI, &ReadRAWAVI::audioSampleReady, &myAudioPlay, &MyAudioPlay::on_audioSample_Ready);
    connect(&myAudioPlay, &MyAudioPlay::saveAudioSample, &writeRawAVI, &WriteRawAVI::writeAudioFrame);
    connect(this, &MainWindow::saveEncodedFrame, &writeRawAVI, &WriteRawAVI::writeEncodedFrame);
    connect(this, &MainWindow::saveFrame, &writeRawAVI, &WriteRawAVI::writeVideoFrame);
    connect(this, &MainWindow::saveFrameWeb, &writeRawAVI, &WriteRawAVI::writeVideoFrameWeb);
    connect(&myMicrophne, &MyMicrophone::saveAudioSample, &writeRawAVI, &WriteRawAVI::writeAudioFrame);
    rawAVI.moveToThread(&aviThread);
    myAudioPlay.moveToThread(&audioPlayThread);
    myVideoPlay.moveToThread(&videoPlayThread);
    aviThread.start(QThread::Priority::LowPriority);
    audioPlayThread.start(QThread::Priority::LowPriority);
    videoPlayThread.start(QThread::Priority::LowPriority);
    //saveThread.start(QThread::Priority::LowPriority);
    writeRawAVI.openFile("C:\\Users\\Andy BadMan\\Desktop\\mp4\\myAVIWriteFile1VideoFrame.avi");
    //writeRawAVI.moveToThread(&saveThread);

    // webcam
    const QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    camera = new QCamera(cameras.at(0));
    camera->load();
    auto supportedRes = camera->supportedViewfinderResolutions();
    int bestRes(0), bestArea(0);
    for(int i(0); i < supportedRes.size(); ++i){
        int area = supportedRes.at(i).width() * supportedRes.at(i).height();
        if(area > bestArea){
            bestRes = i;
            bestArea = area;
        }
        qDebug() << supportedRes.at(i);
    }
    qDebug() << supportedRes.at(bestRes);
    auto supportedRate = camera->supportedViewfinderFrameRateRanges();
    int bestRate(0), maximumRate(0);
    for(int i(0); i < supportedRate.size(); ++i){
        int rateHigh = supportedRate.at(i).maximumFrameRate;
        if(maximumRate < rateHigh) {
            bestRate = i;
            maximumRate = rateHigh;
        }
        qDebug() << supportedRate.at(i).maximumFrameRate << supportedRate.at(i).minimumFrameRate;
    }
    qDebug() << supportedRate.at(bestRate).maximumFrameRate << supportedRate.at(bestRate).minimumFrameRate;

    auto supportedformates = camera->supportedViewfinderPixelFormats();
    QVideoFrame::PixelFormat pixelFormate;
    for(int i(0); i < supportedformates.size(); ++i) {
        pixelFormate = supportedformates.at(i);
        qDebug() << pixelFormate;
    }

    cameraSettings.setMaximumFrameRate(supportedRate.at(bestRate).maximumFrameRate);
    cameraSettings.setMinimumFrameRate(supportedRate.at(bestRate).minimumFrameRate);
    cameraSettings.setResolution(supportedRes.at(bestRes));
    cameraSettings.setPixelFormat(pixelFormate);
    camera->setViewfinderSettings(cameraSettings);
    connect(&webcam, &MyWebcam::imageReady, this, &MainWindow::on_frameDisplayFromCamera_Ready);
    connect(&webcam, &MyWebcam::imageReadyYUV, &writeRawAVI, &WriteRawAVI::writeEncodedFrame);
    camera->setViewfinder(&webcam);
    //connect(&probe, &QVideoProbe::videoFrameProbed, &webcam, &MyWebcam::processFrame);
    //qDebug() << probe.setSource(camera); // Returns true, hopefully.
    myMicrophne.setup();
    //setWebcamSave();
    //camera->start();
    //myMicrophne.startListening();
    cccc = 0;
}

MainWindow::~MainWindow()
{
    disconnect(timer, &QTimer::timeout, this, &MainWindow::on_nextFrameButton_hold);
    disconnect(&rawAVI, &ReadRAWAVI::frameReady, &myVideoPlay, &MyVideoPlay::on_frame_Ready);
    disconnect(&myVideoPlay, &MyVideoPlay::frameDisplayReady, this, &MainWindow::on_frameDisplay_Ready);
    disconnect(this, &MainWindow::openVideo, &rawAVI, &ReadRAWAVI::openAVI);
    disconnect(&rawAVI, &ReadRAWAVI::videoReady, this, &MainWindow::on_videoReady);
    disconnect(this, &MainWindow::playVideo, &rawAVI, &ReadRAWAVI::playAVI);
    disconnect(&rawAVI, &ReadRAWAVI::audioSampleReady, &myAudioPlay, &MyAudioPlay::on_audioSample_Ready);
    disconnect(&myAudioPlay, &MyAudioPlay::saveAudioSample, &writeRawAVI, &WriteRawAVI::writeAudioFrame);
    disconnect(this, &MainWindow::saveEncodedFrame, &writeRawAVI, &WriteRawAVI::writeEncodedFrame);
    disconnect(this, &MainWindow::saveFrame, &writeRawAVI, &WriteRawAVI::writeVideoFrame);
    disconnect(this, &MainWindow::saveFrameWeb, &writeRawAVI, &WriteRawAVI::writeVideoFrameWeb);
    disconnect(&webcam, &MyWebcam::imageReady, this, &MainWindow::on_frameDisplayFromCamera_Ready);
    disconnect(&webcam, &MyWebcam::imageReadyYUV, &writeRawAVI, &WriteRawAVI::writeEncodedFrame);
    disconnect(&probe, &QVideoProbe::videoFrameProbed, &webcam, &MyWebcam::processFrame);
    disconnect(&myMicrophne, &MyMicrophone::saveAudioSample, &writeRawAVI, &WriteRawAVI::writeAudioFrame);

    delete ui;
    delete timer;
    delete model;
    delete camera;
}

void MainWindow::on_nextFrameButton_clicked()
{
    int rowCount(model->rowCount());
    model->insertRow(rowCount);
    QModelIndex index = model->index(rowCount);
    model->setData(index, rawAVI.readNextByte());
    ui->output->scrollToBottom();
}

void MainWindow::on_saveOutput_clicked()
{
    ui->saveOutput->setEnabled(false);
    ui->nextFrameButton->setEnabled(false);
    QFile outFile("C:\\Users\\Andy BadMan\\Desktop\\mp4\\myoutFile.txt");
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&outFile);
        int rowCount(model->rowCount());
        for(int i(0); i < rowCount; ++i)
            out << model->data(model->index(i)).toString() << "\n";
        outFile.close();
    }
    msgBoxSaved.exec();
    ui->saveOutput->setEnabled(true);
    ui->nextFrameButton->setEnabled(true);
}

void MainWindow::on_nextFrameButton_pressed()
{
    on_nextFrameButton_hold_active = true;
    timer->stop();
    timer->start(500);
}

void MainWindow::on_nextFrameButton_hold()
{
    if(on_nextFrameButton_hold_active) {
        timer->stop();
        on_nextFrameButton_clicked();
        timer->start(50);
    }
}

void MainWindow::on_nextFrameButton_released()
{
    on_nextFrameButton_hold_active = false;
    timer->stop();
}

void MainWindow::on_readNBytes_clicked()
{
    int bytes = ui->numberOfBytes->value();
    for(int i(0); i < bytes; ++i){
        int rowCount(model->rowCount());
        model->insertRow(rowCount);
        QModelIndex index = model->index(rowCount);
        model->setData(index, rawAVI.readNextByte());
    }
    ui->output->scrollToBottom();
}

void MainWindow::on_readChunk_clicked()
{
    //rawAVI.readNextChunk();
    //setAudioDevice();
    emit openVideo();
}

void MainWindow::on_videoReady(){
    myAudioPlay.setAudioDevice(rawAVI.getWaveFormateHeader());
    myVideoPlay.setDelay(rawAVI.getMainHeader()->dwMicroSecPerFrame / 1000);
    /*ReadRAWAVI::AVIMAINHEADERSTR mainHeader = *rawAVI.getMainHeader();
    ReadRAWAVI::AVISTREAMHEADERSTR videoStream = *rawAVI.getVideoStreamHeader();
    ReadRAWAVI::BITMAPINFOHEADERSTR bitMapInfo = *rawAVI.getBitmapInfoHeader();
    mainHeader.dwMaxBytesPerSec = mainHeader.dwWidth * mainHeader.dwHeight * 3 * 25;
    mainHeader.dwStreams = 1;
    mainHeader.dwSuggestedBufferSize = mainHeader.dwWidth * mainHeader.dwHeight * 3;

    videoStream.fccHandler = 0;
    videoStream.dwSuggestedBufferSize = mainHeader.dwSuggestedBufferSize;

    bitMapInfo.biBitCount = 24;
    bitMapInfo.biCompression = 0;
    bitMapInfo.biSizeImage = mainHeader.dwSuggestedBufferSize;

    const WriteRawAVI::AVIALLHEADERS header {
        &mainHeader,
        &videoStream,
        rawAVI.getAudioStreamHeader(),
        &bitMapInfo,
        rawAVI.getWaveFormateHeader()
    };
    writeRawAVI.writeHeader(header);*/
    emit playVideo();
}

void MainWindow::on_showFrame_clicked()
{
    QImage img(rawAVI.getImage());
    ui->display->setPixmap(QPixmap::fromImage(img));
    //ui->display->display(img);
}
void MainWindow::on_frameDisplay_Ready(QImage *frame) {
    ui->display->setPixmap(QPixmap::fromImage(*frame));
    //if(ReadRAWAVI::FIRST50Frames > 0) {
        //emit saveEncodedFrame(rawAVI.removeMe);
        //emit saveFrame(frame);
    --ReadRAWAVI::bufferSize;
    //++cccc;
    //if(cccc == 30)
    //    on_StopSave_clicked();
        //writeRawAVI.writeEncodedFrame(rawAVI.removeMe);
    //}
    //else {
    //    writeRawAVI.close();
    //}

    //ui->display->display(*frame);
}
void MainWindow::on_frameDisplayFromCamera_Ready(QImage frame) {
    //frame = frame.scaled(848, 480, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    emit saveFrameWeb(frame);
    ui->display->setPixmap(QPixmap::fromImage(frame));
    ++cccc;
    if(cccc == 120)
        on_StopSave_clicked();
}
void MainWindow::on_StopSave_clicked()
{
    camera->stop();
    while (camera->status() == QCamera::ActiveStatus) {
        qApp->processEvents();
        QThread::msleep(40);
        qApp->processEvents();
    }
    myMicrophne.stopListening();
    writeRawAVI.close();
    qDebug() << "AVI Closed";
}

int convertFCC2Int(const char* fcc) {
    const int *ptr = (const int*)fcc;
    return *ptr;
}

void MainWindow::setWebcamSave() {

    WriteRawAVI::AVIALLHEADERS aviHeader;
    ReadRAWAVI::AVIMAINHEADERSTR mainHeader;
    ReadRAWAVI::AVISTREAMHEADERSTR videoStream;
    ReadRAWAVI::AVISTREAMHEADERSTR audioStream;
    ReadRAWAVI::BITMAPINFOHEADERSTR bitMapInfo;
    ReadRAWAVI::WAVEFORMATEXSTR wave;
    int width(cameraSettings.resolution().width()), height(cameraSettings.resolution().height()), area(0);
    qreal fsp(cameraSettings.maximumFrameRate());
    auto formate = myMicrophne.getAudioFormate();
    //fsp = 25;
    area = width * height;
    mainHeader.fcc = convertFCC2Int("avih");
    qDebug() << QString(QByteArray((char*)&mainHeader.fcc, 4));
    mainHeader.cb = 56;
    mainHeader.dwMicroSecPerFrame = 1000000 / fsp;
    //mainHeader.dwMaxBytesPerSec = (area + area) * cameraSettings.maximumFrameRate();//(Image Byte Count (width[100] * height[100] * channels[2])[20,000] * frames per sec[10])[200,000] + (audio sample rate[8000] * audio channels[2] * audio byte size[2])[32,000]{Total=232,000}
    mainHeader.dwMaxBytesPerSec = (area * 2 * fsp) + (formate->sampleRate() * formate->channelCount() * (formate->sampleSize() / 8));
    mainHeader.dwPaddingGranularity = 0;
    mainHeader.dwFlags = 0;
    mainHeader.dwTotalFrames = 0;
    mainHeader.dwInitialFrames = 0;
    //mainHeader.dwSuggestedBufferSize = area + area;//Set to Image Byte Count
    mainHeader.dwSuggestedBufferSize = area * 2;
    mainHeader.dwStreams = 2;
    mainHeader.dwWidth = width;
    mainHeader.dwHeight = height;
    mainHeader.dwReserved[0] = 0;
    mainHeader.dwReserved[1] = 0;
    mainHeader.dwReserved[2] = 0;
    mainHeader.dwReserved[3] = 0;

    aviHeader.mainHeader = &mainHeader;

    videoStream.fcc = convertFCC2Int("strh");
    videoStream.cb = 56;
    videoStream.fccType = convertFCC2Int("vids");
    //videoStream.fccHandler = 0;
    videoStream.fccHandler = convertFCC2Int("YUY2"); //YUYV = VYUY
    videoStream.dwFlags = 0;
    videoStream.wPriority = 0;
    videoStream.wLanguage = 0;
    videoStream.dwInitialFrames = 0;
    videoStream.dwScale = 1;
    videoStream.dwRate = fsp;
    videoStream.dwStart = 0;
    videoStream.dwLength = 0;
    //videoStream.dwSuggestedBufferSize = area + area;
    videoStream.dwSuggestedBufferSize = area * 2 * fsp;
    videoStream.dwQuality = -1;
    videoStream.dwSampleSize = 0;
    videoStream.rcFrame.left = 0;
    videoStream.rcFrame.top = 0;
    videoStream.rcFrame.right = width;
    videoStream.rcFrame.bottom = height;

    aviHeader.videoStreamHeader = &videoStream;

    bitMapInfo.biSize = 40;
    bitMapInfo.biWidth = width;
    bitMapInfo.biHeight = height;
    bitMapInfo.biPlanes = 1;
    bitMapInfo.biBitCount = 16; //YUYV Bit count os 16, I420 12
    //bitMapInfo.biBitCount = 24;
    bitMapInfo.biCompression = 0;
    bitMapInfo.biCompression = convertFCC2Int("YUY2");
    //bitMapInfo.biSizeImage = area + area;
    bitMapInfo.biSizeImage = area * 2;
    bitMapInfo.biXPelsPerMeter = 0;
    bitMapInfo.biYPelsPerMeter = 0;
    bitMapInfo.biClrUsed = 0;
    bitMapInfo.biClrImportant = 0;

    aviHeader.bitMapInfoHeader = &bitMapInfo;

    wave.wFormatTag = ReadRAWAVI::WAVEFORMATETYPE::PCM; //PCM
    wave.nChannels = formate->channelCount();
    wave.nSamplesPerSec = formate->sampleRate();
    wave.wBitsPerSample = formate->sampleSize();
    wave.nAvgBytesPerSec = wave.nSamplesPerSec * wave.nChannels * (wave.wBitsPerSample / 8);
    wave.nBlockAlign = (wave.nChannels * wave.wBitsPerSample) / 8;
    wave.cbSize = 0;

    audioStream.fcc = convertFCC2Int("strh");
    audioStream.cb = 56;
    audioStream.fccType = convertFCC2Int("auds");
    audioStream.fccHandler = ReadRAWAVI::WAVEFORMATETYPE::PCM; //PCM
    audioStream.dwFlags = 0;
    audioStream.wPriority = 0;
    audioStream.wLanguage = 0;
    audioStream.dwInitialFrames = 0;
    audioStream.dwScale = 1;
    audioStream.dwRate = formate->sampleRate();
    //formate->framesForBytes()
    audioStream.dwStart = 0;
    audioStream.dwLength = 0;
    audioStream.dwSuggestedBufferSize = 30720;
    audioStream.dwQuality = -1;
    audioStream.dwSampleSize = wave.nBlockAlign;
    audioStream.rcFrame.left = 0;
    audioStream.rcFrame.top = 0;
    audioStream.rcFrame.right = 0;
    audioStream.rcFrame.bottom = 0;

    aviHeader.audioStreamHeader = &audioStream;
    //aviHeader.audioStreamHeader = Q_NULLPTR;
    aviHeader.waveFormateEx = &wave;
    //aviHeader.waveFormateEx = Q_NULLPTR;

    writeRawAVI.writeHeader(aviHeader);
}
