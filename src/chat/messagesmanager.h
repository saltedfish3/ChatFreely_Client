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

    void addMessage(const Message& msg, bool isStoreDB = true);
    void addMessages(const QList<Message>& msgs, bool isStoreDB = true);

    bool updateMessageStatus(const QString& tempMsgID, const QString& newServerMsgID, Status status, int64_t newTimeStamp, int64_t newConvSeq);

    Message getLastMessage() const;
    Message getFrontMessage() const;
    const QList<Message>& getMessages() const;

    int indexOfMsg(const QString& msgID) const;
    void removeOfIndex(int index);

    void retryMessage(int index);
    qint64 getNextConvSeq();

signals:
    void messageAdd(int row);
    void messagePrepend(int count);
    void messageUpdate(int row);
    void messageRemove(int row);
    void messageMove(int oldRow, int newRow);

private:
    QList<Message> messages;
    QHash<QString, int> index_message;//消息索引(消息ID，行号)

    QString conversationID;
    qint64 lastConvSeq = 0;

    int findInsertIndex(qint64 convSeq) const;
    void saveMoveMessage(int row);
    void calcShowTimestamp(int row);
    void addIndex(const Message& msg, int index);
    void rebuildIndex();
};

#endif // MESSAGESMANAGER_H
