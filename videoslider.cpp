#include "videoslider.h"


videoslider::videoslider(QWidget *parent):QSlider(parent)
{

}
void videoslider::mousePressEvent(QMouseEvent *event)
{
    int value = QStyle::sliderValueFromPosition(minimum(), maximum(), event->pos().x(), width());
    setValue(value);
    emit SIG_valueChanged(value);
}
