#include "GlobalVariable.h"

QString GlobalVariable::pos_ini;
QString GlobalVariable::pos_downloadFile;
QString GlobalVariable::pos_chatRecord;
QString GlobalVariable::pos_imageCache;
GlobalVariable GlobalVariable::myself;

GlobalVariable::GlobalVariable()
{
    //初始化 配置文件
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    this->pos_ini = configDir + "/config.ini";

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/record";
    QDir().mkpath(dataDir);
    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDir().mkpath(downloadDir);
    QString imageCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/images";
    QDir().mkpath(imageCacheDir);

    if(!QFileInfo::exists(this->pos_ini))
    {
        QSettings settings(this->pos_ini,QSettings::IniFormat);
        settings.setValue("pos_downloadFile",downloadDir);
        settings.setValue("pos_chatRecord",dataDir);
        settings.setValue("pos_imageCache",imageCacheDir);
        this->pos_downloadFile = downloadDir;
        this->pos_chatRecord = dataDir;
        this->pos_imageCache = imageCacheDir;
    }
    else
    {
        QSettings settings(this->pos_ini,QSettings::IniFormat);
        this->pos_downloadFile = settings.value("pos_downloadFile").toString();
        if(this->pos_downloadFile.isEmpty())
        {
            settings.setValue("pos_downloadFile",downloadDir);
            this->pos_downloadFile = downloadDir;
        }
        this->pos_chatRecord = settings.value("pos_chatRecord").toString();
        if(this->pos_chatRecord.isEmpty())
        {
            settings.setValue("pos_chatRecord",dataDir);
            this->pos_chatRecord = dataDir;
        }
        this->pos_imageCache = settings.value("pos_imageCache").toString();
        if(this->pos_imageCache.isEmpty())
        {
            settings.setValue("pos_imageCache",imageCacheDir);
            this->pos_imageCache = imageCacheDir;
        }
    }
}

GlobalVariable &GlobalVariable::initGlobalSettings()
{
    static GlobalVariable myself;
    return myself;
}

QString GlobalVariable::getPosOfDownloadFile()
{
    return pos_downloadFile;
}

void GlobalVariable::setPosOfDownloadFile(const QString &dir)
{
    QSettings settings(pos_ini,QSettings::IniFormat);
    settings.setValue("pos_downloadFile",pos_downloadFile);
}

QString GlobalVariable::getPosOfChatRecord()
{
    return pos_chatRecord;
}

void GlobalVariable::setPosOfChatRecord(const QString &dir)
{
    QSettings settings(pos_ini,QSettings::IniFormat);
    settings.setValue("pos_chatRecord", dir + "/record");
    pos_chatRecord = dir + "/record";
}

QString GlobalVariable::getPosOfImageCache()
{
    return pos_imageCache;
}

QString GlobalVariable::getChatRecordSize()
{
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
    QSettings settings(pos_ini, QSettings::IniFormat);
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
}

bool GlobalVariable::isMigrating()
{
    QSettings settings(pos_ini, QSettings::IniFormat);
    return settings.value("isMigrating", false).toBool();
}

QString GlobalVariable::getMigratingOldPath()
{
    QSettings settings(pos_ini, QSettings::IniFormat);
    return settings.value("migrating_old", "").toString();
}

QString GlobalVariable::getMigratingNewPath()
{
    QSettings settings(pos_ini, QSettings::IniFormat);
    return settings.value("migrating_new", "").toString();
}

void GlobalVariable::cleanMigratingTemp(const QString &tempPath)
{
    QDir(tempPath).removeRecursively();
}
