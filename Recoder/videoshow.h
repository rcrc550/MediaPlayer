#ifndef VIDEOSHOW_H
#define VIDEOSHOW_H

#include <QWidget>
#include<QImage>
#include<QPaintEvent>
#include<QPainter>
namespace Ui {
class VideoShow;
}

class VideoShow : public QWidget
{
    Q_OBJECT

public:
    explicit VideoShow(QWidget *parent = nullptr);
    ~VideoShow();
public slots:
    void paintEvent(QPaintEvent *event);
    void slot_setImage(QImage img);
private:
    Ui::VideoShow *ui;
    QImage m_img;
};

#endif // VIDEOSHOW_H
