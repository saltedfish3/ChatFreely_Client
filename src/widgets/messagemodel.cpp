#include "messagemodel.h"

MessageModel::MessageModel(MessagesManager* manager, QObject *parent)
    : QAbstractListModel(parent), manager(manager)
{
    connect(manager, &MessagesManager::messageAdd, this, &MessageModel::onMessageAdd);
    connect(manager, &MessagesManager::messagesAdd, this, &MessageModel::onMessagesAdd);
    connect(manager, &MessagesManager::messageUpdate, this, &MessageModel::onMessageUpdate);
    connect(manager, &MessagesManager::messagePrepend, this, &MessageModel::onMessagePrepend);
    connect(manager, &MessagesManager::messageRemove, this, &MessageModel::onMessageRemove);
    connect(manager, &MessagesManager::messageMove, this, &MessageModel::onMessageMove);
    connect(manager, &MessagesManager::resetModel, this, &MessageModel::onResetModel);
    connect(&FriendManage::getFriendManage(), &FriendManage::friendAvatarUpdate, this, &MessageModel::onMessageFriendAvatarUpdate);
    connect(&UserInfo::getUserInfo(), &UserInfo::updateAvatar, this, &MessageModel::onMessageMyselfAvatarUpdate);
    connect(manager, &MessagesManager::uploadProgressUpdate, this, &MessageModel::onUploadProgressUpdate);
    connect(manager, &MessagesManager::mediaExpired, this, &MessageModel::onMediaExpired);
    connect(manager, &MessagesManager::mediaAvailable, this, &MessageModel::onMediaExpired);
    connect(manager, &MessagesManager::updateDownloadStatus, this, &MessageModel::onUpdateDownloadStatus);
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : this->manager->getMessages().size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() >= rowCount())
        return {};
    //获取指定行
    const Message& msg = this->manager->getMessages().at(index.row());
    switch(role)
    {
    case Qt::DisplayRole:
        break;
    case ContentRole:
        return msg.content;
    case TimeStamp:
        return msg.timeStamp;
    case IsMyselfRole:
        return msg.senderUID == UserInfo::getUserInfo().getUID();
    case AvatarRole:
        return msg.senderUID == UserInfo::getUserInfo().getUID() ? UserInfo::getUserInfo().getAvatar(QSize(40, 40)) : FriendManage::getFriendManage().getFriendAvatar(msg.senderUID, QSize(40, 40));
    case MessageStatusRole:
        return msg.status;
    case IsNeedShowTime:
        return msg.showTimestamp;
    case ConvSeqRole:
        return msg.convSeq;
    case MessageIDRole:
        return msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID;
    case ImageRole:
    {
        if(!msg.info.thumbnailUrl.isEmpty())
        {
            QPixmap pix = ImageCacheManager::getManager().fastLoadThumbnail(msg.info.thumbnailUrl);
            if(!pix.isNull())
                return pix;
        }
        return ImageCacheManager::getManager().fastLoadImage(msg.info.url);
    }
    case ContentTypeRole:
        return static_cast<int>(msg.contentType);
    case ImageStateRole:
        return static_cast<int>(ImageCacheManager::getManager().getImageState(msg.info.thumbnailUrl.isEmpty() ? msg.info.url : msg.info.thumbnailUrl));
    case MediaUrl:
        return msg.info.url;
    case MediaWidth:
        return msg.info.width;
    case MediaHeight:
        return msg.info.height;
    case MediaSize:
        return msg.info.size;
    case VideoThumbnail:
        return ImageCacheManager::getManager().fastLoadThumbnail(msg.info.thumbnailUrl);
    case VideoDuration:
        return msg.info.duration;
    case FileName:
        return msg.info.name;
    case MediaUploadProgress:
        if(msg.status != Sending)
            return -1;
        return this->manager->getUploadProgres(msg.info.url);
    case MediaExpired:
        return this->manager->isMediaExpired(msg.info.url);
    case FileDownloadProgress:
        if(msg.status != Success)
            return -1;
        return this->manager->getDownloadProgress(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID);
    case FileDownloadStatus:
        return this->manager->isDownloaded(msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID);
    default:
        return {};
    }
    return {};
}

void MessageModel::onResetModel()
{
    beginResetModel();
    endResetModel();
}

void MessageModel::onUploadProgressUpdate(const QString &tempID, const QString &filePath, int percent)
{
    int row = this->manager->indexOfMsg(tempID);
    if(row < 0)
        return;

    emit dataChanged(index(row), index(row), {MediaUploadProgress});
}

void MessageModel::onMediaExpired(const QString &url)
{
    for(int i = 0; i < rowCount(); i++)
    {
        const Message& msg = this->manager->getMessages().at(i);
        if((msg.contentType == Video || msg.contentType == File) && msg.info.url == url)
            emit dataChanged(index(i), index(i), {MediaExpired});
    }
}

void MessageModel::onDownloadProgressUpdate(const QString &msgID, const QString &url, int percent)
{
    int row = this->manager->indexOfMsg(msgID);
    if(row < 0)
        return;

    emit dataChanged(index(row), index(row), {FileDownloadProgress, FileDownloadStatus});
}

void MessageModel::onUpdateDownloadStatus(const QString &url)
{
    for(int i = 0; i < this->manager->getMessages().size(); i++)
    {
        const Message& msg = this->manager->getMessages().at(i);
        if(msg.contentType == File && msg.info.url == url)
            emit dataChanged(index(i), index(i), {FileDownloadProgress, FileDownloadStatus});
    }
}

void MessageModel::onMessageAdd(int row)
{
    if(row < 0 || row > rowCount())
        return;
    beginInsertRows(QModelIndex(), row, row);
    endInsertRows();
}

void MessageModel::onMessagesAdd(int first, int end)
{
    if(first < 0 || first > end || first >= this->manager->getMessages().size())
        return;

    beginInsertRows(QModelIndex(), first, end);
    endInsertRows();
}

void MessageModel::onMessageUpdate(int row)
{
    if(row < 0 || row >= rowCount())
        return;
    QModelIndex idx = index(row);

    emit dataChanged(idx, idx, {MessageStatusRole, TimeStamp, ConvSeqRole, IsNeedShowTime, MessageIDRole, AvatarRole, ContentRole, ImageRole,
                                ImageStateRole, MediaUrl, MediaWidth, MediaHeight, MediaSize, VideoThumbnail, VideoDuration, FileName});
}

void MessageModel::onMessagesUpdate(int first, int end)
{
    if(first < 0 || end < first || end >= rowCount())
        return;

    emit dataChanged(index(first), index(end), {AvatarRole});
}

void MessageModel::onMessagePrepend(int count)
{
    if(count <= 0)
        return;

    beginInsertRows(QModelIndex(), 0, count - 1);
    endInsertRows();
}

void MessageModel::onMessageRemove(int row)
{
    if(row < 0 || row >= rowCount())
        return;

    beginRemoveRows(QModelIndex(), row, row);
    endRemoveRows();
}

void MessageModel::onMessageMove(int oldRow, int newRow)
{
    if(oldRow < 0 || oldRow >= rowCount())
        return;

    if(newRow < 0 || newRow >= rowCount())
        return;

    beginMoveRows(QModelIndex(), oldRow, oldRow, QModelIndex(), oldRow < newRow ? newRow + 1 : newRow);
    endMoveRows();

    emit dataChanged(index(qMin(oldRow, newRow)), index(qMax(oldRow, newRow)), {MessageStatusRole, MessageIDRole, ConvSeqRole, TimeStamp, IsNeedShowTime});
}

void MessageModel::onMessageMyselfAvatarUpdate(const QPixmap &avatar, const QSize& size)
{
    for(int i = 0; i < this->manager->getMessages().size(); i++)
    {
        const Message& msg = this->manager->getMessages().at(i);
        if(msg.senderUID == UserInfo::getUserInfo().getUID())
            emit dataChanged(index(i), index(i), {AvatarRole});
    }
}

void MessageModel::onMessageFriendAvatarUpdate(const QString &uid, const QPixmap &avatar, const QSize& size)
{
    if(!size.isEmpty() && size != QSize(40, 40))
        return;
    for(int i = 0; i < this->manager->getMessages().size(); i++)
    {
        const Message& msg = this->manager->getMessages().at(i);
        if(msg.senderUID == uid)
            emit dataChanged(index(i), index(i), {AvatarRole});
    }
}
