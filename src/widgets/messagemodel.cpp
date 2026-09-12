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
        return ImageCacheManager::getManager().fastLoadImage(msg.content);
    case ContentTypeRole:
        return static_cast<int>(msg.contentType);
    case ImageStateRole:
        return static_cast<int>(ImageCacheManager::getManager().getImageState(msg.content));
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

    emit dataChanged(idx, idx, {MessageStatusRole, TimeStamp, ConvSeqRole, IsNeedShowTime, MessageIDRole, AvatarRole, ContentRole, ImageRole});
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
    if(size != QSize(40, 40))
        return;
    for(int i = 0; i < this->manager->getMessages().size(); i++)
    {
        const Message& msg = this->manager->getMessages().at(i);
        if(msg.senderUID == uid)
            emit dataChanged(index(i), index(i), {AvatarRole});
    }
}
