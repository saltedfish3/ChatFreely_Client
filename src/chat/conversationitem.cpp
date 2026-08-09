#include "conversationitem.h"

ConversationItem::ConversationItem(const QString& conversationID, QObject *parent)
    : conversationID(conversationID), QObject{parent}, msgManager(conversationID, this)
{
}

QString ConversationItem::getConversationID() const
{
    return this->conversationID;
}

MessagesManager &ConversationItem::getMessagesManager()
{
    return this->msgManager;
}

const MessagesManager &ConversationItem::getMessagesManager() const
{
    return this->msgManager;
}

Message ConversationItem::getLastMessage() const
{
    return msgManager.getLastMessage();
}

int ConversationItem::getUnReadCount() const
{
    return this->unRead;
}

void ConversationItem::addNewMessage(Message msg)
{
    const auto& msgs = this->msgManager.getMessages();
    if(msgs.isEmpty())
        msg.showTimestamp = true;
    else
    {
        const Message& lastMsg = msgs.last();
        if(lastMsg.timeStamp <= 0)
            msg.showTimestamp = false;
        else
            msg.showTimestamp = (msg.timeStamp - lastMsg.timeStamp) > 300;
    }

    this->msgManager.addMessageBack(msg);

    emit PushNewMessage(msg);
    emit LastMessageChange(msg);
}

void ConversationItem::updateMessageStatus(bool isSuccess, const QString& tempMsgID, const QString& messageID, int64_t timeStamp, int64_t convSeq)
{
    this->msgManager.updateMessageStatus(tempMsgID, messageID, isSuccess ? Success : Failed, timeStamp, convSeq);
}

bool ConversationItem::isActive() const
{
    return this->active;
}

void ConversationItem::clearUnRead()
{
    if(this->unRead != 0)
    {
        this->unRead = 0;

        emit UnReadCountChange(0);
        DatabaseManager::getDatabaseManager().addUpdateUnreadTask(this->conversationID, this->unRead);
    }
}

void ConversationItem::addUnReadCount()
{
    this->unRead++;
    emit UnReadCountChange(this->unRead);
    DatabaseManager::getDatabaseManager().addUpdateUnreadTask(this->conversationID, this->unRead);
}

void ConversationItem::setUnReadCount(int count)
{
    if(count < 0)
        count = 0;

    if(this->unRead == count)
        return;

    this->unRead = count;
    emit UnReadCountChange(this->unRead);
}

void ConversationItem::setActive(bool isActive)
{
    this->active = isActive;
}
