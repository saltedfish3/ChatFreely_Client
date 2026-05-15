#include "closebutton.h"

CloseButton::CloseButton(int width,int height,QWidget *parent)
    : QPushButton{parent},wide(width),height(height)
{
    if(width <= 0)
        this->wide = 50;
    if(height <= 0)
        this->height = 50;
    this->resize(this->wide,this->height);
}

void CloseButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor textColor;

    textColor = QColor(31, 41, 55);
    if(this->isDown() || this->underMouse())
    {
        textColor = QColor(255, 255, 255);
    }

    painter.setPen(textColor);
    painter.drawLine(this->wide/5 * 2.1,this->height/3 * 1.1,this->wide/5 * 3.2,this->height/3 * 2);
    painter.drawLine(this->wide/5 * 2.1,this->height/3 * 2,this->wide/5 * 3.2,this->height/3 * 1.1);
}

void CloseButton::enterEvent(QEnterEvent *event)
{
    this->update();
    QPushButton::enterEvent(event);
}

void CloseButton::leaveEvent(QEvent *event)
{
    this->update();
    QPushButton::leaveEvent(event);
}

void CloseButton::mousePressEvent(QMouseEvent *event)
{
    this->update();
    QPushButton::mousePressEvent(event);
}

void CloseButton::mouseReleaseEvent(QMouseEvent *event)
{
    this->update();
    QPushButton::mouseReleaseEvent(event);
}
