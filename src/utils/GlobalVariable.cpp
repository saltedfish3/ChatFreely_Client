#include "GlobalVariable.h"

QString GlobalVariable::pos_ini;
QString GlobalVariable::pos_downloadFile;
QString GlobalVariable::pos_chatRecord;
GlobalVariable GlobalVariable::myself;

GlobalVariable::GlobalVariable()
{
    //初始化 配置文件
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    this->pos_ini = configDir + "/config.ini";

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QDir().mkpath(downloadDir);

    if(!QFileInfo::exists(this->pos_ini))
    {
        QSettings settings(this->pos_ini,QSettings::IniFormat);
        settings.setValue("pos_downloadFile",downloadDir);
        settings.setValue("pos_chatRecord",dataDir);
        this->pos_downloadFile = downloadDir;
        this->pos_chatRecord = dataDir;
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
    settings.setValue("pos_chatRecord",pos_chatRecord);
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

void GlobalVariable::clearAllChatRecord()
{
    QDir dir(pos_chatRecord);
    if(dir.removeRecursively())
    {
        dir.mkdir(".");
    }
}
