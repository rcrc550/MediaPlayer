#ifndef AUDIO_READ_H
#define AUDIO_READ_H

#include <QObject>
#include "common.h"
#include"savevideofile.h"
#include<vector>
#include<QAudioInput>
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000//// 1 second of 48khz 32bit audio
//计算方法 采样率 * 采样时间间隔 * 一次采样字节数 = 数据量
//单通道 16 位 一次采样 2 字节 双通道 16 位 一次采样 4 字节
//aac 数据是一帧 1024 采样点 ffmpeg 使用 fltp 是 float 一个点 双声道*2 就是*8 即 8192 字节
//PCM 采样 选用 s16 就是 16 位 即 2 字节 , 双声道*2 就是*4 即采样字节数 4096
//44.1 kHz * 2 * 2 * AUDIO_INTERVAL ms = 4096
//AUDIO_INTERVAL = 23.2 即每次采样时间间隔不能低于 23.2 可以采用 24 ms
//48 kHz * 2 * 2 * AUDIO_INTERVAL ms = 4096
//AUDIO_INTERVAL = 21.3 即每次采样时间间隔不能低于 23.2 可以采用 22 ms
//单声道 队列投递一次字节数
#define OneAudioSize (4096)
//采样频率
#define AudioCollectFrequency (44100)
//采样间隔
#define AUDIO_INTERVAL (20)
//定时不准 一般延迟一些
//读一帧可能会有噪音 读两帧试试
//一次读取 8 帧 32768 4096
#define FrameOnce (1)
//声道数
#define AudioChannelCount (2)
class Audio_Read : public QObject
{
    Q_OBJECT
public:
    Audio_Read();
    ~Audio_Read();
    //用于获取音频队列, 进而投递数据
    void setSaveVideoFileThread(SaveVideoFileThread *newSaveVideoFileThread);
    public slots:
    void slot_readMore();//定时获取音频数据投递到队列
    void PauseAudio(); //暂停采集
    void ResumeAudio(); //开始采集
    void UnInit(); //回收
    private:
    QAudioInput* audio_in; //Qt 音频采集对象
    QIODevice *myBuffer_in;//IO 设备, 读写数据
     QAudioFormat format; //音频设备格式
    //采集状态
    enum ENUM_AUDIO_STATE{state_stop , state_play , state_pause };
    int m_playState;
    QTimer *timer;//定时器
    SaveVideoFileThread* m_saveVideoFileThread;//编码写文件线程类
    bool isBegin;
    SwrContext *swr; // 用于音频的转换
    int nb_samples;
    std::vector<uint8_t> m_audiobuff; //缓冲区排队执行
    int m_buffPos; // 未取走数据的起始位置

};

#endif // AUDIO_READ_H
