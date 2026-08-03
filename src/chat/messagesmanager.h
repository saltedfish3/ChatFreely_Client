#ifndef MESSAGESMANAGER_H
#define MESSAGESMANAGER_H

#include <QHash>
#include "message.h"

class MessagesManager
{
public:
    MessagesManager();
    void append(const Message& msg);
    bool updateStatus(const QString& tempMsgID, const QString& newServerMsgID, Status status, int64_t newTimeStamp, int64_t newConvSeq);
    Message getLastMessage() const;
    const QList<Message>& getMessages() const;
    int indexOfMsg(const QString& msgID) const;

private:
    QList<Message> messages;
    QHash<QString, int> index_message;//消息索引(消息ID，行号)

    void addIndex(const Message& msg, int index);
    void rebuildIndex();
};

#endif // MESSAGESMANAGER_H
