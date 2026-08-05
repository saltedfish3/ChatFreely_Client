#include "messagesmanager.h"

MessagesManager::MessagesManager() {}

void MessagesManager::append(const Message &msg)
{
    addIndex(msg, this->messages.size());
    this->messages.append(msg);
}

bool MessagesManager::updateStatus(const QString &tempMsgID, const QString &newServerMsgID, Status status, int64_t newTimeStamp, int64_t newConvSeq)
{
    auto it = this->index_message.find(tempMsgID);
    if(it == this->index_message.end() || newServerMsgID.isEmpty() || newTimeStamp <= 0 || newConvSeq <= 0)
        return false;

    int row = it.value();
    Message& msg = this->messages[row];
    msg.status = status;
    msg.serverMsgID = newServerMsgID;
    msg.timeStamp = newTimeStamp;
    msg.convSeq = newConvSeq;

    this->index_message.remove(tempMsgID);
    addIndex(msg, row);

    return true;
}

Message MessagesManager::getLastMessage() const
{
    return this->messages.isEmpty() ? Message() : this->messages.last();
}

const QList<Message> &MessagesManager::getMessages() const
{
    return this->messages;
}

QList<Message> &MessagesManager::getMessages()
{
    return this->messages;
}

int MessagesManager::indexOfMsg(const QString &msgID) const
{
    auto it = this->index_message.find(msgID);
    return  (it != this->index_message.end()) ? it.value() : -1;
}

void MessagesManager::addIndex(const Message &msg, int index)
{
    this->index_message[msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID] = index;
}

void MessagesManager::rebuildIndex()
{
    this->index_message.clear();
    for(int i = 0; i < this->messages.size(); i++)
    {
        const Message& msg = this->messages[i];
        addIndex(msg, i);
    }
}
