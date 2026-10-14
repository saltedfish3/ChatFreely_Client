#include "conversationitem.h"

ConversationItem::ConversationItem(const QString& conversationID, ConversationType type, QObject *parent)
    : conversationID(conversationID), QObject{parent}, msgManager(conversationID, this), type(type)
{
    connect(&this->msgManager, &MessagesManager::finishLoadedImage, this, [this](){
        emit finishLoadedImage();
    });

    connect(&this->msgManager, &MessagesManager::startLoadingImage, this, [this](){
        emit startLoadingImage();
    });

    connect(&this->msgManager, &MessagesManager::loadMessagesFinished, this, [this](MessagesManager::LoadDir dir){
        emit loadMessagesFinished(dir);
    });

    connect(&this->msgManager, &MessagesManager::messagesInserted, this, [this](){
        emit messagesInserted();
    });

    connect(&this->msgManager, &MessagesManager::jumpToFailed, this, [this](const QString& msgID){
        emit jumpToFailed(msgID);
    });

    connect(&this->msgManager, &MessagesManager::jumpToFinished, this, [this](int row, const QString& msgID){
        emit jumpToFinished(row, msgID);
    });
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

ConversationItem::ConversationType ConversationItem::getConversationType()
{
    return this->type;
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

void ConversationItem::updateMessageContent(const QString &tempMsgID, const QString &content)
{
    this->msgManager.updateMessageContent(tempMsgID, content);
}

bool ConversationItem::cancelUpload(const QString &tempMsgID)
{
    bool result = this->msgManager.cancelUpload(tempMsgID);
    emit messageStatusChange();
    return result;
}

void ConversationItem::reloadImage(const QString &msgID)
{
    int index = this->msgManager.indexOfMsg(msgID);
    if(index < 0)
        return;

    this->msgManager.reLoadImage(index);
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

void ConversationItem::clearMessages()
{
    this->msgManager.clearMessages();
    this->unRead = 0;
    emit UnReadCountChange(0);
    emit LastMessageChange(Message());
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
