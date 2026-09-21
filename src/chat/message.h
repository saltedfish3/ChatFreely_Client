#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QPixmap>
#include <QJsonObject>
#include <QJsonDocument>

enum Status
{
    Sending = 0,
    Success,
    Failed
};

enum ContentType
{
    Text = 0,
    Image,
    Video,
    File
};

struct MediaInfo
{
    QString url;

    //视频
    int width = 0;
    int height = 0;
    qint64 duration = 0;
    QString thumbnailUrl;

    //文件
    QString name;

    qint64 size = 0;
};

struct Message
{
    QString serverMsgID;
    QString tempMsgID;

    int64_t timeStamp = 0;
    int64_t convSeq = 0;
    bool showTimestamp = false;

    QString senderUID;
    ContentType contentType = Text;
    QString content;
    MediaInfo info;
    Status status = Success;

    void parseMedia();
};

inline void Message::parseMedia()
{
    if(content.isEmpty())
        return;

    if(contentType == Text)
        return;

    if(!content.startsWith("{"))
    {
        info.url = content;
        return;
    }

    QJsonObject obj = QJsonDocument::fromJson(content.toUtf8()).object();

    if(contentType == Image)
    {
        info.url = obj["Url"].toString();
        info.width = obj["Width"].toString().toInt();
        info.height = obj["Height"].toString().toInt();
    }
    else if(contentType == Video)
    {
        info.url = obj["Url"].toString();
        info.width = obj["Width"].toString().toInt();
        info.height = obj["Height"].toString().toInt();
        info.duration = obj["Duration"].toString().toLongLong();
        info.thumbnailUrl = obj["ThumbnailUrl"].toString();
        info.size = obj["Size"].toString().toLongLong();
    }
    else if(contentType == File)
    {
        info.url = obj["Url"].toString();
        info.name = obj["Name"].toString();
        info.size = obj["Size"].toString().toLongLong();
    }
}

#endif // MESSAGE_H
