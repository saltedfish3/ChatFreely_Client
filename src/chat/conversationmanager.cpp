#include "conversationmanager.h"

ConversationManager &ConversationManager::getConversationManager()
{
    static ConversationManager cm;
    return cm;
}

ConversationItem *ConversationManager::getConversationItem(const QString &conversationID)
{
    auto it = this->conversations.find(conversationID);
    if(it == this->conversations.end())
    {
        ConversationItem* item = new ConversationItem(conversationID, this);
        this->conversations[conversationID] = item;
        emit conversationCreated(item);
        return item;
    }
    return it.value();
}

void ConversationManager::cleanAll()
{
    for(auto it = this->conversations.cbegin(); it != this->conversations.cend(); it++)
        it.value()->deleteLater();
    this->conversations.clear();
}

ConversationManager::ConversationManager(QObject *parent)
    : QObject{parent}
{
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::sendMessageStatus, [this]
            (bool isSuccess, QString tempMsgID, QString receiverUID, QString messageID, int64_t timeStamp, int64_t convSeq){
        ConversationItem* item = getConversationItem(receiverUID);
        item->updateMessageInfo(isSuccess, tempMsgID, receiverUID, messageID, timeStamp, convSeq);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::pushMessage, [this]
            (QString senderUID, QString content, QString messageID, int64_t timeStamp, int64_t convSeq){
                Message msg;
                msg.senderUID = senderUID;
                msg.content = content;
                msg.convSeq = convSeq;
                msg.status = Status::Success;
                msg.serverMsgID = messageID;
                msg.timeStamp = timeStamp;

                ConversationItem* item = getConversationItem(senderUID);
                item->addNewMessage(msg);
    });

    connect(&UserInfo::getUserInfo(), &UserInfo::updateAvatar, this, [this](QPixmap avatar){
        for(auto it = this->conversations.begin(); it != this->conversations.end(); it++)
            it.value()->updateSenderAvatar(UserInfo::getUserInfo().getUID(), avatar);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &ConversationManager::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &ConversationManager::cleanAll);
}
