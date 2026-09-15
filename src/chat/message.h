#ifndef MESSAGE_H
#define MESSAGE_H

#include <QString>
#include <QPixmap>

enum Status
{
    Sending = 0,
    Success,
    Failed
};

enum ContentType
{
    Text = 0,
    Image
};

struct MediaInfo
{
    QString url;
    int width = 0;
    int height = 0;
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
};

#endif // MESSAGE_H
