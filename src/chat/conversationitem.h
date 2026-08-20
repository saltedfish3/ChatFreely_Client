#ifndef CONVERSATIONITEM_H
#define CONVERSATIONITEM_H

#include <QObject>
#include "message.h"
#include "messagesmanager.h"
#include "../database/databasemanager.h"

class ConversationItem : public QObject
{
    Q_OBJECT
public:
    explicit ConversationItem(const QString& conversationID, QObject *parent = nullptr);

    const MessagesManager& getMessagesManager() const;
    MessagesManager& getMessagesManager();
    Message getLastMessage() const;
    QString getConversationID() const;
    int getUnReadCount() const;

    void addNewMessage(Message msg);

    void updateMessageStatus(bool isSuccess, const QString& tempMsgID, const QString& messageID, int64_t timeStamp, int64_t convSeq);
    void loadHistoryMessages(int limit = 20);

    bool isActive() const;
    void clearUnRead();
    void addUnReadCount();
    void setUnReadCount(int count);
    void setActive(bool isActive);

signals:
    void PushNewMessage(const Message& msg);
    void LastMessageChange(const Message& msg);
    void UnReadCountChange(int num);
    //用于会话窗口减少loadingcount
    void messageStatusChange();
    void firstLoadingMessages();
    void historyMessagesload(int insertCount);

private:
    QString conversationID;
    MessagesManager msgManager;
    int unRead = 0;
    bool active = false;
    bool isFirstLoad = false;
};

#endif // CONVERSATIONITEM_H
