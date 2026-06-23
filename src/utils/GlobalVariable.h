#ifndef GLOBALVARIABLE_H
#define GLOBALVARIABLE_H

#define AppVersion "1.0"
#define AppChineseName "畅聊"
#define AppEnglishName "ChatFreely"

#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include <QFileInfo>
#include <QDirIterator>

class GlobalVariable
{
public:
    enum MainPage
    {
        Login = 0,
        Register,
        Main
    };

    GlobalVariable(const GlobalVariable&) = delete;
    GlobalVariable& operator=(const GlobalVariable&) = delete;

    static GlobalVariable& initGlobalSettings();
    static QString getPosOfDownloadFile();
    static void setPosOfDownloadFile(const QString& dir);

    static QString getPosOfChatRecord();
    static void setPosOfChatRecord(const QString& dir);

    static QString getChatRecordSize();
    static void clearAllChatRecord();

private:
    GlobalVariable();

    static GlobalVariable myself;

    static QString pos_ini;
    static QString pos_downloadFile;
    static QString pos_chatRecord;
};



#endif // GLOBALVARIABLE_H
