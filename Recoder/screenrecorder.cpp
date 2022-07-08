#include "screenrecorder.h"
#include "savevideofile.h"
#include "audio_read.h"
#include "picinpic_read.h"
ScreenRecorder::ScreenRecorder()
{
    //m_videoThread = new Video_Read();
    m_audioThread = new Audio_Read();
    //m_screenThread = new Screen_Read();
    m_picinpicThread = new PicInPic_Read();
    m_saveVideoFileThread = new SaveVideoFileThread;
    //m_videoThread->setSaveVideoFileThread(m_saveVideoFileThread);
    m_audioThread->setSaveVideoFileThread(m_saveVideoFileThread);
    m_picinpicThread->setSaveVideoFileThread(m_saveVideoFileThread);
    //m_screenThread->setSaveVideoFileThread(m_saveVideoFileThread);
    //connect( m_videoThread , SIGNAL(SIG_sendVideoFrame(QImage))
           //  , this , SIGNAL(SIG_GetOneImage(QImage)) );
    connect( m_picinpicThread , SIGNAL(SIG_sendVideoFrame(QImage)), this , SIGNAL(SIG_GetOneImage(QImage)) );
    //用于画中画显示
    connect( m_picinpicThread , SIGNAL(SIG_sendPicInPic(QImage)), this , SIGNAL(SIG_sendPicInPic(QImage)) );
    //connect( m_screenThread , SIGNAL( SIG_sendScreenFrame(QImage))
          //   , this , SIGNAL(SIG_GetOneImage(QImage)) );
}
ScreenRecorder::~ScreenRecorder()
{
    //delete m_videoThread;
    delete m_audioThread;
    //delete m_screenThread;
    delete m_picinpicThread;
    if (m_saveVideoFileThread)
    {
        delete m_saveVideoFileThread;
    }
}
void ScreenRecorder::setFileName(char *str)
{
    if (m_saveVideoFileThread != NULL)
        m_saveVideoFileThread->setFileName(str);
}
int ScreenRecorder::init(bool useDesk, bool useCam , bool usePicinpic, bool useAudio)
{
    m_useVideo = useCam;
    m_useAudio = useAudio;
    m_useDesktop = useDesk;
    m_usePicInPic =usePicinpic;
    if (m_saveVideoFileThread != NULL)
    {
        m_saveVideoFileThread->setContainsVideo(m_useVideo|| m_useDesktop ||m_usePicInPic);
        m_saveVideoFileThread->setContainsAudio(m_useAudio);
    }
    return 1;
}
void ScreenRecorder::startRecord()
{
    if (m_saveVideoFileThread != NULL)
        m_saveVideoFileThread->startEncode();
    //if (m_useVideo)
        //m_videoThread->slot_openVideo();
    if ( m_usePicInPic)
        m_picinpicThread->slot_openVideo();
    //if (m_useDesktop)
        //m_screenThread->slot_openScreen();
    if (m_useAudio)
        m_audioThread->ResumeAudio();
}
void ScreenRecorder::stopRecord()
{
    if (m_useAudio)
        m_audioThread->PauseAudio();
    if (m_usePicInPic)
        m_picinpicThread->slot_closeVideo();
    if (m_useVideo)
        //m_videoThread->slot_closeVideo();
    if( m_useDesktop )
        //m_screenThread->slot_closeScreen();
    if (m_saveVideoFileThread != NULL)
        m_saveVideoFileThread->stopEncode();
}
void ScreenRecorder::setVideoFrameRate(int value)
{
    if (m_saveVideoFileThread != NULL)
        m_saveVideoFileThread->setVideoFrameRate(value);
}
double ScreenRecorder::getVideoPts()
{
    if (m_saveVideoFileThread != NULL)
        return m_saveVideoFileThread->getVideoPts();
    else
        return 0;
}
double ScreenRecorder::getAudioPts()
{
    if (m_saveVideoFileThread != NULL)
        return m_saveVideoFileThread->getAudioPts();
    else
        return 0;
}
