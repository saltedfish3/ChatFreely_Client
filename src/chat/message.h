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

struct Message
{
    QString serverMsgID;
    QString tempMsgID;

    int64_t timeStamp = 0;
    int64_t convSeq = 0;
    bool showTimestamp = false;

    QString senderUID;
    QString content;
    Status status = Success;

    QString username;
    QPixmap avatar;
};

#endif // MESSAGE_H
