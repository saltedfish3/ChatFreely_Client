#include "messagemodel.h"

MessageModel::MessageModel(ConversationItem* item, QObject *parent)
    : QAbstractListModel(parent), item(item)
{
    connect(item, &ConversationItem::PushNewMessage, this, &MessageModel::onNewMessage);
    connect(item, &ConversationItem::messageStatusChange, this, &MessageModel::onMessageStatusChanged);
    connect(item, &ConversationItem::senderAvatarUpdate, this, &MessageModel::onSenderAvatarUpdate);
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : this->item->getMessagesManager().getMessages().count();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid() || index.row() >= rowCount())
        return {};
    //获取指定行
    const Message& msg = this->item->getMessagesManager().getMessages().at(index.row());
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
        return msg.avatar;
    case MessageStatusRole:
        return msg.status;
    case IsNeedShowTime:
        return msg.showTimestamp;
    case ConvSeqRole:
        return msg.convSeq;
    case MessageIDRole:
        return msg.serverMsgID.isEmpty() ? msg.tempMsgID : msg.serverMsgID;
    default:
        return {};
    }
    return {};
}

void MessageModel::onNewMessage(const Message &msg)
{
    int pos = rowCount();
    beginInsertRows(QModelIndex(), pos, pos);
    endInsertRows();
}

void MessageModel::onMessageStatusChanged(const QString &tempMsgID, Status status)
{
    int row = this->item->getMessagesManager().indexOfMsg(tempMsgID);
    if(row >= 0)
    {
        QModelIndex idx = index(row);
        emit dataChanged(idx, idx, {MessageStatusRole, IsNeedShowTime, TimeStamp});
    }
}

void MessageModel::onSenderAvatarUpdate(const QString &senderUID)
{
    auto& msgs = this->item->getMessagesManager().getMessages();
    for(int i = 0; i < msgs.size(); i++)
    {
        if(msgs.at(i).senderUID == senderUID)
            emit dataChanged(index(i), index(i), {AvatarRole});
    }
}

void MessageModel::loadHistoryMessages(const QList<Message> &msgs)
{
    if(msgs.isEmpty())
        return;
    beginInsertRows(QModelIndex(), 0, msgs.size() - 1);
    endInsertRows();
}
