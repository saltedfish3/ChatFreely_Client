#ifndef MINIMIZEBUTTON_H
#define MINIMIZEBUTTON_H

#include <QPushButton>
#include <Qpainter>
#include <QStyleOption>

class MinimizeButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MinimizeButton(int width,int height,QWidget *parent = nullptr);
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

#endif // MINIMIZEBUTTON_H
