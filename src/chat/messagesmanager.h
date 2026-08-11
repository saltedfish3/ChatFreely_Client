#ifndef MESSAGESMANAGER_H
#define MESSAGESMANAGER_H

#include <QObject>
#include <QHash>
#include "message.h"
#include "../database/databasemanager.h"

class MessagesManager : public QObject
{
    Q_OBJECT
public:
    explicit MessagesManager(const QString& conversationID, QObject* parent = nullptr);

    void addMessageBack(const Message& msg);
    void addMessageFront(const QList<Message>& msgs);
    bool updateMessageStatus(const QString& tempMsgID, const QString& newServerMsgID, Status status, int64_t newTimeStamp, int64_t newConvSeq);

    Message getLastMessage() const;
    Message getFrontMessage() const;
    const QList<Message>& getMessages() const;

    int indexOfMsg(const QString& msgID) const;
    void removeOfIndex(int index);

    void retryMessage(int index);

signals:
    void messageAdd(int row);
    void messagePrepend(int count);
    void messageUpdate(int row);
    void messageRemove(int row);
    void messageMove(int oldRow);

private:
    QList<Message> messages;
    QHash<QString, int> index_message;//消息索引(消息ID，行号)

    QString conversationID;

    void addIndex(const Message& msg, int index);
    void rebuildIndex();
};

#endif // MESSAGESMANAGER_H
