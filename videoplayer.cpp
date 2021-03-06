#include "videoplayer.h"
#define MAX_AUDIO_SIZE (1024*16*25*10*16)//音频阈值
#define MAX_VIDEO_SIZE (1024*255*25*2*16*5)//视频阈值

#define SDL_AUDIO_BUFFER_SIZE 1024 //


//当队列里面的数据超过某个大小的时候 就暂停读取 防止一下子就把视频读完了，导致的空间分
//配不足

videoplayer::videoplayer():m_playerState(PlayerState::Stop)
{
   m_volunm=100;
}


void videoplayer::set_filename(QString filename)
{


    if( m_playerState != PlayerState::Stop ) return;
    file_name=filename;
    m_playerState = PlayerState::Playing;
    this->start();


}

void videoplayer::pause()
{

   if( m_playerState != Playing ) return;
    m_videoState.isPause = true;
    m_playerState= Pause;
}
void videoplayer::play()
{
    m_videoState.isPause = false;
    if( m_playerState != Pause) return;
    m_playerState = Playing;
}

void videoplayer::stop(bool iswait)
{
    m_videoState.quit = 1;
    if( iswait ) //阻塞标志
    {
        while(!m_videoState.readThreadFinished )//等待读取线程退出
        {
            SDL_Delay(10);
        }
    }
    m_playerState = PlayerState::Stop;
    Q_EMIT SIG_PlayerStateChanged(PlayerState::Stop);


}
//跳转
void videoplayer::seek(int64_t pos) //精确到微秒
{
 if(!m_videoState.seek_req)
 {
 m_videoState.seek_pos = pos;
 m_videoState.seek_req = 1;
 } }

void videoplayer::set_Volumn(int ak)
{
    m_volunm=ak;
}

void videoplayer::run()
{
    //开始解码获取图片
    //添加音频需要的变量
    int audioStream = -1;//音频解码器需要的流的索引
    AVCodecContext *pAudioCodecCtx = NULL;//音频解码器信息指针
    AVCodec *pAudioCodec = NULL; //音频解码器
    //SDL
    SDL_AudioSpec wanted_spec; //SDL 音频设置
    SDL_AudioSpec spec ; //SDL 音频设置
    //视频
    AVCodecContext *pCodecCtx ; //视频的解码器信息指针
    AVCodec *pCodec ; //视频解码器
    AVFrame *pFrame, *pFrameRGB;// 用来存解码后的数据
    AVPacket *packet;//读取解码前的包
    int numBytes;//帧数据大小
    uint8_t * out_buffer;//存储转化为 RGB 格式数据的缓冲区
    static struct SwsContext *img_convert_ctx;//YUV 转 RGB 的结构
    //1.初始化 FFMPEG 调用了这个才能正常适用编码器和解码器 注册所用函数
    av_register_all();
    avformat_network_init();
    avdevice_register_all();
    //SDL 初始化
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("Couldn't init SDL:%s\n", SDL_GetError());
        return;
    }
    memset ( &m_videoState , 0 , sizeof(VideoState) );
    //2.需要分配一个 AVFormatContext，FFMPEG 所有的操作都要通过这个 AVFormatContext 来
    //进行 可
    //以理解为视频文件指针
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    //中文兼容
    std::string path = file_name.toStdString();
    const char* file_path = path.c_str();
    //3. 打开视频文件
    if( avformat_open_input(&pFormatCtx, file_path, NULL, NULL) != 0 )
    {
        qDebug()<<"can't open file";
        avformat_close_input(&pFormatCtx);
        //回收资源之后,在最后添加读取文件线程退出标志.
        m_videoState.readThreadFinished = true;
        //视频自动结束 置标志位
        m_playerState = PlayerState::Stop;
        stop(true);
        return;
    }
    //3.1 获取视频文件信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        qDebug()<<"Could't find stream infomation.";
        return;
    }
    //4.读取视频流
    int videoStream = -1;
    //查找音频视频流索引
    if (find_stream_index(pFormatCtx, &videoStream, &audioStream) == -1)
    {
        printf("Couldn't find stream index\n");
        return;
    }
    m_videoState.pFormatCtx = pFormatCtx;
    m_videoState.videoStream = videoStream;
    m_videoState.audioStream = audioStream;
    m_videoState.m_player = this;
    if(videoStream != -1)
    {
        //5.查找解码器
        pCodecCtx = pFormatCtx->streams[ videoStream ]->codec;
        pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        if (pCodec == NULL) {
            printf("Codec not found.");
            return;
        }
        //打开解码器
        if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
            printf("Could not open codec.");
            return;
        }
        //视频流
        m_videoState.video_st = pFormatCtx->streams[ videoStream ];
        m_videoState.pCodecCtx = pCodecCtx;
        //视频同步队列
        m_videoState.videoq = new PacketQueue;
        packet_queue_init( m_videoState.videoq);
        //创建视频线程
        m_videoState.video_tid = SDL_CreateThread( video_thread ,"video_thread" , &m_videoState );
    }
    if( audioStream != -1 )
    {
        //5.找到对应的音频解码器
        pAudioCodecCtx = pFormatCtx->streams[audioStream]->codec;
        pAudioCodec = avcodec_find_decoder(pAudioCodecCtx ->codec_id);
        if (!pAudioCodec)
        {
            printf( "Couldn't find decoder\n");
            return;
        }//打卡音频解码器
        avcodec_open2(pAudioCodecCtx, pAudioCodec, NULL);
        m_videoState.audio_st = pFormatCtx->streams[audioStream];
        m_videoState.pAudioCodecCtx = pAudioCodecCtx;
        //6.设置音频信息, 用来打开音频设备。
        SDL_LockAudio();
        wanted_spec.freq = pAudioCodecCtx->sample_rate;
        switch (pFormatCtx->streams[audioStream]->codec->sample_fmt)
        {
        case AV_SAMPLE_FMT_U8:
            wanted_spec.format = AUDIO_S8;
            break;
        case AV_SAMPLE_FMT_S16:
            wanted_spec.format = AUDIO_S16SYS;
            break;
        default:
            wanted_spec.format = AUDIO_S16SYS;
            break;
        };
        wanted_spec.channels = pAudioCodecCtx->channels; //通道数
        wanted_spec.silence = 0; //设置静音值
        wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE; //读取第一帧后调整
        wanted_spec.callback = audio_callback;//回调函数
        wanted_spec.userdata = /*pAudioCodecCtx*/&m_videoState;//回调函数参数
        //7.打开音频设备
        m_videoState.audioID =SDL_OpenAudioDevice(SDL_GetAudioDeviceName(0,0),0,&wanted_spec, &spec,0);
        if( m_videoState.audioID < 0 ) //第二次打开 audio 会返回-1
        {
            printf( "Couldn't open Audio: %s\n", SDL_GetError());
            return;
        }
        //设置参数，供解码时候用, swr_alloc_set_opts 的 in 部分参数
        switch (pFormatCtx->streams[audioStream]->codec->sample_fmt)
        {
        case AV_SAMPLE_FMT_U8:
            m_videoState.out_frame.format = AV_SAMPLE_FMT_U8;
            break;
        case AV_SAMPLE_FMT_S16:
            m_videoState.out_frame.format = AV_SAMPLE_FMT_S16;
            break;
        default:
            m_videoState.out_frame.format = AV_SAMPLE_FMT_S16;
            break;
        };
        m_videoState.out_frame.sample_rate = pAudioCodecCtx->sample_rate;
        m_videoState.out_frame.channel_layout =
                av_get_default_channel_layout(pAudioCodecCtx->channels);
        m_videoState.out_frame.channels = pAudioCodecCtx->channels;
        m_videoState.audioq = new PacketQueue;
        m_videoState.audioFrame= av_frame_alloc();
        //初始化队列
        packet_queue_init(m_videoState.audioq);
        SDL_UnlockAudio();
        // SDL 播放声音 0 播放
        SDL_PauseAudioDevice(m_videoState.audioID,0);
    }
    packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个 packet
    m_playerState = Playing;
    int DelayCount=0;
    //8.循环读取视频帧, 转换为 RGB 格式, 抛出信号去控件显示
    Q_EMIT SIG_TotalTime( getTotalTime() );

    while(1)
    {
        if( m_videoState.quit ) break;
        if(m_videoState.isPause)
        {
            SDL_Delay(10);
            continue;
        }
        //这里做了个限制 当队列里面的数据超过某个大小的时候 就暂停读取 防止一下子就把
        //视频读完了，导致的空间分配不足
        /* 这里 audioq.size 是指队列中的所有数据包带的音频数据的总量或者视频数据总量，并
    不是包的数量 */
        //这个值可以稍微写大一些
        if( m_videoState.audioStream != -1 && m_videoState.audioq->size >MAX_AUDIO_SIZE ) {
            SDL_Delay(10);
            continue;
        }
        if ( m_videoState.videoStream != -1 &&m_videoState.videoq->size >
             MAX_VIDEO_SIZE) {
            SDL_Delay(10);
            continue;
        }
        if( m_videoState.seek_req )
            // 跳转标志位 seek_req --> 1 清除队列里的缓存 3s --> 3min 3s 里面的数据 存在 队列和解码器
            // 3s 在解码器里面的数据和 3min 的会合在一起 引起花屏 --> 解决方案 清理解码器缓存
            //AV_flush_...
                    //什么时候清理 -->要告诉它 , 所以要来标志包 FLUSH_DATA "FLUSH"
                    //关键帧--比如 10 秒 --> 15 秒 跳转关键帧 只能是 10 或 15 , 如果你要跳到 13 , 做法是跳到
                    //10 然后 10-13 的包全扔掉
            {
                int stream_index = -1;
                int64_t seek_target = m_videoState.seek_pos;//微秒
                if (m_videoState.videoStream >= 0)
                    stream_index = m_videoState.videoStream;
                else if (m_videoState.audioStream >= 0)
                    stream_index = m_videoState.audioStream;
                AVRational aVRational = {1, AV_TIME_BASE};
                if (stream_index >= 0) {
                    seek_target = av_rescale_q(seek_target, aVRational,
                                               pFormatCtx->streams[stream_index]->time_base); //跳转到的位置
                }
                if (av_seek_frame(m_videoState.pFormatCtx, stream_index, seek_target,
                                  AVSEEK_FLAG_BACKWARD) < 0) {
                    qDebug()<<stderr<< " "<<m_videoState.pFormatCtx->filename<<": error while seeking\n";
                } else {
                    if (m_videoState.audioStream >= 0) {
                        AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个 packet
                        av_new_packet(packet, 10);
                        strcpy((char*)packet->data,FLUSH_DATA);
                        packet_queue_flush(m_videoState.audioq); //清除队列
                        packet_queue_put(m_videoState.audioq, packet); //往队列中存入用来清除的包
                    }
                    if (m_videoState.videoStream >= 0) {
                        AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个 packet
                        av_new_packet(packet, 10);
                        strcpy((char*)packet->data,FLUSH_DATA);
                        packet_queue_flush(m_videoState.videoq); //清除队列
                        packet_queue_put(m_videoState.videoq, packet); //往队列中存入用来清除的包
                        m_videoState.video_clock = 0; //考虑到向左快退 避免卡死
                        //视频解码过快会等音频 循环 SDL_Delay 在循环过程中 音频时钟会改变 , 快退 音
                        //频时钟变小
                    }
                }
                m_videoState.seek_req = 0;
                m_videoState.seek_time = m_videoState.seek_pos ; //精确到微妙 seek_time 是用来做视
                //频音频的时钟调整 --关键帧
                m_videoState.seek_flag_audio = 1; //在视频音频循环中 , 判断, AVPacket 是 FLUSH_DATA
                //清空解码器缓存
                m_videoState.seek_flag_video = 1; }
                //可以看出 av_read_frame 读取的是一帧视频，并存入一个 AVPacket 的结构中
                if (av_read_frame(pFormatCtx, packet) < 0)
                {
                    DelayCount++;
                    if( DelayCount>= 300)
                    {
                        //会和视频音频做连动，然后退出来。
                        m_videoState.readFinished = true;
                        DelayCount = 0 ;
                    }

                    if( m_videoState.quit) break; //解码线程执行完 退出

                    SDL_Delay(10);
                    continue;
                }
        DelayCount = 0;

        //生成图片
        if (packet->stream_index == m_videoState.videoStream)
        {
            packet_queue_put(m_videoState.videoq, packet);
        }
        else if ( packet->stream_index == m_videoState.audioStream)
        {
            packet_queue_put(m_videoState.audioq, packet);
        }
        else
        {
            av_free_packet(packet);
        }
    }
    //当退出循环时,等待音频和视频解码结束
    while( !m_videoState.quit)
    {
        SDL_Delay(100);
    }
    //回收空间
    if( m_videoState.videoStream != -1)
        packet_queue_flush( m_videoState.videoq);//队列回收
    if( m_videoState.audioStream != -1)
        packet_queue_flush( m_videoState.audioq); //队列回收
    while( m_videoState.videoStream != -1 && !m_videoState.videoThreadFinished )
    {
        SDL_Delay(10);
    }


    //回收空间
    if( audioStream != -1 )
     {
        while( m_videoState.audioq->nb_packets != 0)
        {
            if( m_videoState .quit ) break;
            SDL_Delay(100);
        }
        avcodec_close(pAudioCodecCtx);
      }
    if( videoStream != -1 )
        {
        while( m_videoState.audioq->nb_packets != 0)
        {
            if( m_videoState .quit ) break;
            SDL_Delay(100);
        }
          avcodec_close(pCodecCtx);
        }
    avformat_close_input(&pFormatCtx);
    //关闭SDL 音频设备
        if (m_videoState.audioID != 0) {
                SDL_LockAudio();
            SDL_PauseAudioDevice(m_videoState.audioID,1);//停止播放,即停止音频回调函数
            SDL_CloseAudioDevice( m_videoState.audioID );
            SDL_UnlockAudio();
                m_videoState.audioID = 0;
        }
        m_playerState = PlayerState::Stop;
        av_free(m_videoState.audioFrame);
    //回收资源之后,在最后添加读取文件线程退出标志.
        // 清屏
            QImage img; //把图像复制一份 传递给界面显示
            img.fill(Qt::black);
            SendGetOneImage(img); //调用激发信号的函数


    m_videoState.readThreadFinished = true;





}
double videoplayer::getCurrentTime()
{
    return m_videoState.audio_clock;
}
//获取总时间
int64_t videoplayer::getTotalTime()
{
    if( m_videoState.pFormatCtx )
    return m_videoState.pFormatCtx->duration;
    return -1;
}
PlayerState videoplayer::getPlayerState() const
{
    return m_playerState;
}

int video_thread(void *arg)
{
    VideoState *is = (VideoState *) arg;
    AVPacket pkt1, *packet = &pkt1;
    int ret, got_picture, numBytes;
    double video_pts = 0; //当前视频的 pts
    double audio_pts = 0; //音频 pts
    ///解码视频相关
    AVFrame *pFrame, *pFrameRGB;
    uint8_t *out_buffer_rgb; //解码后的 rgb 数据
    struct SwsContext *img_convert_ctx; //用于解码后的视频格式转换
    AVCodecContext *pCodecCtx = is->pCodecCtx; //视频解码器
    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();
    ///这里我们改成了 将解码后的 YUV 数据转换成 RGB32
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                     pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
    numBytes = avpicture_get_size(AV_PIX_FMT_RGB32,
                                  pCodecCtx->width,pCodecCtx->height);
    out_buffer_rgb = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *) pFrameRGB, out_buffer_rgb, AV_PIX_FMT_RGB32,
                   pCodecCtx->width, pCodecCtx->height);

    while(1)
    {
        if( is->quit ) break;
        if(is->isPause)
        {
            SDL_Delay(10);
            continue;
        }
        if (packet_queue_get(is->videoq, packet, 0) <= 0) {
            if( is->readFinished && is->audioq->nb_packets == 0)//播放到结束
            {//读线程完毕
                break;
            }else
            {
                SDL_Delay(1); //只是队列里面暂时没有数据而已
                continue;
            } }
        //读取完毕了
        if(strcmp((char*)packet->data,FLUSH_DATA) == 0)
         {
         avcodec_flush_buffers(is->video_st->codec);
         av_free_packet(packet);
        is->video_clock = 0; //很关键 , 不清空 向左跳转, 视频帧会等待音频帧
         continue;
         }
        //添加位置如图
        //音频里没东西直接退出，音频时钟不会增加，视频再等没意义
        while(1)
        {
            if( is->quit) break;
            if( is ->audioq->size == 0   ) break;
            audio_pts = is->audio_clock;
            video_pts = is->video_clock;
            if (video_pts <= audio_pts) break;
            SDL_Delay(5);
        }

        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,packet);
        if (packet->dts == AV_NOPTS_VALUE && pFrame->opaque&& *(uint64_t*)
                pFrame->opaque != AV_NOPTS_VALUE)
        {
            video_pts = *(uint64_t *) pFrame->opaque;
        }
        else if (packet->dts != AV_NOPTS_VALUE)
        {
            video_pts = packet->dts;
        }
        else
        {
            video_pts = 0;
        }
        if( !is->beginFrame ) { is->begin_pts = video_pts ; is->beginFrame = true;
        is->begin_pts = ( is->begin_pts > 0 ? is->begin_pts: 0 );
         }
        video_pts -= is->begin_pts;
        video_pts *= 1000000 *av_q2d(is->video_st->time_base);
        video_pts = synchronize_video(is, pFrame, video_pts);//视频时钟补偿
        if (is->seek_flag_video) {
         //发生了跳转 则跳过关键帧到目的时间的这几帧
         if (video_pts < is->seek_time)
         {
         av_free_packet(packet);
         continue;
         }else
         {
         is->seek_flag_video = 0;
         } }
        if (got_picture) {
            sws_scale(img_convert_ctx,
                      (uint8_t const * const *) pFrame->data,
                      pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
                      pFrameRGB->linesize);
            //把这个 RGB 数据 用 QImage 加载
            QImage tmpImg((uchar *)out_buffer_rgb,pCodecCtx->width,pCodecCtx->height,QImage::Format_RGB32);
            QImage image = tmpImg.copy(); //把图像复制一份 传递给界面显示
            is->m_player->SendGetOneImage(image); //调用激发信号的函数
        }
        av_free_packet(packet);
    }
    if( !is->quit)
    {
        is->quit = true;
    }

    av_free(pFrame);
    av_free(pFrameRGB);
    av_free(out_buffer_rgb);
    is->videoThreadFinished = true;

    return 0;
}
void videoplayer::SendGetOneImage(QImage& img)
{
    emit SIG_getoneImage(img); //发送信号
}
//时间补偿函数--视频延时
double synchronize_video(VideoState *is, AVFrame *src_frame, double pts) {
    double frame_delay;
    if (pts != 0) {
        /* if we have pts, set video clock to it */
        is->video_clock = pts;
    } else {
        /* if we aren't given a pts, set it to the clock */
        pts = is->video_clock;
    }
    /* update the video clock */
    frame_delay = av_q2d(is->video_st->codec->time_base);
    /* if we are repeating a frame, adjust clock accordingly */
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    is->video_clock += frame_delay;
    return pts;
}
//13.回调函数中将从队列中取数据, 解码后填充到播放缓冲区.
void audio_callback(void *userdata, Uint8 *stream, int len)//全局的函数
{

    // AVCodecContext *pcodec_ctx = (AVCodecContext *) userdata;
    VideoState *is = (VideoState *) userdata;

    int len1, audio_data_size;
    memset(stream,0,len);
    if(is->isPause)return;
   // static uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
    //static unsigned int audio_buf_size = 0;
    //static unsigned int audio_buf_index = 0;
    /* len 是由 SDL 传入的 SDL 缓冲区的大小，如果这个缓冲未满，我们就一直往里填充数据 */
    /* audio_buf_index 和 audio_buf_size 标示我们自己用来放置解码出来的数据的缓冲区，*/
    /* 这些数据待 copy 到 SDL 缓冲区， 当 audio_buf_index >= audio_buf_size 的时候意味着我*/
    /* 们的缓冲为空，没有数据可供 copy，这时候需要调用 audio_decode_frame 来解码出更
/* 多的桢数据 */
    while (len > 0)
    {
        if (is->audio_buf_index >= is->audio_buf_size) {
            audio_data_size = audio_decode_frame( is, is->audio_buf,sizeof(is->audio_buf));
            /* audio_data_size < 0 标示没能解码出数据，我们默认播放静音 */
            if (audio_data_size < 0) {
                /* silence */
                is->audio_buf_size = 1024;
                /* 清零，静音 */
                memset(is->audio_buf, 0, is->audio_buf_size);
            } else {
                is->audio_buf_size = audio_data_size;
            }
            is->audio_buf_index = 0;
        }
        /* 查看 stream 可用空间，决定一次 copy 多少数据，剩下的下次继续 copy */
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len) {
            len1 = len;
        }
        //SDL_MixAudio 并不能用
        //memcpy(stream, (uint8_t *) audio_buf + audio_buf_index, len1);
        memset( stream , 0 , len1);
        //混音函数 sdl 2.0 版本使用该函数 替换 SDL_MixAudio
        SDL_MixAudioFormat(stream, (uint8_t *) is->audio_buf + is->audio_buf_index,
                           AUDIO_S16SYS,len1,is->m_player->m_volunm);
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    } }
//音频解码函数.
//对于音频来说，一个 packet 里面，可能含有多帧(frame)数据。
//解码音频帧函数
int audio_decode_frame(VideoState *is, uint8_t *audio_buf, int buf_size)
{
    static AVPacket pkt;
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    int len1, data_size;
    int sampleSize = 0;
    AVCodecContext *aCodecCtx = is->pAudioCodecCtx;
    static AVFrame *audioFrame = is->audioFrame/*av_frame_alloc()*/;
    PacketQueue *audioq = is->audioq;
    AVFrame wanted_frame = is->out_frame;
    if( !aCodecCtx|| !audioFrame ||!audioq) return -1;
    static struct SwrContext *swr_ctx = NULL;
    int convert_len;
    int n = 0;
    for(;;)
    {
        if( is->quit ) break;
        if(is->isPause)
        {
            SDL_Delay(10);
            continue;
        }
        if( is->quit ) return -1;
        if( !audioq ) return -1;
        if(audioq&&packet_queue_get(audioq, &pkt, 0) <= 0) //一定注意
        {
            if( is->readFinished && is->audioq->nb_packets == 0 )
                is->quit = true;
            return -1;
        }
        if(strcmp((char*)pkt.data,FLUSH_DATA) == 0)
         {
            avcodec_flush_buffers(is->audio_st->codec);
            av_free_packet(&pkt);
            continue;
         }
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
        while(audio_pkt_size > 0)
        {
            if( is->quit ) return -1;
            int got_picture;
            memset(audioFrame, 0, sizeof(AVFrame));
            int ret =avcodec_decode_audio4( aCodecCtx, audioFrame, &got_picture, &pkt);
            if( ret < 0 ) {
                printf("Error in decoding audio frame.\n");
                exit(0);
            } //一帧一个声道读取数据字节数是 nb_samples , channels 为声道数 2 表示 16 位 2 个字节
            //data_size = audioFrame->nb_samples * wanted_frame.channels * 2;
            switch( is->out_frame.format )
            {
            case AV_SAMPLE_FMT_U8:
                data_size = audioFrame->nb_samples * is->out_frame.channels * 1;
                break;
            case AV_SAMPLE_FMT_S16:
                data_size = audioFrame->nb_samples * is->out_frame.channels * 2;
                break;
            default:
                data_size = audioFrame->nb_samples * is->out_frame.channels * 2;
                break;
            } //sampleSize 表示一帧(大小 nb_samples)audioFrame 音频数据对应的字节数.
            sampleSize = av_samples_get_buffer_size(NULL, is->pAudioCodecCtx->channels,
                                                    audioFrame->nb_samples,
                                                    is->pAudioCodecCtx->sample_fmt, 1);
            //n 表示每次采样的字节数
            n =av_get_bytes_per_sample(is->pAudioCodecCtx->sample_fmt)*is->pAudioCodecCtx->channels; //时钟每次要加一帧数据的时间= 一帧数据的大小/一秒钟采样 sample_rate 多次对应的自己数.
            is->audio_clock += (double)sampleSize*1000000/(double) (n* is->pAudioCodecCtx->sample_rate);
            //跳转到关键帧,跳过一些帧
            if( is->seek_flag_audio) {
             if( is ->audio_clock < is->seek_time) //没有到目的时间
             {
             if( pkt.pts != AV_NOPTS_VALUE)
             {
             is->audio_clock = av_q2d( is->audio_st->time_base )*pkt.pts *1000000 ; //取
            //音频时钟 可能精度不够
             }
             break;
             }
             else
             {
             if( pkt.pts != AV_NOPTS_VALUE)
             {
             is->audio_clock = av_q2d( is->audio_st->time_base )*pkt.pts *1000000 ; //取
            //音频时钟 可能精度不够
             }
             is->seek_flag_audio = 0 ;
             } }
            if( got_picture )
            {
                if (swr_ctx != NULL)
                {
                    swr_free(&swr_ctx);
                    swr_ctx = NULL;
                }
                swr_ctx = swr_alloc_set_opts(NULL, wanted_frame.channel_layout,
                                             (AVSampleFormat)wanted_frame.format,wanted_frame.sample_rate,
                                             audioFrame->channel_layout,(AVSampleFormat)audioFrame->format,
                                             audioFrame->sample_rate, 0, NULL);
                //初始化
                if (swr_ctx == NULL || swr_init(swr_ctx) < 0)
                {
                    printf("swr_init error\n");
                    break;
                }
                convert_len = swr_convert(swr_ctx, &audio_buf,
                                          AVCODEC_MAX_AUDIO_FRAME_SIZE,
                                          (const uint8_t **)audioFrame->data,
                                          audioFrame->nb_samples);
                swr_free(&swr_ctx);
            }
            audio_pkt_size -= ret;
            if (audioFrame->nb_samples <= 0)
            {
                continue;
            }
            av_free_packet(&pkt);
            return data_size ;
        }

        av_free_packet(&pkt);
    } }
//查找数据流函数
int videoplayer::find_stream_index(AVFormatContext *pformat_ctx, int *video_stream, int
                      *audio_stream)
{
    assert(video_stream != NULL || audio_stream != NULL);
    int i = 0;
    int audio_index = -1;
    int video_index = -1;
    for (i = 0; i < pformat_ctx->nb_streams; i++)
    {
        if (pformat_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_index = i;
        }
        if (pformat_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audio_index = i;
        }
    }
    //注意以下两个判断有可能返回-1.
    if (video_stream == NULL)
    {
        *audio_stream = audio_index;
        return *audio_stream;
    }
    if (audio_stream == NULL)
    {
        *video_stream = video_index;
        return *video_stream;
    }
    *video_stream = video_index;
    *audio_stream = audio_index;
    return 0;
}
