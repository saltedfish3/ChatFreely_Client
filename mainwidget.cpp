#include "mainwidget.h"

MainWidget::MainWidget(int width, int height, QWidget *parent)
    : QWidget{parent}
{
    this->resize(width,height);
}
