// #include "widget.h"
#include "widgets/bodywidget.h"

#include <QApplication>
#include "utils/GlobalVariable.h"
#include "network/tcplongconnection.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icon/images/favicon.ico"));
    QCoreApplication::setOrganizationName("ProChat");
    QCoreApplication::setApplicationName("ProChat");
    GlobalVariable::initGlobalSettings();

    TcpLongConnection::getTcpClient();
    HttpShortConnection::getHttpClient();

    BodyWidget w(800,600,8);
    w.show();
    return a.exec();
}
