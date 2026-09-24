// #include "widget.h"
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QSurfaceFormat>

#include "widgets/bodywidget.h"
#include <QApplication>
#include "utils/GlobalVariable.h"
#include "network/tcplongconnection.h"
#include "database/databasemanager.h"

bool isOpenGLUsable()
{
    QOpenGLContext context;
    if(!context.create())
        return false;

    QOffscreenSurface surface;
    surface.create();
    if(!surface.isValid())
        return false;

    if(!context.makeCurrent(&surface))
        return false;

    bool have = context.format().majorVersion() >= 2;
    context.doneCurrent();

    return have;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icon/images/favicon.ico"));
    QCoreApplication::setOrganizationName("ProChat");
    QCoreApplication::setApplicationName("ChatFreely");
    GlobalVariable::initGlobalSettings();
    GlobalVariable::setHasGPU(isOpenGLUsable());
    qDebug()<< "是否使用GPU: " << GlobalVariable::hasGPU();

    TcpLongConnection::getTcpClient();
    HttpShortConnection::getHttpClient();
    DatabaseManager::getDatabaseManager();
    FriendManage::getFriendManage();
    UserInfo::getUserInfo();
    ImageCacheManager::getManager();
    ImagePreviewWidget::getPreviewWidget();
    VideoPreviewWidget::getPreviewWidget();
    GlobalInitController::getController();

    BodyWidget w(800,600,8);
    w.show();

    int ret = a.exec();

    std::fflush(stderr);
    std::fflush(stdout);
    std::_Exit(ret);
}
