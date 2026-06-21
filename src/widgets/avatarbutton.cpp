#include "avatarbutton.h"

AvatarButton::AvatarButton(QWidget *parent)
    : QPushButton{parent}
{
    this->animation = new QPropertyAnimation(this, "geometry");
    this->animation->setDuration(100);
}

void AvatarButton::enterEvent(QEnterEvent *event)
{
    this->animation->stop();
    this->animation->setDirection(QAbstractAnimation::Forward);
    this->animation->start();
    QPushButton::enterEvent(event);
}

void AvatarButton::leaveEvent(QEvent *event)
{
    this->animation->stop();
    this->animation->setDirection(QAbstractAnimation::Backward);
    this->animation->start();
    QPushButton::leaveEvent(event);
}

void AvatarButton::resizeEvent(QResizeEvent *event)
{
    emit sizeChanged();
    QPushButton::resizeEvent(event);
}

void AvatarButton::updateAnimation()
{
    QRect base = geometry();
    if(base.isEmpty())
        return;

    QRect target = base;
    target.setWidth(base.width() * 1.05);
    target.setHeight(base.height() * 1.05);
    target.moveCenter(base.center());

    this->animation->setStartValue(base);
    this->animation->setEndValue(target);
}
