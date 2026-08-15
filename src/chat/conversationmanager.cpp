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

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newSyncConvStatus, this, [this]
            (bool isSuccess, const QList<QString>& conversations, int64_t newSyncConvSeq){
        if(!isSuccess)
        {
            startSyncMessage();
            return;
        }

        if(conversations.isEmpty())
        {
            return;
        }

        DatabaseManager::getDatabaseManager().setMeta(DatabaseManager::SyncConvSeq, QString::number(newSyncConvSeq));
        for(const QString& conversationID : conversations)
        {
            ConversationItem* item = getOrCreateConversationItem(conversationID);
            if(!item)
                continue;
            qint64 maxConvSeq = -1;
            if(DatabaseManager::getDatabaseManager().getConversationsLastConvSeq(conversationID, maxConvSeq))
            {
                TcpLongConnection::getTcpClient().sendSyncNewMessages(conversationID, maxConvSeq);
            }
            else
            {
                qWarning() << "ConversationManager接收Tcp newSyncConvStatus信号getConversationsLastConvSeq失败：进行全量获取";
                TcpLongConnection::getTcpClient().sendSyncNewMessages(conversationID, 0);
            }
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::syncMessagesStatus, this, [this](bool isSuccess,
                                                                                    const QString& conversationID, const QList<Message>& msgs){
        if(!isSuccess)
        {
            qint64 maxConvSeq = -1;
            if(DatabaseManager::getDatabaseManager().getConversationsLastConvSeq(conversationID, maxConvSeq))
                TcpLongConnection::getTcpClient().sendSyncNewMessages(conversationID, maxConvSeq);
            else
            {
                qWarning() << "ConversationManager接收Tcp syncMessagesStatus信号getConversationsLastConvSeq失败：进行全量获取";
                TcpLongConnection::getTcpClient().sendSyncNewMessages(conversationID, 0);
            }
        }
        else
        {
            ConversationItem* item = getOrCreateConversationItem(conversationID);
            if(!item)
                return;

            for(const auto& msg : msgs)
            {
                item->addNewMessage(msg);
            }
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::reconnectSuccess, this, [this](){
        if(UserInfo::getUserInfo().isLogin())
            startSyncMessage();
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &ConversationManager::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &ConversationManager::cleanAll);
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, &ConversationManager::cleanAll);
}
