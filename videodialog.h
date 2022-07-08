#ifndef VIDEODIALOG_H
#define VIDEODIALOG_H
#include<QString>
#include <QDialog>
#include<QTimer>
#include<QImage>
#include"videoplayer.h"
#include"customwidget.h"
#include"onlinedialog.h"
#include"csqlite.h"
#include<QListWidgetItem>
#include<QMenu>
#define MAX_LISTCOUNT 100
QT_BEGIN_NAMESPACE
namespace Ui { class VideoDialog; }
QT_END_NAMESPACE

class VideoDialog : public CustomMoveDialog
{
    Q_OBJECT

public:
    VideoDialog(QWidget *parent = nullptr);
    ~VideoDialog();

private slots:
    void on_pb_start_clicked();
    void slot_refresh(QImage amge);
    void on_pb_pause_clicked();
    void slot_getTotalTime(qint64 uSec);

    void slot_TimerTimeOut();
    void on_pb_resume_clicked();

    void on_pb_stop_clicked();

    void slot_slider_video_valueChanged(int value);
    void slot_PlayerStateChanged(int state);
    void on_pb_listshow_clicked();

    void on_pb_quit_clicked();

    void on_pb_max_clicked();

    void on_pb_min_clicked();
    void SLOT_playRtmpUrl(QString);
    void on_slider_music_valueChanged(int value);

    void loadDB();
    void on_pb_online_clicked();
    void slot_dealListMenu(QAction *a);
    void on_lw_videolist_itemClicked(QListWidgetItem *item);

private:
    Ui::VideoDialog *ui;
    videoplayer *m_player;
    QTimer *m_Timer;
    OnlineDialog *m_onlinedialog;
    int istop;
    CSqlite *m_sql;
    QString m_videolist[MAX_LISTCOUNT];
    int m_count;
    QMenu m_listmennu;
};
#endif // VIDEODIALOG_H
