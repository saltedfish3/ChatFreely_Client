#ifndef CLOSEBUTTON_H
#define CLOSEBUTTON_H

#include <QPushButton>
#include <QPainter>
#include <QPainterPath>

class CloseButton : public QPushButton
{
    Q_OBJECT
public:
    explicit CloseButton(int width,int height, QWidget *parent = nullptr);
    inline int GetHeight(){return this->height;};

private:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    int wide;
    int height;

signals:
};

#endif // CLOSEBUTTON_H
