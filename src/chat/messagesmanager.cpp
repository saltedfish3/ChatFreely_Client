#include "messagesmanager.h"
#include "conversationitem.h"

MessagesManager::MessagesManager(const QString& conversationID, int conversationType, QObject* parent)
    : QObject(parent), conversationID(conversationID), conversationType(conversationType)
{
    this->timer_highLight = new QTimer(this);
    this->timer_highLight->setSingleShot(true);

    connect(this->timer_highLight, &QTimer::timeout, this, [this](){
        setHighLightMsgID({});
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::uploadProgressChanged, this, [this](const QString& filePath, qint64 sendsize, qint64 totalsize, int percent){
        if(!this->hash_registerUploadID.contains(filePath))
            return;

        this->hash_uploadProgress[filePath] = percent;

        for(const QString& tempID : this->hash_registerUploadID.value(filePath))
            emit uploadProgressUpdate(tempID, filePath, percent);
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::downloadStarted, [this](const QString& url){
        if(!this->hash_registerDownloadID.contains(url))
            return;

        this->hash_downloadProgress[url] = 0;

        for(const QString& msgID : this->hash_registerDownloadID.value(url))
            emit downloadProgressUpdate(msgID, url, 0);
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::downloadProgressChanged, [this](const QString& url, qint64 received, qint64 total){
        if(!this->hash_registerDownloadID.contains(url))
            return;

        int percent = total > 0 ? static_cast<int>(received * 100 / total) : 0;
        this->hash_downloadProgress[url] = percent;

        for(const QString& msgID : this->hash_registerDownloadID.value(url))
            emit downloadProgressUpdate(msgID, url, percent);
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::downloadFinished, [this](const QString& url, bool isSuccess, const QString& info){
        if(!this->hash_registerDownloadID.contains(url))
            return;

        const QList<QString> msgIDs = this->hash_registerDownloadID.take(url);
        this->hash_downloadProgress.remove(url);

        if(isSuccess)
        {
            this->set_downloadFileExists.insert(url);
            emit updateDownloadStatus(url);
        }

        for(const QString& msgID : msgIDs)
        {
            this->hash_tempIDToDownloadUrl.remove(msgID);
            emit downloadProgressUpdate(msgID, url, isSuccess ? 100 : -1);
        }
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
    {
        this->lastConvSeq = msg.convSeq;
    }

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

    if((msg.contentType == Video || msg.contentType == File) && msg.info.url.startsWith("http"))
    {
        if(this->set_expiredMediaUrl.remove(msg.info.url))
            emit mediaAvailable(msg.info.url);
    }

    if(isStoreDB)
        calcShowTimestamp(index);

    emit messageAdd(index);
    if(isStoreDB)
        DatabaseManager::getDatabaseManager().addInsertMessageTask(this->conversationID, this->messages.at(index), getConversationType());

    checkMediaExpiredStatus(msg);
    updateRanges(msg.convSeq, msg.convSeq);
}

void MessagesManager::addMessages(const QList<Message> &msgs, qint64 convSeqFirst, qint64 convSeqLast, bool isStoreDB)
{
    if(msgs.isEmpty())
        return;

    QList<Message> validMsgs;
    for(const auto& msg : msgs)
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
                continue;
        }
        validMsgs.append(msg);
    }

    if(validMsgs.isEmpty())
        return;

    for(const auto& msg : validMsgs)
    {
        int index = findInsertIndex(msg.convSeq);
        this->messages.insert(index, msg);
        emit messageAdd(index);

        if(msg.status == Success && msg.convSeq > this->lastConvSeq)
            this->lastConvSeq = msg.convSeq;
    }
    rebuildIndex();

    emit messagesInserted();

    updateRanges(convSeqFirst, convSeqLast);

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

        if((msg.contentType == Video || msg.contentType == File) && msg.info.url.startsWith("http"))
        {
            if(this->set_expiredMediaUrl.remove(msg.info.url))
                emit mediaAvailable(msg.info.url);
        }

        if(isStoreDB)
        {
            const QString msgID = msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID;
            int index = indexOfMsg(msgID);

            if(index != -1)
            {
                calcShowTimestamp(index);
                DatabaseManager::getDatabaseManager().addInsertMessageTask(this->conversationID, this->messages.at(index), getConversationType());
            }
        }
        checkMediaExpiredStatus(msg);
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

    if((msg.contentType == Video || msg.contentType == File) && msg.info.url.startsWith("http"))
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
    this->loadGen_older++;
    this->loadGen_newer++;
    this->loadGen_around++;
    this->loadGen_jump++;

    this->messages.clear();
    this->index_message.clear();
    this->ranges.clear();
    // this->lastConvSeq = 0;
    this->highLightMsgID.clear();
    this->timer_highLight->stop();

    this->hash_registerUploadID.clear();
    this->hash_tempIDToFilePath.clear();
    this->hash_uploadProgress.clear();
    this->hash_registerDownloadID.clear();
    this->hash_tempIDToDownloadUrl.clear();
    this->hash_downloadProgress.clear();

    emit resetModel();
}

void MessagesManager::loadMessages(LoadDir dir, qint64 cursorConvSeq, int limit)
{
    auto finish = [this, dir](){
        emit loadMessagesFinished(dir);
    };

    if(dir == LoadDir::Around)
    {
        if(findRangeIndexByconvSeq(cursorConvSeq) >= 0)
        {
            finish();
            return;
        }

        quint64 gen = ++this->loadGen_around;
        DatabaseManager::getDatabaseManager().loadConversationMessages(this->conversationID, cursorConvSeq, DatabaseManager::MessageQueryDir::Around, limit, [this, gen, finish](const QList<Message>& msgs){
            if(gen != this->loadGen_around)
            {
                finish();
                return;
            }

            if(!msgs.isEmpty())
            {
                qint64 convSeqFirst = msgs.first().convSeq;
                qint64 convSeqLast = msgs.last().convSeq;
                addMessages(msgs, convSeqFirst, convSeqLast, false);
            }
            finish();
        });

        return;
    }

    bool isOlder = dir == LoadDir::Older;

    if(findRangeIndexByconvSeq(isOlder ? cursorConvSeq - 1 : cursorConvSeq + 1) >= 0)
    {
        finish();
        return;
    }

    quint64 gen = isOlder ? ++this->loadGen_older : ++this->loadGen_newer;
    DatabaseManager::getDatabaseManager().loadConversationMessages(this->conversationID, cursorConvSeq, isOlder ? DatabaseManager::MessageQueryDir::Older : DatabaseManager::MessageQueryDir::Newer, limit,
        [this, gen, cursorConvSeq, isOlder, limit, finish](const QList<Message>& msgs){
        quint64 currentGen = isOlder ? this->loadGen_older : this->loadGen_newer;
        if(gen != currentGen)
        {
            finish();
            return;
        }

        if(msgs.isEmpty())
        {
            int rangeIndex = findRangeIndexByconvSeq(cursorConvSeq);
            if(isOlder)
            {
                if(rangeIndex <= 0)
                    updateRanges(0, cursorConvSeq - 1);
            }
            else
            {
                if(rangeIndex < 0 || rangeIndex >= this->ranges.size() - 1)
                    updateRanges(cursorConvSeq + 1, INT64_MAX);
            }
            finish();
            return;
        }

        qint64 convSeqFirst;
        qint64 convSeqLast;
        if(isOlder)
        {
            convSeqFirst = msgs.size() < limit ? 0 : msgs.first().convSeq;
            convSeqLast = cursorConvSeq - 1;
        }
        else
        {
            convSeqFirst = cursorConvSeq + 1;
            convSeqLast = msgs.size() < limit ? INT64_MAX : msgs.last().convSeq;
        }

        addMessages(msgs, convSeqFirst, convSeqLast, false);
        finish();
    });
}

void MessagesManager::jumpToMessage(const QString &msgID, qint64 convSeq)
{
    int row = indexOfMsg(msgID);
    if(row >= 0)
    {
        emit jumpToFinished(row, msgID);
        return;
    }

    quint64 gen = ++this->loadGen_jump;
    DatabaseManager::getDatabaseManager().loadConversationMessages(this->conversationID, convSeq, DatabaseManager::MessageQueryDir::Around, 20, [this, gen, msgID, convSeq](const QList<Message>& msgs){
        if(gen != this->loadGen_jump)
            return;

        if(msgs.isEmpty())
        {
            emit jumpToFailed(msgID);
            return;
        }

        qint64 convSeqFirst = msgs.first().convSeq;
        qint64 convSeqLast = msgs.last().convSeq;
        addMessages(msgs, convSeqFirst, convSeqLast, false);

        int targetRow = indexOfMsg(msgID);
        if(targetRow < 0)
        {
            targetRow = indexOfConvSeqOrBefore(convSeq);
            if(targetRow < 0)
                targetRow = indexOfConvSeqOrAfter(convSeq);
            if(targetRow < 0)
                targetRow = 0;
        }

        emit jumpToFinished(targetRow, msgID);
    });
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

bool MessagesManager::cancelDownload(const QString &msgID)
{
    QString url = this->hash_tempIDToDownloadUrl.value(msgID);
    if(url.isEmpty())
        return false;

    unregisterDownload(msgID);
    return !this->hash_registerDownloadID.contains(url);
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

void MessagesManager::registerDownload(const QString &msgID, const QString &url)
{
    auto& list = this->hash_registerDownloadID[url];
    bool isNew = false;
    if(!list.contains(msgID))
    {
        list.append(msgID);
        isNew = true;
    }

    this->hash_tempIDToDownloadUrl[msgID] = url;

    if(!this->hash_downloadProgress.contains(url))
        this->hash_downloadProgress[url] = 0;

    if(isNew)
        emit downloadProgressUpdate(msgID, url, this->hash_downloadProgress.value(url));
}

void MessagesManager::unregisterDownload(const QString &msgID)
{
    QString url = this->hash_tempIDToDownloadUrl.take(msgID);
    if(url.isEmpty())
        return;

    auto it = this->hash_registerDownloadID.find(url);
    if(it == this->hash_registerDownloadID.end())
        return;

    it.value().removeAll(msgID);
    if(it.value().isEmpty())
    {
        this->hash_registerDownloadID.erase(it);
        this->hash_downloadProgress.remove(url);
    }
}

int MessagesManager::getDownloadProgress(const QString &msgID) const
{
    if(msgID.isEmpty())
        return -1;

    auto it = this->hash_tempIDToDownloadUrl.find(msgID);
    if(it == this->hash_tempIDToDownloadUrl.end())
        return -1;

    auto it1 = this->hash_downloadProgress.find(it.value());
    if(it1 == this->hash_downloadProgress.end())
        return -1;

    return it1.value();
}

bool MessagesManager::isDownloaded(const QString &msgID)
{
    int index = indexOfMsg(msgID);
    if(index < 0)
        return false;

    const Message& msg = this->messages.at(index);
    if(msg.info.name.isEmpty() || !msg.info.url.startsWith("http"))
        return false;

    if(this->set_downloadFileExists.contains(msg.info.url))
        return true;

    bool isExists = GlobalVariable::isDownloadFileExists(msg.info.name);
    if(isExists)
        this->set_downloadFileExists.insert(msg.info.url);
    return isExists;
}

bool MessagesManager::isDownloading(const QString &url) const
{
    auto it = this->hash_downloadProgress.find(url);
    if(it == this->hash_downloadProgress.end())
        return false;
    return true;
}

void MessagesManager::setDownloadExists(const QString &url, bool exists)
{
    bool isChanged = false;
    if(exists)
    {
        if(!this->set_downloadFileExists.contains(url))
        {
            this->set_downloadFileExists.insert(url);
            isChanged = true;
        }
    }
    else
    {
        if(this->set_downloadFileExists.remove(url))
            isChanged = true;
    }

    if(isChanged)
        emit updateDownloadStatus(url);
}

bool MessagesManager::isMediaExpired(const QString &url) const
{
    return this->set_expiredMediaUrl.contains(url);
}

const QList<MessagesManager::Range> &MessagesManager::getRanges() const
{
    return this->ranges;
}

int MessagesManager::indexOfConvSeqOrAfter(qint64 convSeq) const
{
    int low = 0;
    int high = this->messages.size() - 1;
    int result = -1;
    while(low <= high)
    {
        int mid = (low + high) / 2;
        if(this->messages.at(mid).convSeq >= convSeq)
        {
            result = mid;
            high = mid - 1;
        }
        else
            low = mid + 1;
    }

    return result;
}

int MessagesManager::indexOfConvSeqOrBefore(qint64 convSeq) const
{
    int low = 0;
    int high = this->messages.size() - 1;
    int result = -1;
    while(low <= high)
    {
        int mid = (low + high) / 2;
        if(this->messages.at(mid).convSeq <= convSeq)
        {
            result = mid;
            low = mid + 1;
        }
        else
            high = mid - 1;
    }

    return result;
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

void MessagesManager::updateRanges(qint64 convSeqFirst, qint64 convSeqLast)
{
    if(convSeqFirst > convSeqLast)
        return;

    int i = 0;
    while(i < this->ranges.size())
    {
        const Range& range = this->ranges[i];
        if(range.last + 1 >= convSeqFirst && range.first <= convSeqLast + 1)
        {
            convSeqFirst = qMin(convSeqFirst, range.first);
            convSeqLast = qMax(convSeqLast, range.last);
            ranges.removeAt(i);
        }
        else if(range.first > convSeqLast + 1)
            break;
        else
            i++;
    }

    int pos = 0;
    while(pos < this->ranges.size() && this->ranges[pos].first < convSeqFirst)
        pos++;
    this->ranges.insert(pos, {convSeqFirst, convSeqLast});
}

int MessagesManager::findRangeIndexByconvSeq(qint64 convSeq) const
{
    int low = 0;
    int high = this->ranges.size() - 1;
    int result = -1;

    while(low <= high)
    {
        int mid = (low + high) / 2;
        if(this->ranges[mid].first <= convSeq)
        {
            result = mid;
            low = mid + 1;
        }
        else
            high = mid - 1;
    }

    if(result < 0 || convSeq > this->ranges[result].last)
        return -1;
    return result;
}

void MessagesManager::setHighLightMsgID(const QString &msgID)
{
    if(msgID == this->highLightMsgID)
        return;

    QString oldMsgID = this->highLightMsgID;
    this->highLightMsgID = msgID;

    emit highLightChanged(oldMsgID, msgID);

    if(msgID.isEmpty())
        this->timer_highLight->stop();
    else
    {
        if(this->timer_highLight->isActive())
            this->timer_highLight->stop();

        this->timer_highLight->start(1000);
    }
}

QString MessagesManager::getHighLightMsgID() const
{
    return this->highLightMsgID;
}

int MessagesManager::getConversationType() const
{
    if(!GroupManager::getGroupManage().getGroupInfo(this->conversationID).conversationID.isEmpty())
        return static_cast<int>(ConversationItem::ConversationType::Group);
    if(!FriendManage::getFriendManage().getFriendInfo(this->conversationID).uid.isEmpty())
        return static_cast<int>(ConversationItem::ConversationType::Personal);

    return this->conversationType;
}
