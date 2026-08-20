#include "conversationmanager.h"

ConversationManager &ConversationManager::getConversationManager()
{
    static ConversationManager cm;
    return cm;
}

ConversationItem *ConversationManager::getOrCreateConversationItem(const QString &conversationID)
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

void ConversationManager::startSyncMessage()
{
    QString syncConvSeq;
    DatabaseManager::getDatabaseManager().getMeta(DatabaseManager::SyncConvSeq, syncConvSeq);
    TcpLongConnection::getTcpClient().getConversationsFriendUIDListAndSeq(syncConvSeq.isEmpty() ? 0 : syncConvSeq.toLongLong());
}

void ConversationManager::cleanAll()
{
    DatabaseManager::getDatabaseManager().prepareToSwitchUser();
    for(auto it = this->conversations.begin(); it != this->conversations.end(); it++)
    {
        it.value()->deleteLater();
    }
    this->conversations.clear();
}

ConversationManager::ConversationManager(QObject *parent)
    : QObject{parent}
{
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::sendMessageStatus, this, [this]
            (bool isSuccess, QString tempMsgID, QString receiverUID, QString messageID, int64_t timeStamp, int64_t convSeq){
        ConversationItem* item = getOrCreateConversationItem(receiverUID);
        if(!item)
            return;
        if(messageID.isEmpty() || messageID == "0")
            messageID = QString();
        item->updateMessageStatus(isSuccess, tempMsgID, messageID, timeStamp, convSeq);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::pushMessage, this, [this]
            (QString senderUID, QString content, QString messageID, int64_t timeStamp, int64_t convSeq){
        Message msg;
        msg.senderUID = senderUID;
        msg.content = content;
        msg.convSeq = convSeq;
        msg.status = Status::Success;
        msg.serverMsgID = messageID;
        msg.timeStamp = timeStamp;

        ConversationItem* item = getOrCreateConversationItem(senderUID);
        item->addNewMessage(msg);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &ConversationManager::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &ConversationManager::cleanAll);
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, &ConversationManager::cleanAll);
}
