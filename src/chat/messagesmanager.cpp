#include "messagesmanager.h"

MessagesManager::MessagesManager(const QString& conversationID, QObject* parent)
    : QObject(parent), conversationID(conversationID)
{
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::uploadProgressChanged, this, [this](const QString& filePath, qint64 sendsize, qint64 totalsize, int percent){
        if(!this->hash_registerUploadID.contains(filePath))
            return;

        this->hash_uploadProgress[filePath] = percent;

        for(const QString& tempID : this->hash_registerUploadID.value(filePath))
            emit uploadProgressUpdate(tempID, filePath, percent);
    });
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

    if(msg.contentType == Image)
    {
        emit startLoadingImage();
        if(!msg.info.thumbnailUrl.isEmpty())
        {
            ImageCacheManager::getManager().loadThumbnail(msg.info.thumbnailUrl, [this, msg](const QPixmap&){
                emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                emit finishLoadedImage();
            });
        }
        else
        {
            ImageCacheManager::getManager().loadImage(msg.info.url, [this, msg](const QPixmap&){
                emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                emit finishLoadedImage();
            });
        }
    }
    else if(msg.contentType == Video && !msg.info.thumbnailUrl.isEmpty())
    {
        emit startLoadingImage();
        ImageCacheManager::getManager().loadThumbnail(msg.info.thumbnailUrl, [this, msg](const QPixmap&){
            emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
            emit finishLoadedImage();
        });
    }

    if(isStoreDB)
        calcShowTimestamp(index);

    emit messageAdd(index);
    if(isStoreDB)
        DatabaseManager::getDatabaseManager().addInsertMessageTask(this->conversationID, this->messages.at(index));

    checkMediaExpiredStatus(msg);
}

void MessagesManager::addMessages(const QList<Message> &msgs, bool isStoreDB)
{
    if(msgs.isEmpty())
        return;

    QSet<QString> existsID;
    QList<Message> validMsgs;
    for(const auto& msg : msgs)
    {
        if(msg.status == Success)
        {
            //消息去重
            bool isExist = false;
            if(!msg.serverMsgID.isEmpty())
                isExist = indexOfMsg(msg.serverMsgID) == -1 ? false : true || existsID.contains(msg.serverMsgID);
            if(!isExist && !msg.tempMsgID.isEmpty())
                isExist = indexOfMsg(msg.tempMsgID) == -1 ? false : true || existsID.contains(msg.tempMsgID);

            if(isExist)
                continue;
        }
        existsID.insert(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID);
        validMsgs.append(msg);
    }

    if(validMsgs.isEmpty())
        return;

    for(const auto& msg : validMsgs)
    {
        int index = findInsertIndex(msg.convSeq);
        this->messages.insert(index, msg);

        if(msg.status == Success && msg.convSeq > this->lastConvSeq)
            this->lastConvSeq = msg.convSeq;
    }
    rebuildIndex();

    for(const auto& msg : validMsgs)
        checkMediaExpiredStatus(msg);

    if(isStoreDB)
    {
        for(const auto& msg : validMsgs)
        {
            int index = indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID);
            if(index != -1)
            {
                calcShowTimestamp(index);
                DatabaseManager::getDatabaseManager().addInsertMessageTask(this->conversationID, this->messages.at(index));
            }
        }
    }

    int first = INT_MAX;
    int end = -1;
    for(const auto& msg : validMsgs)
    {
        int index = indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID);
        first = qMin(index, first);
        end = qMax(index, end);
    }
    if(first <= end)
        emit messagesAdd(first, end);
    else
        emit resetModel();

    for(const auto& msg : validMsgs)
    {
        if(msg.contentType == Image)
        {
            emit startLoadingImage();
            if(!msg.info.thumbnailUrl.isEmpty())
            {
                ImageCacheManager::getManager().loadThumbnail(msg.info.thumbnailUrl, [this, msg](const QPixmap&){
                    emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                    emit finishLoadedImage();
                });
            }
            else
            {
                ImageCacheManager::getManager().loadImage(msg.info.url, [this, msg](const QPixmap&){
                    emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                    emit finishLoadedImage();
                });
            }
        }
        else if(msg.contentType == Video && !msg.info.thumbnailUrl.isEmpty())
        {
            emit startLoadingImage();
            ImageCacheManager::getManager().loadThumbnail(msg.info.thumbnailUrl, [this, msg](const QPixmap&){
                emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                emit finishLoadedImage();
            });
        }
    }
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

bool MessagesManager::updateMessageContent(const QString &tempMsgID, const QString &content)
{
    auto it = this->index_message.find(tempMsgID);
    if(it == this->index_message.end())
        return false;

    int row = it.value();
    Message& msg = this->messages[row];
    msg.content = content;
    msg.parseMedia();

    if(msg.contentType == Image)
    {
        emit startLoadingImage();
        if(!msg.info.thumbnailUrl.isEmpty())
        {
            ImageCacheManager::getManager().loadThumbnail(msg.info.thumbnailUrl, [this, msg](const QPixmap&){
                emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                emit finishLoadedImage();
            });
        }
        else
        {
            ImageCacheManager::getManager().loadImage(msg.info.url, [this, msg](const QPixmap&){
                emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                emit finishLoadedImage();
            });
        }
    }

    if(msg.contentType == Video && msg.info.url.startsWith("http"))
    {
        if(this->set_expiredMediaUrl.remove(msg.info.url))
            emit mediaAvailable(msg.info.url);
    }

    if(msg.contentType == Video)
    {
        if(content.startsWith("{") && !msg.info.thumbnailUrl.isEmpty())
        {
            emit startLoadingImage();
            ImageCacheManager::getManager().loadThumbnail(msg.info.thumbnailUrl, [this, msg](const QPixmap&){
                emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                emit finishLoadedImage();
            });
        }
    }

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

bool MessagesManager::cancelUpload(const QString &tempMsgID)
{
    QString filePath = this->hash_tempIDToFilePath.value(tempMsgID);
    if(filePath.isEmpty())
        return false;

    updateMessageStatus(tempMsgID, QString(), Cancelled, -1, -1);
    unregisterUpload(tempMsgID);
    return !this->hash_registerUploadID.contains(filePath);
}

void MessagesManager::reLoadImage(int index)
{
    if(this->messages.isEmpty() || index < 0 || index >= this->messages.size())
        return;

    Message& msg = this->messages[index];
    if(msg.contentType == Image)
    {
        emit startLoadingImage();
        if(!msg.info.thumbnailUrl.isEmpty())
        {
            ImageCacheManager::getManager().loadThumbnail(msg.info.thumbnailUrl, [this, msg](const QPixmap&){
                emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                emit finishLoadedImage();
            });
        }
        else
        {
            ImageCacheManager::getManager().loadImage(msg.info.url, [this, msg](const QPixmap&){
                emit messageUpdate(indexOfMsg(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID));
                emit finishLoadedImage();
            });
        }
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

void MessagesManager::registerUpload(const QString &tempID, const QString &filePath)
{
    auto& list = this->hash_registerUploadID[filePath];
    if(!list.contains(tempID))
        list.append(tempID);

    this->hash_tempIDToFilePath[tempID] = filePath;

    if(!this->hash_uploadProgress.contains(filePath))
        this->hash_uploadProgress[filePath] = 0;
}

void MessagesManager::unregisterUpload(const QString &tempID)
{
    QString filePath = this->hash_tempIDToFilePath.take(tempID);
    if(filePath.isEmpty())
        return;

    auto it = this->hash_registerUploadID.find(filePath);
    if(it == this->hash_registerUploadID.end())
        return;

    it.value().removeAll(tempID);
    if(it.value().isEmpty())
    {
        this->hash_registerUploadID.erase(it);
        this->hash_uploadProgress.remove(filePath);
    }
}

int MessagesManager::getUploadProgres(const QString &filePath) const
{
    if(filePath.isEmpty())
        return -1;

    auto it = this->hash_uploadProgress.find(filePath);
    if(it == this->hash_uploadProgress.end())
        return -1;

    return it.value();
}

bool MessagesManager::isMediaExpired(const QString &url) const
{
    return this->set_expiredMediaUrl.contains(url);
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

void MessagesManager::checkMediaExpiredStatus(const Message &msg)
{
    if(msg.contentType != Video && msg.contentType != File)
        return;

    const QString& url = msg.info.url;
    if(url.isEmpty() || !url.startsWith("http"))
        return;

    if(this->set_expiredMediaUrl.contains(url) || this->set_checkingMediaUrl.contains(url))
        return;

    QString localPath = VideoUtils::getLocalUrlPath(url);
    if(!localPath.isEmpty() && QFile::exists(localPath))
        return;

    this->set_checkingMediaUrl.insert(url);

    HttpShortConnection::getHttpClient().checkUrlExists(url, [this, url](bool isExists){
        this->set_checkingMediaUrl.remove(url);
        if(!isExists)
        {
            this->set_expiredMediaUrl.insert(url);
            emit mediaExpired(url);
        }
    });
}
