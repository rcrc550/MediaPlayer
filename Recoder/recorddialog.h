#ifndef RECORDDIALOG_H
#define RECORDDIALOG_H
#include <QDialog>
#include"screenrecorder.h"
#include"common.h"
#include<QFileDialog>
#include<QApplication>//获取exe路径 分辨率 桌面
#include<qdesktopwidget.h>//获取桌面会用到的

#include"videoshow.h"
QT_BEGIN_NAMESPACE
namespace Ui { class RecordDialog; }
QT_END_NAMESPACE

class RecordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RecordDialog(QWidget *parent = nullptr);
    ~RecordDialog();
    void initDev();
    enum Enum_Record_State{Stop,Begin};
private slots:
    void on_pb_recordBegin_clicked(); //开始录制
    void on_pb_recordEnd_clicked(); //结束录制
    void on_pb_savePath_clicked();//保存路径设置
    void on_pb_rtmpUrl_clicked();//直播地址设置
    void on_cb_cam_clicked();//摄像头录制
    void on_cb_desk_clicked();//桌面录制
    void on_cb_picinpic_clicked();//画中画录制
    void slot_setRecordState( int state);//设置录制状态
private:
    Ui::RecordDialog *ui;
    QString m_savePath;
    ScreenRecorder *m_screenRecorder; //录屏对象
    QRect rct;
    VideoShow * m_picInpic;//画中画显示窗口

};
#endif // RECORDDIALOG_H
