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
    this->msgManager.addMessage(msg);

    if(msg.senderUID != UserInfo::getUserInfo().getUID() && !this->isActive())
        addUnReadCount();

    emit PushNewMessage(msg);
    emit LastMessageChange(msg);
}

void ConversationItem::updateMessageStatus(bool isSuccess, const QString& tempMsgID, const QString& messageID, int64_t timeStamp, int64_t convSeq)
{
    this->msgManager.updateMessageStatus(tempMsgID, messageID, isSuccess ? Success : Failed, timeStamp, convSeq);
    emit messageStatusChange();
}

void ConversationItem::loadHistoryMessages(int limit)
{
    Message msg = this->getMessagesManager().getFrontMessage();
    qint64 endConvSeq = -1;
    if(!msg.senderUID.isEmpty())
        endConvSeq = msg.convSeq;
    DatabaseManager::getDatabaseManager().loadConversationMessages(this->conversationID, 20, endConvSeq, [this](const QList<Message>& msgs){
        if(msgs.isEmpty())
            return;
        this->getMessagesManager().addMessages(msgs, false);
        emit historyMessagesload(msgs.size());
        if(!this->isFirstLoad)
        {
            this->isFirstLoad = true;
            emit firstLoadingMessages();
        }
    });
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
