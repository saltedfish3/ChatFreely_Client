#ifndef AVATARBUTTON_H
#define AVATARBUTTON_H

#include <QPushButton>
#include <QPropertyAnimation>
#include <QEvent>

class AvatarButton : public QPushButton
{
    Q_OBJECT
public:
    explicit AvatarButton(QWidget *parent = nullptr);
    void updateAnimation();
signals:
    void sizeChanged();

protected:
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:


    QPropertyAnimation* animation;
};

#endif // AVATARBUTTON_H
