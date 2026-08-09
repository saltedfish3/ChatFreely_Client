// #include "widget.h"
#include "widgets/bodywidget.h"

#include <QApplication>
#include "utils/GlobalVariable.h"
#include "network/tcplongconnection.h"
#include "database/databasemanager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icon/images/favicon.ico"));
    QCoreApplication::setOrganizationName("ProChat");
    QCoreApplication::setApplicationName("ChatFreely");
    GlobalVariable::initGlobalSettings();

    TcpLongConnection::getTcpClient();
    HttpShortConnection::getHttpClient();
    DatabaseManager::getDatabaseManager();

    BodyWidget w(800,600,8);
    w.show();
    return a.exec();
}
