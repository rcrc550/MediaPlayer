#include "recorddialog.h"
#include "ui_recorddialog.h"

RecordDialog::RecordDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RecordDialog)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::WindowMinimizeButtonHint|Qt::WindowCloseButtonHint);
    m_picInpic = new VideoShow;
    m_picInpic->setGeometry(0,0,320,240);//设置出现位置和大小
    //设置对话框带 最小化最大化

     m_screenRecorder = NULL;
    //初始化设备
    initDev();
    //设置默认保存路径
    m_savePath = QApplication::applicationDirPath() + "/1.flv";
    ui->le_savePath->setText( m_savePath );
    //控件初始化
    ui->cb_cam->setChecked(true);
    ui->cb_desk->setChecked(false);
    ui->cb_picinpic->setChecked(false);
    //设置停止播放状态 UI 部分
    slot_setRecordState( Enum_Record_State::Stop );
}

RecordDialog::~RecordDialog()
{

    if( m_screenRecorder ) delete m_screenRecorder;
    if( m_picInpic ) delete m_picInpic;
    delete ui;
}
//初始化设备
void RecordDialog::initDev()
{
    av_register_all();
    avformat_network_init();
    avdevice_register_all();
    rct = qApp->desktop()->screenGeometry(); //获取用户的桌面大小, 用于录制屏幕的尺寸.
}
//
void RecordDialog::slot_setRecordState(int state)
{
    switch( state )
    {
    case Enum_Record_State::Stop:
        ui->lb_recTime->setText("00:00");
        //封闭控件
        ui->cbx_audio->setEnabled( true );
        ui->cbx_cam->setEnabled(true);
        ui->cb_cam->setEnabled(true);
        ui->cb_desk->setEnabled(true);
        ui->cb_picinpic->setEnabled(true);
        ui->pb_recordBegin->setEnabled(true);
        ui->pb_recordEnd->setEnabled(false);
        ui->pb_rtmpUrl->setEnabled(true);
        ui->pb_savePath->setEnabled( true );
    {
        QImage img;
        img.fill( Qt::black);
        ui->wdg_videoItem->slot_setImage( img );//结束录制, 发送图片, 清空显示
        m_picInpic->slot_setImage( img );
    }
        m_picInpic->close();
        break;
    case Enum_Record_State::Begin:
        ui->cbx_audio->setEnabled( false );
        ui->cbx_cam->setEnabled(false);
        ui->cb_cam->setEnabled(false);
        ui->cb_desk->setEnabled(false);
        ui->cb_picinpic->setEnabled(false);
        ui->pb_recordBegin->setEnabled(false);
        ui->pb_recordEnd->setEnabled(true);
        ui->pb_rtmpUrl->setEnabled(false);
        ui->pb_savePath->setEnabled( false );
        this->showMinimized();
        //加定时器用于显示录制时间, 这里省略了
        // m_recTime = QTime::currentTime();
        // m_recTimer->start(500);
        break;
    }
}
//勾选录制摄像头
void RecordDialog::on_cb_cam_clicked()
{
    ui->cb_cam ->setChecked( true);
    ui->cb_desk ->setChecked( false );
    ui->cb_picinpic->setChecked( false );
}
//勾选录制桌面
void RecordDialog::on_cb_desk_clicked()
{
    ui->cb_cam ->setChecked( false );
    ui->cb_desk ->setChecked( true );
    ui->cb_picinpic->setChecked( false );
}
//勾选画中画
void RecordDialog::on_cb_picinpic_clicked()
{
    ui->cb_cam ->setChecked( false );
    ui->cb_desk ->setChecked( false );
    ui->cb_picinpic->setChecked( true );
}
//设置保存路径
void RecordDialog::on_pb_savePath_clicked()
{
    //根据当前时间创建文件
    QString tmpPath = QTime::currentTime().toString("hhmmsszzz");
    tmpPath += ".flv";
    tmpPath += "D:/"+ tmpPath;
    QString path = QFileDialog::getSaveFileName( this ,"保存",tmpPath ,"视频文件(*.flv);;");
    if( path.remove(" ").isEmpty() )
    {
        QMessageBox::about(this ,"提示","重新设置");
        return;
    }
    ui->le_broadcast->setText( "" );
    m_savePath = path.replace("/" , "\\\\");
    ui->le_savePath->setText( m_savePath );
}
//设置直播地址
void RecordDialog::on_pb_rtmpUrl_clicked()
{
    QString url = ui->le_broadcast->text();
    m_savePath = url;
}
//设置录制开始
void RecordDialog::on_pb_recordBegin_clicked()
{
    //设置 录制
    if(m_screenRecorder && m_screenRecorder->m_saveVideoFileThread &&m_screenRecorder->m_saveVideoFileThread->isRunning() )
    {
        QMessageBox::critical(this , "提示","正在处理录制视频,稍后再来");
        return;
    }
    if( m_savePath.remove(" ").isEmpty())
    {
        QMessageBox::critical(this,"提示"," 先设置保存文件的名字! ");
        return;
    }
    if (m_screenRecorder){
        delete m_screenRecorder; m_screenRecorder = NULL;
    }
    m_screenRecorder = new ScreenRecorder;
    //录制画面预览
    connect(m_screenRecorder , SIGNAL(SIG_GetOneImage(QImage)) ,ui->wdg_videoItem , SLOT(slot_setImage(QImage)) );
    //画中画预览
    connect(m_screenRecorder , SIGNAL(SIG_sendPicInPic(QImage)) ,m_picInpic , SLOT(slot_setImage(QImage)) );
    //设置保存路径
    std::string str = m_savePath.toStdString();
    char* buf =(char*)str.c_str();
    m_screenRecorder->setFileName(buf);
    //设置录制帧率
    m_screenRecorder->setVideoFrameRate(FRAME_RATE);//设置帧率 默认 15 , 可以开
    //放设置添加帧率
    if (ui->cb_desk->isChecked()) //看是摄像头 ,桌面还是画中画
    {// 参数分别是桌面 摄像头 画中画 音频 是否参与录制
        m_screenRecorder->init(true,false,false,true);
       m_screenRecorder->startRecord();
    }
    else if( ui->cb_picinpic->isChecked() )
    {
        m_screenRecorder->init(false,false,true,true);
        m_screenRecorder->startRecord();
        m_picInpic->show();
    }else{
        m_screenRecorder->init(false,true,false,true);
        m_screenRecorder->startRecord();
    }
    //封闭控件
    slot_setRecordState( Enum_Record_State::Begin );
}

//设置录制结束
void RecordDialog::on_pb_recordEnd_clicked()
{
    //停止录像
    if( m_screenRecorder )
        m_screenRecorder->stopRecord();
    ui->lb_res->setText( m_savePath +" 录制成功");
    m_savePath="";
    slot_setRecordState( Enum_Record_State::Stop );
}
