#include "videoshow.h"
#include "ui_videoshow.h"

VideoShow::VideoShow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoShow)
{
    ui->setupUi(this);
}

VideoShow::~VideoShow()
{
    delete ui;
}

void VideoShow::paintEvent(QPaintEvent *event)
{
    //画黑色背景
    QPainter pianer(this);
    pianer.setBrush(Qt::black);//设置黑色画刷
    pianer.drawRect(0,0,this->width(),this->height());//画矩形
    //画视频帧
    //缩放
    if( m_img.size().height()<= 0  ) return;

    m_img = m_img.scaled(this->width() ,this->height()  , Qt::KeepAspectRatio );

    QPixmap pix =QPixmap::fromImage( m_img );
    int x = this->width() - pix.width();
    x = x/2;
    int y = this->height() - pix.height() ;
    y = y/2 ;

    pianer.drawPixmap( QPoint( x , y ) , pix);  //用画笔把图片画出来了

    pianer.end();
}

void VideoShow::slot_setImage(QImage img)
{
    m_img=img;
    this->update();
}
