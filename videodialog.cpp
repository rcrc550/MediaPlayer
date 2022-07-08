#include "videodialog.h"
#include "ui_videodialog.h"
#include<QDebug>
#include<QFileDialog>
#include<QMessageBox>
#include<QApplication>
#define FILE_NAME "C:/Users/ZhangZilong/Music/MV/123.mp4"

VideoDialog::VideoDialog(QWidget *parent)
    : CustomMoveDialog(parent)
    , ui(new Ui::VideoDialog),istop(true)
{

    ui->setupUi(this);
    m_count=0;
    m_player=new videoplayer;
    m_onlinedialog=new OnlineDialog;
    //m_player->set_filename(FILE_NAME);
    connect(m_player,SIGNAL(SIG_getoneImage(QImage)),this,SLOT(slot_refresh(QImage)));
    connect( m_player, SIGNAL( SIG_TotalTime(qint64)) ,this ,SLOT( slot_getTotalTime(qint64)) );
    m_Timer = new QTimer; //定时器-获取当前视频时间
     m_sql=new CSqlite;
    connect(m_Timer,SIGNAL(timeout()),this,SLOT(slot_TimerTimeOut()));
    m_Timer->setInterval(500);
    connect( ui->slider_video, SIGNAL(SIG_valueChanged(int)) , this,SLOT(slot_slider_video_valueChanged(int)) );
    ui->lw_videolist->hide();
    //slot_PlayerStateChanged(PlayerState::Stop);
    connect( m_player, SIGNAL(SIG_PlayerStateChanged(int)) , this,SLOT(slot_PlayerStateChanged(int)) );
    connect( m_onlinedialog,SIGNAL(SIG_rtmp(QString)) , this,SLOT(SLOT_playRtmpUrl(QString)) );
    ui->lw_videolist->hide();
    this->setWindowFlags(Qt::FramelessWindowHint);
    loadDB();
    ui->slider_music->setValue(100);
    m_listmennu.addAction("播放音乐");
    m_listmennu.addAction("删除音乐");
    m_listmennu.addAction("清空音乐");
    connect(&m_listmennu,SIGNAL(triggered(QAction*)),this,SLOT(slot_dealListMenu(QAction*)));
}

VideoDialog::~VideoDialog()
{
    if(m_player)
    {
        m_player->stop(true);
    }
    if(m_player&&m_player->isRunning())
    {
        m_player->terminate();
        m_player->wait(100);
        delete m_player;
        m_player=NULL;
    }
    else if(m_player)
    {
        delete m_player;
        m_player=NULL;
    }

    if(m_onlinedialog)
    {
        m_onlinedialog->close();
        delete m_onlinedialog;
        m_onlinedialog=NULL;
    }
    delete ui;

}

void VideoDialog::slot_PlayerStateChanged(int state)
{
switch( state )
{
case PlayerState::Stop:
    qDebug()<< "VideoPlayer::Stop";
    m_Timer->stop();
    ui->slider_video->setValue(0);
    ui->lb_time2->setText("00:00:00");
    ui->lb_time1->setText("00:00:00");
    ui->lb_videoname->setText("");
    ui->pb_pause->hide();
    ui->pb_resume->show();

    this->update();
    istop = true;
    break;
case PlayerState::Playing:
    qDebug()<< "VideoPlayer::Playing";
    //ui->pb_play->hide();
    ui->pb_pause->show();
    m_Timer->start();
    this->update();
    istop = false;
    break;
}
}
void VideoDialog::slot_TimerTimeOut()
{
    if (QObject::sender() == m_Timer)
    {
        qint64 Sec = m_player->getCurrentTime()/1000000;
        ui->slider_video->setValue(Sec);
        QString hStr = QString("00%1").arg(Sec/3600);
        QString mStr = QString("00%1").arg(Sec/60%60);
        QString sStr = QString("00%1").arg(Sec%60);
        QString str = QString("%1:%2:%3").arg(hStr.right(2)).arg(mStr.right(2)).arg(sStr.right(2));
        ui->lb_time1->setText(str);
        if(ui->slider_video->value() == ui->slider_video->maximum()&& m_player->m_playerState == PlayerState::Stop)
        {
            slot_PlayerStateChanged( PlayerState::Stop );
        }
        else if(ui->slider_video->value() + 1 ==ui->slider_video->maximum()&& m_player->m_playerState == PlayerState::Stop)
        {
           slot_PlayerStateChanged( PlayerState::Stop );
        }
    }
}
//获取视频全部时间
void VideoDialog::slot_getTotalTime(qint64 uSec)
{
    qint64 Sec = uSec/1000000;
    ui->slider_video->setRange(0,Sec);//精确到秒
    QString hStr = QString("00%1").arg(Sec/3600);
    QString mStr = QString("00%1").arg(Sec/60);
    QString sStr = QString("00%1").arg(Sec%60);
    QString str =QString("%1:%2:%3").arg(hStr.right(2)).arg(mStr.right(2)).arg(sStr.right(2));
    ui->lb_time2->setText(str);
}

void VideoDialog::on_pb_start_clicked()
{//打开文件 弹出对话框 参数:父窗口, 标题, 默认路径, 筛选器
    QString path = QFileDialog::getOpenFileName(this,"选择要播放的文件" , "F:/",
    "视频文件 (*.flv *.rmvb *.avi *.MP4 *.mkv);; 所有文件(*.*);;");
    if(!path.isEmpty())
    {
    qDebug()<< path ;
    QFileInfo info(path);
    if( info.exists() )
    {
    if(m_player->m_playerState != PlayerState::Stop)
    m_player->stop( true ); //如果播放 你要先关闭
    m_player->set_filename(path);

    ui->lb_videoname->setText( info.baseName() );
    slot_PlayerStateChanged(PlayerState::Playing);

    //添加到列表
      ui->lw_videolist->addItem(info.baseName());
      m_videolist[m_count++]=path;
      QString  sqlstr=QString("insert into t_videolist(fileName,filePath) values('%1','%2')").arg(info.baseName() ).arg(path);
      m_sql->UpdateSql(sqlstr);
     }
    else
    {
        QMessageBox::information( this, "提示" , "打开文件失败");
    }
    }

   // m_player->start();
   // m_Timer->start();
    //开启线程 获取图片 抛出信号给槽函数 槽函数来接  然后刷新界面
}

void VideoDialog::slot_refresh(QImage amge)
{
    ui->wdg_video->slot_setImage(amge);

}


void VideoDialog::on_pb_pause_clicked()
{
    if( istop ) return;//停止状态自然不能播放
     m_player->pause();
    if( m_player->getPlayerState() == PlayerState::Pause)
    {
    ui->pb_pause->hide();
    ui->pb_resume->show();
    this->update();
    }
    m_Timer->stop();
}


void VideoDialog::on_pb_resume_clicked()
{
    if( istop ) return;
    m_player->play();
    if(m_player->getPlayerState() == PlayerState::Playing)
    {
    ui->pb_resume->hide();
    ui->pb_pause->show();
    this->update();
    }
     m_Timer->start();
}


void VideoDialog::on_pb_stop_clicked()
{
   m_player->stop(true);
     m_Timer->stop();
}


void VideoDialog::slot_slider_video_valueChanged(int value)
{

       m_player->seek((qint64)value*1000000); //value 秒

}


void VideoDialog::on_pb_listshow_clicked()
{
    if(ui->lw_videolist->isHidden())
      {
        ui->lw_videolist->show();
        ui->pb_listshow->setText(">");
      }
    else
        {
           ui->lw_videolist->hide();
           ui->pb_listshow->setText("<");
        }
}


void VideoDialog::on_pb_quit_clicked()
{
    this->slot_close();
}


void VideoDialog::on_pb_max_clicked()
{
   this->showMaximized();
}


void VideoDialog::on_pb_min_clicked()
{
    this->showMinimized();
}

void VideoDialog::SLOT_playRtmpUrl(QString str)
{
    if(m_player->m_playerState != PlayerState::Stop)
    m_player->stop( true ); //如果播放 你要先关闭
    m_player->set_filename(str);

    ui->lb_videoname->setText( "网络资源");
    slot_PlayerStateChanged(PlayerState::Playing);
}


void VideoDialog::on_slider_music_valueChanged(int value)
{
    m_player->set_Volumn(value);
}
//加载数据库 为了写播放列表
void VideoDialog::loadDB()
{
      QString dbstr=QApplication::applicationDirPath()+"/sql/";//exe同级目录下查找这个路径
      QString filename="video.db";
      QDir tmpdir;
      //是否存在路径
      if(!tmpdir.exists(dbstr))
      {
          qDebug()<<"不存在这个路径，创建";
          tmpdir.mkdir(dbstr);
      }
      //是否存在这个文件 创建文件 创建表
      QFile tmpfile;
      if(!tmpfile.exists(dbstr+filename))
      {
           qDebug()<<"不存在这个文件，创建";
           tmpfile.setFileName(dbstr+filename);
           if(!tmpfile.open(QIODevice::WriteOnly|QIODevice::Text))
           {
               qDebug()<<"创建失败";
           }
           else
           {
               tmpfile.close();
               m_sql->ConnectSql(dbstr+filename);
               QString sqlstr="create table t_videolist (fileName varchar(260),filePath varchar(260));";
               m_sql->UpdateSql(sqlstr);
           }
      }
      else//存在的话 加载到空间
      {

           qDebug()<<"存在这个文件";
           m_sql->ConnectSql(dbstr+filename);

           QString sqlstr="select fileName,filePath from t_videolist;";
           QStringList reslist;
           bool res=m_sql->SelectSql(sqlstr,2,reslist);
           if(!res)
           { qDebug()<<"索取错误";
               return;
           }
           for(int i=0;i<reslist.count();i+=2)
           {
                qDebug()<<reslist[i];
               qDebug()<<reslist[i+1];
               ui->lw_videolist->addItem(reslist[i]);
               m_videolist[m_count++]=reslist[i+1];

           }
      }
}


void VideoDialog::on_pb_online_clicked()
{
    m_onlinedialog->showNormal();

}

void VideoDialog::slot_dealListMenu(QAction *action)
{
    if(action->text()=="播放音乐")
    {
      QString path=m_videolist[ui->lw_videolist->currentRow()];
      QFileInfo info(path);
      if(!info.exists())
      {
          qDebug()<<"路径错误";
          return;
      }
      if(m_player->m_playerState != PlayerState::Stop)
      m_player->stop( true ); //如果播放 你要先关闭
      m_player->set_filename(path);

      ui->lb_videoname->setText( info.baseName());
      slot_PlayerStateChanged(PlayerState::Playing);
    }
    else if(action->text()=="删除音乐")
    {
        QString str=QString("delete from t_videolist where filePath = '%1';").arg(m_videolist[ui->lw_videolist->currentRow()]);//先删除数据库
        m_sql->UpdateSql(str);
       ui->lw_videolist->takeItem(ui->lw_videolist->currentRow());//删除一列
        ui->lw_videolist->setCurrentRow(-1);                     //清除焦点 避免删完以后跑到下一行去了
        for(int i=0;i<m_count;i++)
        {
            m_videolist[i]=m_videolist[i+1];
        }
          m_count--;
    }
     else if(action->text()=="清空音乐")
     {

     }
}


void VideoDialog::on_lw_videolist_itemClicked(QListWidgetItem *item)
{
    m_listmennu.exec(QCursor::pos());//鼠标当前位置
}

