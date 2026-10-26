#include "conversationmanager.h"

ConversationManager &ConversationManager::getConversationManager()
{
    static ConversationManager cm;
    return cm;
}

ConversationItem *ConversationManager::getOrCreateConversationItem(const QString &conversationID, ConversationItem::ConversationType type)
{
    auto it = this->conversations.find(conversationID);
    if(it == this->conversations.end())
    {
        bool isDeleted = false;

        if(!GroupManager::getGroupManage().getGroupInfo(conversationID).conversationID.isEmpty())
            type = ConversationItem::ConversationType::Group;
        else
        {
            FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(conversationID);
            if(!info.uid.isEmpty())
            {
                type = ConversationItem::ConversationType::Personal;
                isDeleted = info.isDeleted;
            }
        }

        ConversationItem* item = new ConversationItem(conversationID, type, isDeleted, this);
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
            (bool isSuccess, const QString& tempMsgID,const QString& conversationID, const QString& messageID, int64_t timeStamp, int64_t convSeq){

        ConversationItem::ConversationType conversationType = GroupManager::getGroupManage().getGroupInfo(conversationID).conversationID.isEmpty() ?
                                                                  ConversationItem::ConversationType::Personal : ConversationItem::ConversationType::Group;

        ConversationItem* item = getOrCreateConversationItem(conversationID, conversationType);
        if(!item)
            return;

        item->updateMessageStatus(isSuccess, tempMsgID, messageID.isEmpty() || messageID == "0" ? QString() : messageID, timeStamp, convSeq);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::pushMessage, this, [this]
            (const QString& conversationID, const QString& senderUID, ContentType type, const QString& content, const QString& messageID, int64_t timeStamp, int64_t convSeq){
        Message msg;
        msg.senderUID = senderUID;
        msg.contentType = type;
        msg.content = content;
        msg.convSeq = convSeq;
        msg.status = Status::Success;
        msg.serverMsgID = messageID;
        msg.timeStamp = timeStamp;

        msg.parseMedia();

        ConversationItem::ConversationType conversationType = GroupManager::getGroupManage().getGroupInfo(conversationID).conversationID.isEmpty() ?
                                                                  ConversationItem::ConversationType::Personal : ConversationItem::ConversationType::Group;

        ConversationItem* item = getOrCreateConversationItem(conversationID, conversationType);
        item->addNewMessage(msg);
    });

    connect(&DatabaseManager::getDatabaseManager(), &DatabaseManager::allChatMessagesClean, this, [this](){
        for(auto* it : std::as_const(this->conversations))
            it->clearMessages();
    });

    connect(&GroupManager::getGroupManage(), &GroupManager::groupCreated, this, [this](const QString& conversationID){
        if(conversationID.isEmpty())
            return;

        ConversationItem* item = getOrCreateConversationItem(conversationID, ConversationItem::ConversationType::Group);
    });

    connect(&FriendManage::getFriendManage(), &FriendManage::allFriendList, this, [this](){
        for(auto* item : std::as_const(this->conversations))
        {
            if(!item || item->getConversationType() != ConversationItem::ConversationType::Personal)
                continue;

            FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(item->getConversationID());
            item->setIsDeleted(info.isDeleted);
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &ConversationManager::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &ConversationManager::cleanAll);
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, &ConversationManager::cleanAll);
}
