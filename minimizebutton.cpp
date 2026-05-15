#include "minimizebutton.h"

MinimizeButton::MinimizeButton(int width,int height,QWidget *parent)
    : QPushButton{parent},wide(width),height(height)
{
    if(width <= 0)
        this->wide = 50;
    if(height <= 0)
        this->height = 50;
    this->resize(this->wide,this->height);
}

void MinimizeButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);
    QColor textColor;

    textColor = QColor(31, 41, 55);

    painter.setPen(textColor);
    painter.drawLine(this->wide / 5 * 2,this->height / 2, this->wide / 5 * 3, this->height / 2);
}

void MinimizeButton::enterEvent(QEnterEvent *event)
{
    this->update();
    QPushButton::enterEvent(event);
}

void MinimizeButton::leaveEvent(QEvent *event)
{
    this->update();
    QPushButton::leaveEvent(event);
}

void MinimizeButton::mousePressEvent(QMouseEvent *event)
{
    this->update();
    QPushButton::mousePressEvent(event);
}

void MinimizeButton::mouseReleaseEvent(QMouseEvent *event)
{
    this->update();
    QPushButton::mouseReleaseEvent(event);
}
