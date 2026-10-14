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
    enum class ConversationType
    {
        Personal,
        Group
    };
    explicit ConversationItem(const QString& conversationID, ConversationType type, QObject *parent = nullptr);

    const MessagesManager& getMessagesManager() const;
    MessagesManager& getMessagesManager();
    Message getLastMessage() const;
    QString getConversationID() const;
    int getUnReadCount() const;
    ConversationType getConversationType();

    void addNewMessage(Message msg);

    void updateMessageStatus(bool isSuccess, const QString& tempMsgID, const QString& messageID, int64_t timeStamp, int64_t convSeq);
    void updateMessageContent(const QString& tempMsgID, const QString& content);

    bool cancelUpload(const QString& tempMsgID);
    void reloadImage(const QString& msgID);

    bool isActive() const;
    void clearUnRead();
    void clearMessages();
    void addUnReadCount();
    void setUnReadCount(int count);
    void setActive(bool isActive);

signals:
    void PushNewMessage(const Message& msg);
    void LastMessageChange(const Message& msg);
    void UnReadCountChange(int num);
    //用于会话窗口减少loadingcount
    void messageStatusChange();
    void startLoadingImage();
    void finishLoadedImage();
    void loadMessagesFinished(MessagesManager::LoadDir dir);
    void messagesInserted();

    void jumpToFinished(int row, const QString& msgID);
    void jumpToFailed(const QString& msgID);

private:
    QString conversationID;
    MessagesManager msgManager;
    int unRead = 0;
    bool active = false;
    bool isFirstLoad = false;

    ConversationType type = ConversationType::Personal;

};

#endif // CONVERSATIONITEM_H
