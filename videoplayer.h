#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H
#include"packetqueue.h"
#include"QImage"
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000 //1 second of 48khz 32bit audio
extern "C"
{
 #include "libavcodec/avcodec.h"
 #include "libavformat/avformat.h"
 #include "libavutil/pixfmt.h"
 #include "libswscale/swscale.h"
 #include "libavformat/avformat.h"
 #include "libavdevice/avdevice.h"
 #include"libswresample/swresample.h"
 #include"libavutil/time.h"
 #include "SDL.h"

}
enum PlayerState
{
    Playing = 0,
    Pause,
    Stop
};

class videoplayer ;
typedef struct VideoState {
    uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int audio_buf_size = 0;
    unsigned int audio_buf_index = 0;
    AVFormatContext *pFormatCtx;//相当于视频”文件指针”
    ///////////////音频///////////////////////////////////
    AVStream *audio_st; //音频流
    PacketQueue *audioq;//音频缓冲队列
    AVCodecContext *pAudioCodecCtx ;//音频解码器信息指针
    int audioStream;//视频音频流索引
    double audio_clock; ///<pts of last decoded frame 音频时钟
    SDL_AudioDeviceID audioID; //音频 ID
    AVFrame out_frame; //设置参数，供音频解码后的 swr_alloc_set_opts 使用。
    //////////////////////////////////////////////////////
    ///////////////视频///////////////////////////////////
    AVStream *video_st; //视频流
    PacketQueue *videoq;//视频队列
    AVCodecContext *pCodecCtx ;//音频解码器信息指针
    int videoStream;//视频音频流索引
    double video_clock; ///<pts of last decoded frame 视频时钟
    SDL_Thread *video_tid; //视频线程 id
    /////////////////////////////////////////////////////

    //////////播放控制//////////////////////////////////////
    int quit;
    bool isPause;//暂停标志
    bool readFinished; //读线程文件读取完毕
    bool readThreadFinished; //读取线程是否结束
    bool videoThreadFinished; // 视频线程是否结束
    /////////////////////////////////////////////////////
    //// 跳转相关的变量

    int             seek_req; //跳转标志 -- 读线程
    int64_t         seek_pos; //跳转的位置 -- 微秒
    int             seek_flag_audio;//跳转标志 -- 用于音频线程中
    int             seek_flag_video;//跳转标志 -- 用于视频线程中
    double          seek_time; //跳转的时间(秒)  值和seek_pos是一样的
    AVFrame *audioFrame /*av_frame_alloc()*/;
    //////////////////////////////////////////////////////
    int64_t start_time; //单位 微秒
    int64_t begin_pts ;
    bool beginFrame;
    VideoState()
    {
        audio_clock = video_clock = start_time = 0;
        begin_pts = 0;
        beginFrame=false;
        //volunm=50;

    }
    videoplayer* m_player;//用于调用函数

} VideoState;
#include<QDebug>
#include<QThread>
class videoplayer:public QThread
{
    Q_OBJECT
signals:
    void SIG_getoneImage(QImage img);
    void SIG_PlayerStateChanged(int ik);
    void SIG_TotalTime( qint64 ik );

public:
    videoplayer();
public slots:
    void set_filename(QString filename);

    void SendGetOneImage(QImage& img);
    double getCurrentTime();
    int64_t getTotalTime();
    int find_stream_index(AVFormatContext *pformat_ctx, int *video_stream, int *audio_stream);
    PlayerState getPlayerState() const;
    void seek(int64_t pos);
    void set_Volumn(int ak);
public:
    QString file_name;
    PlayerState m_playerState;
    VideoState m_videoState;
    int             m_volunm;  //音量
public:
    void run();
    void pause();
    void play();
    void  stop(bool iswait=false);
};
int video_thread(void *arg);
 void audio_callback(void *userdata, Uint8 *stream, int len);
 double synchronize_video(VideoState *is, AVFrame *src_frame, double pts) ;
 int audio_decode_frame(VideoState *is, uint8_t *audio_buf, int buf_size);


#endif // VIDEOPLAYER_H
