// #include "widget.h"
#include "widgets/bodywidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    BodyWidget w(800,600,8);
    w.show();
    return a.exec();
}
