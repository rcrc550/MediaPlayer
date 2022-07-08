#ifndef PICINPIC_READ_H
#define PICINPIC_READ_H

#include <QObject>
#include "SaveVideoFile.h"

#include"common.h"
#include<QApplication>
#include<QDesktopWidget>
#include<QScreen>
//opencv 头文件

class PicInPic_Read : public QObject
{
    Q_OBJECT
public:
    explicit PicInPic_Read(QObject *parent = nullptr);

    signals:
    void SIG_sendVideoFrame( QImage img ); // 用于预览
    void SIG_sendPicInPic( QImage img ); //用于显示画中画
    public slots:
    void slot_getVideoFrame(); //定时器周期获取画面
    void slot_openVideo(); //开启采集
    void slot_closeVideo(); //关闭采集
    void setSaveVideoFileThread(SaveVideoFileThread* pThread );
    private:
    VideoCapture cap; //opencv 采集摄像头对象
    QTimer * timer; //定时器
    int ImageToYuvBuffer(QImage &image,uint8_t **buffer); //RGB24 转为 yuv420p
    public:
    //时间戳 用于起始时间戳
    qint64 firstTime ;
    bool m_getFirst ;
    qint64 timeIndex ;
    // 编码线程
    SaveVideoFileThread* m_saveVideoFileThread;



};

#endif // PICINPIC_READ_H
