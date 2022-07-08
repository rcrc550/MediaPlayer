#ifndef VIDEOSLIDER_H
#define VIDEOSLIDER_H

#include <QObject>
#include<QSlider>
#include<QMouseEvent>
#include<QStyle>
class videoslider : public QSlider
{
    Q_OBJECT
public:
  explicit videoslider(QWidget *parent=nullptr);

signals:
    void SIG_valueChanged(int);
protected:
    void mousePressEvent(QMouseEvent *event);

};

#endif // VIDEOSLIDER_H
