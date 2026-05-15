#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(int width, int height, QWidget *parent = nullptr);

signals:
};

#endif // MAINWIDGET_H
