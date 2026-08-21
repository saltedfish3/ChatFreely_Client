#include "messagesmanager.h"

MessagesManager::MessagesManager(const QString& conversationID, QObject* parent)
    : QObject(parent), conversationID(conversationID)
{
}

void MessagesManager::addMessage(const Message &msg, bool isStoreDB)
{
    if(msg.status == Success)
    {
        //消息去重
        bool isExist = false;
        if(!msg.serverMsgID.isEmpty())
            isExist = indexOfMsg(msg.serverMsgID) == -1 ? false : true;
        if(!isExist && !msg.tempMsgID.isEmpty())
            isExist = indexOfMsg(msg.tempMsgID) == -1 ? false : true;

        if(isExist)
            return;
    }

    int index = findInsertIndex(msg.convSeq);

    this->messages.insert(index, msg);
    rebuildIndex();

    if(msg.status == Success && msg.convSeq > this->lastConvSeq)
        this->lastConvSeq = msg.convSeq;

    if(isStoreDB)
        calcShowTimestamp(index);

    emit messageAdd(index);
    if(isStoreDB)
        DatabaseManager::getDatabaseManager().addInsertMessageTask(this->conversationID, this->messages.at(index));
}

void MessagesManager::addMessages(const QList<Message> &msgs, bool isStoreDB)
{
    if(msgs.isEmpty())
        return;
    for(const auto& msg : msgs)
        addMessage(msg, isStoreDB);
}

bool MessagesManager::updateMessageStatus(const QString &tempMsgID, const QString &newServerMsgID, Status status, int64_t newTimeStamp, int64_t newConvSeq)
{
    auto it = this->index_message.find(tempMsgID);
    if(it == this->index_message.end())
        return false;

    int row = it.value();
    Message& msg = this->messages[row];
    msg.status = status;

    if(status != Failed)
    {
        if(!newServerMsgID.isEmpty())
        {
            this->index_message.remove(tempMsgID);
            msg.serverMsgID = newServerMsgID;
            addIndex(msg, row);
        }
        if(newTimeStamp > 0)
            msg.timeStamp = newTimeStamp;

        if(newConvSeq > 0)
        {
            if(newConvSeq > this->lastConvSeq)
                this->lastConvSeq = newConvSeq;
            if(newConvSeq != msg.convSeq)
            {
                msg.convSeq = newConvSeq;
                QString messageID = msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID;
                int oldRow = row;

                saveMoveMessage(row);

                int newRow = indexOfMsg(messageID);
                if(newRow < 0)
                    return true;

                if(newRow == oldRow)
                {
                    calcShowTimestamp(newRow);
                    DatabaseManager::getDatabaseManager().addUpdateMessageTask(this->conversationID, this->messages.at(newRow));
                    emit messageUpdate(newRow);
                }
                else
                {
                    //防止更新到特别前面导致时间标签错乱
                    int edge = this->messages.size() - 1;
                    int start = std::max(std::min(oldRow, newRow), 0);
                    int end = std::min(std::max(oldRow, newRow) + 1, edge);
                    for(int i = start; i <= end; i++)
                    {
                        calcShowTimestamp(i);
                        DatabaseManager::getDatabaseManager().addUpdateMessageTask(this->conversationID, this->messages.at(i));
                        emit messageUpdate(i);
                    }
                }
                return true;
            }
        }
    }

    if(newTimeStamp > 0)
        calcShowTimestamp(row);

    emit messageUpdate(row);
    DatabaseManager::getDatabaseManager().addUpdateMessageTask(this->conversationID, msg);
    return true;
}

void MessagesManager::clearMessages()
{
    this->messages.clear();
    this->index_message.clear();
    emit resetModel();
}

Message MessagesManager::getLastMessage() const
{
    return this->messages.isEmpty() ? Message() : this->messages.last();
}

Message MessagesManager::getFrontMessage() const
{
    return this->messages.isEmpty() ? Message() : this->messages.front();
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

    if(index < this->messages.size())
    {
        calcShowTimestamp(index);
        DatabaseManager::getDatabaseManager().addUpdateMessageTask(this->conversationID, this->messages.at(index));
        emit messageUpdate(index);
    }
}

void MessagesManager::retryMessage(int index)
{
    if(this->messages.isEmpty() || index < 0 || index >= this->messages.size())
        return;

    Message& msg = this->messages[index];
    int oldRow = index;

    msg.status = Sending;
    msg.timeStamp = QDateTime::currentSecsSinceEpoch();
    msg.serverMsgID.clear();
    msg.convSeq = getNextConvSeq();

    //拷贝值，移动后引用指向的不是源数据
    QString tempMsgID = msg.tempMsgID;
    bool haveNext = (index < this->messages.size() - 1);

    saveMoveMessage(index);

    int newRow = indexOfMsg(tempMsgID);
    if(newRow < 0)
        return;

    calcShowTimestamp(newRow);
    DatabaseManager::getDatabaseManager().addUpdateMessageTask(this->conversationID, this->messages.at(newRow));
    emit messageUpdate(newRow);

    //更新原下一条消息时间标签
    if(haveNext && newRow != oldRow)
    {
        calcShowTimestamp(oldRow);
        DatabaseManager::getDatabaseManager().addUpdateMessageTask(this->conversationID, this->messages.at(oldRow));
        emit messageUpdate(oldRow);
    }
}

qint64 MessagesManager::getNextConvSeq()
{
    if(!this->messages.isEmpty())
    {
        const Message& last = this->messages.last();
        if(last.status != Success)
            return last.convSeq;

        if(last.convSeq > this->lastConvSeq)
            this->lastConvSeq = last.convSeq;
    }
    return this->lastConvSeq + 1;
}

int MessagesManager::findInsertIndex(qint64 convSeq) const
{
    int low = 0;
    int high = this->messages.size();
    while(low < high)
    {
        int mid = (low + high) / 2;
        if(this->messages.at(mid).convSeq <= convSeq)
            low = mid + 1;
        else
            high = mid;
    }

    return low;
}

void MessagesManager::saveMoveMessage(int row)
{
    if(row < 0 || row >= this->messages.size())
        return;

    Message msg = this->messages.takeAt(row);
    int newRow = findInsertIndex(msg.convSeq);

    this->messages.insert(newRow, msg);

    if(newRow == row)
        return;

    rebuildIndex();
    emit messageMove(row, newRow);
}

void MessagesManager::calcShowTimestamp(int row)
{
    if(row < 0 || row >= this->messages.size())
        return;

    Message& msg = this->messages[row];
    if(row == 0)
    {
        msg.showTimestamp = true;
        return;
    }

    const Message& prevMsg = this->messages.at(row - 1);
    msg.showTimestamp = (prevMsg.timeStamp > 0 && msg.timeStamp > 0) && (msg.timeStamp - prevMsg.timeStamp) > 300;
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
