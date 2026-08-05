#ifndef CONVERSATIONITEM_H
#define CONVERSATIONITEM_H

#include <QObject>
#include "message.h"
#include "messagesmanager.h"
#include "../network/tcplongconnection.h"
#include "../utils/userinfo.h"
#include "../utils/friendmanage.h"

class ConversationItem : public QObject
{
    Q_OBJECT
public:
    explicit ConversationItem(const QString& conversationID, QObject *parent = nullptr);
    QString getConversationID();
    const MessagesManager& getMessagesManager() const;
    Message getLastMessage() const;
    int getUnReadCount() const;
    void addNewMessage(const Message& msg);
    void updateMessageInfo(bool isSuccess, QString tempMsgID, QString receiverUID, QString messageID, int64_t timeStamp, int64_t convSeq);
    void updateSenderAvatar(const QString& senderUID, const QPixmap& avatar);
    bool isActive() const;
    void clearUnRead();
    void addUnReadCount();
    void setActive(bool isActive);

signals:
    void LastMessageChange(const Message& msg);
    void PushNewMessage(const Message& msg);
    void messageStatusChange(const QString& tempMsgID, Status status);
    void UnReadCountChange(int num);
    void senderAvatarUpdate(const QString& senderUID);

private:
    void setSenderInfo(Message& msg);

    QString conversationID;
    MessagesManager msgManager;
    int unRead = 0;
    bool active = false;
};

#endif // CONVERSATIONITEM_H
