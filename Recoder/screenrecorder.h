#ifndef SCREENRECORDER_H
#define SCREENRECORDER_H

#include <QObject>
#include "savevideofile.h"
//#include "video_read.h"
#include "audio_read.h"
//#include "screen_read.h"
#include "picinpic_read.h"
#include"common.h"
class ScreenRecorder : public QObject
{
    Q_OBJECT
public:
explicit ScreenRecorder();
    ~ScreenRecorder();
    void setFileName(char* str);
    int init(bool useDesk, bool useCam , bool usePicinpic , bool useAudio);
    void startRecord();
    void stopRecord();
    void setVideoFrameRate(int value);
    double getVideoPts();
    double getAudioPts();
signals:
    void SIG_GetOneImage(QImage img);
    void SIG_sendPicInPic( QImage img );
public:
    SaveVideoFileThread * m_saveVideoFileThread; //保存成视频文件的线程
    //Video_Read *m_videoThread; //获取视频的线程
    Audio_Read* m_audioThread; //获取音频的线程
    //Screen_Read * m_screenThread;
    PicInPic_Read *m_picinpicThread;
    bool m_useDesktop;
    bool m_useVideo;
    bool m_usePicInPic;
    bool m_useAudio;

signals:

};

#endif // SCREENRECORDER_H
