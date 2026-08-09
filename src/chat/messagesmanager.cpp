#include "messagesmanager.h"

MessagesManager::MessagesManager(const QString& conversationID, QObject* parent)
    : QObject(parent), conversationID(conversationID)
{
}

void MessagesManager::addMessageBack(const Message &msg)
{
    int row = this->messages.size();

    this->messages.append(msg);
    addIndex(msg, row);

    emit messageAdd(row);
    DatabaseManager::getDatabaseManager().addInsertMessageTask(this->conversationID, msg);
}

void MessagesManager::addMessageFront(const QList<Message>& msgs)
{
    if(msgs.isEmpty())
        return;

    this->messages = msgs + this->messages;

    rebuildIndex();
    emit messagePrepend(msgs.size());
}

bool MessagesManager::updateMessageStatus(const QString &tempMsgID, const QString &newServerMsgID, Status status, int64_t newTimeStamp, int64_t newConvSeq)
{
    auto it = this->index_message.find(tempMsgID);
    if(it == this->index_message.end())
        return false;

    int row = it.value();
    Message& msg = this->messages[row];
    msg.status = status;

    if(!newServerMsgID.isEmpty())
    {
        this->index_message.remove(tempMsgID);
        msg.serverMsgID = newServerMsgID;
        addIndex(msg, row);
    }
    if(newTimeStamp > 0)
        msg.timeStamp = newTimeStamp;
    if(newConvSeq > 0)
        msg.convSeq = newConvSeq;

    emit messageUpdate(row);
    DatabaseManager::getDatabaseManager().addUpdateMessageTask(this->conversationID, msg);
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

int MessagesManager::indexOfMsg(const QString &msgID) const
{
    auto it = this->index_message.find(msgID);
    return (it != this->index_message.end()) ? it.value() : -1;
}

void MessagesManager::removeOfIndex(int index)
{
    if(index < 0 || index >= this->messages.size())
        return;
    const Message& msg = this->messages.at(index);
    const QString& key = msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID;
    this->index_message.remove(key);
    this->messages.removeAt(index);

    rebuildIndex();
    emit messageRemove(index);
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
