#include "GlobalVariable.h"

QString GlobalVariable::pos_downloadFile;
QString GlobalVariable::pos_chatRecord;
QString GlobalVariable::pos_imageCache;
bool GlobalVariable::isHaveGPU = false;
QString GlobalVariable::serverHost = "chatfreely.test";
int GlobalVariable::tcpPort = 9443;
bool GlobalVariable::useHttps = true;
QReadWriteLock GlobalVariable::rwLock;

void GlobalVariable::initGlobalSettings()
{
    //初始化 配置文件
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/record";
    QDir().mkpath(dataDir);
    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDir().mkpath(downloadDir);
    QString imageCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/images";
    QDir().mkpath(imageCacheDir);

    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ChatFreelyOrg", "ChatClient");
    if(!settings.contains("pos_downloadFile"))
        settings.setValue("pos_downloadFile", downloadDir);

    if(!settings.contains("pos_chatRecord"))
        settings.setValue("pos_chatRecord", dataDir);

    if(!settings.contains("pos_imageCache"))
        settings.setValue("pos_imageCache", imageCacheDir);

    pos_downloadFile = settings.value("pos_downloadFile").toString();
    pos_chatRecord = settings.value("pos_chatRecord").toString();
    pos_imageCache = settings.value("pos_imageCache").toString();

    if(!settings.contains("serverHost"))
        settings.setValue("serverHost", serverHost);

    if(!settings.contains("tcpPort"))
        settings.setValue("tcpPort", tcpPort);

    if(!settings.contains("useHttps"))
        settings.setValue("useHttps", useHttps);

    serverHost = settings.value("serverHost").toString();
    tcpPort = settings.value("tcpPort").toInt();
    useHttps = settings.value("useHttps").toBool();
}

QString GlobalVariable::getPosOfDownloadFile()
{
    QReadLocker locker(&rwLock);
    return pos_downloadFile;
}

void GlobalVariable::setPosOfDownloadFile(const QString &dir)
{
    QWriteLocker locker(&rwLock);
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ChatFreelyOrg", "ChatClient");
    settings.setValue("pos_downloadFile", dir);
    pos_downloadFile = dir;
    settings.sync();
}

bool GlobalVariable::isDownloadFileExists(const QString &filename)
{
    QReadLocker locker(&rwLock);
    QFileInfo info(pos_downloadFile + "/" + filename);
    if(info.exists())
        return true;
    return false;
}

QString GlobalVariable::getPosOfChatRecord()
{
    QReadLocker locker(&rwLock);
    return pos_chatRecord;
}

void GlobalVariable::setPosOfChatRecord(const QString &dir)
{
    QWriteLocker locker(&rwLock);
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ChatFreelyOrg", "ChatClient");

    QString finalPath = QDir(dir).filePath("record");
    QDir().mkpath(finalPath);

    settings.setValue("pos_chatRecord", finalPath);
    pos_chatRecord = finalPath;
    settings.sync();
}

QString GlobalVariable::getPosOfImageCache()
{
    QReadLocker locker(&rwLock);
    return pos_imageCache;
}

QString GlobalVariable::getChatRecordSize()
{
    QReadLocker locker(&rwLock);
    qint64 totalSize = 0;
    QDirIterator it(pos_chatRecord,QDir::Files|QDir::Hidden|QDir::NoSymLinks,QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        it.next();
        totalSize += it.fileInfo().size();
    }
    const qint64 KB = 1024;
    const qint64 MB = 1024*KB;
    const qint64 GB = 1024*MB;

    if(totalSize >= GB)
        return QString::number(totalSize / (double)GB,'f',2) + "GB";
    else if(totalSize >= MB)
        return QString::number(totalSize / (double)MB,'f',2) + "MB";
    else if(totalSize >= KB)
        return QString::number(totalSize / (double)KB,'f',2) + "KB";
    return QString::number(totalSize) + "B";
}

QString GlobalVariable::getImageCacheSize()
{
    QReadLocker locker(&rwLock);
    qint64 totalSize = 0;
    QDirIterator it(pos_imageCache,QDir::Files|QDir::Hidden|QDir::NoSymLinks,QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        it.next();
        totalSize += it.fileInfo().size();
    }
    const qint64 KB = 1024;
    const qint64 MB = 1024*KB;
    const qint64 GB = 1024*MB;

    if(totalSize >= GB)
        return QString::number(totalSize / (double)GB,'f',2) + "GB";
    else if(totalSize >= MB)
        return QString::number(totalSize / (double)MB,'f',2) + "MB";
    else if(totalSize >= KB)
        return QString::number(totalSize / (double)KB,'f',2) + "KB";
    return QString::number(totalSize) + "B";
}

void GlobalVariable::clearImageCache()
{
    QWriteLocker locker(&rwLock);
    if(pos_imageCache.isEmpty())
        return;

    QDir dir(pos_imageCache);
    if(dir.exists())
    {
        dir.removeRecursively();
        dir.mkdir(".");
    }
}

void GlobalVariable::setMigrationState(bool isMigrating, const QString &oldPath, const QString &newPath)
{
    QWriteLocker locker(&rwLock);
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ChatFreelyOrg", "ChatClient");
    settings.setValue("isMigrating", isMigrating);
    if(isMigrating)
    {
        settings.setValue("migrating_old", oldPath);
        settings.setValue("migrating_new", newPath);
    }
    else
    {
        settings.remove("migrating_old");
        settings.remove("migrating_new");
    }
    settings.sync();
}

bool GlobalVariable::isMigrating()
{
    QReadLocker locker(&rwLock);
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ChatFreelyOrg", "ChatClient");

    return settings.value("isMigrating", false).toBool();
}

QString GlobalVariable::getMigratingOldPath()
{
    QReadLocker locker(&rwLock);
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ChatFreelyOrg", "ChatClient");
    return settings.value("migrating_old", "").toString();
}

QString GlobalVariable::getMigratingNewPath()
{
    QReadLocker locker(&rwLock);
    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ChatFreelyOrg", "ChatClient");
    return settings.value("migrating_new", "").toString();
}

void GlobalVariable::cleanMigratingTemp(const QString &tempPath)
{
    QDir(tempPath).removeRecursively();
}

qreal GlobalVariable::getMaxDevicePixelRatio()
{
    qreal dpr = 1.0;
    const auto screens = QGuiApplication::screens();
    for(const QScreen* screen : screens)
        dpr = qMax(dpr, screen->devicePixelRatio());
    return dpr;
}

bool GlobalVariable::hasGPU()
{
    QReadLocker locker(&rwLock);
    return isHaveGPU;
}

void GlobalVariable::setHasGPU(bool gpu)
{
    QWriteLocker locker(&rwLock);
    isHaveGPU = gpu;
}

QString GlobalVariable::getServerHost()
{
    QReadLocker locker(&rwLock);
    return serverHost;
}

int GlobalVariable::getTcpPort()
{
    QReadLocker locker(&rwLock);
    return tcpPort;
}

QString GlobalVariable::getHttpBaseUrl()
{
    QReadLocker locker(&rwLock);
    QString pre = useHttps ? "https://" : "http://";
    return pre + serverHost;
}

void GlobalVariable::updateServerConfig(const QString &host, int tcpPort, bool useHttps)
{
    if(host.isEmpty() || tcpPort <= 0)
        return;

    GlobalVariable::serverHost = host;
    GlobalVariable::tcpPort = tcpPort;
    GlobalVariable::useHttps = useHttps;

    QSettings settings(QSettings::NativeFormat, QSettings::UserScope, "ChatFreelyOrg", "ChatClient");
    settings.setValue("serverHost", GlobalVariable::serverHost);
    settings.setValue("tcpPort", GlobalVariable::tcpPort);
    settings.setValue("useHttps", GlobalVariable::useHttps);
    settings.sync();
}
