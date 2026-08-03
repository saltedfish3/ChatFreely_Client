#include "conversationitem.h"

ConversationItem::ConversationItem(const QString& conversationID, QObject *parent)
    : conversationID(conversationID), QObject{parent}
{
}

QString ConversationItem::getConversationID()
{
    return this->conversationID;
}

const MessagesManager &ConversationItem::getMessagesManager() const
{
    return this->msgManager;
}

Message ConversationItem::getLastMessage() const
{
    return msgManager.getLastMessage();
}

void ConversationItem::addNewMessage(const Message &msg)
{
    Message changeableMsg = msg;
    setSenderInfo(changeableMsg);
    this->unRead++;
    this->msgManager.append(changeableMsg);

    emit UnReadCountChange(this->unRead);
    emit LastMessageChange(changeableMsg);
    emit PushNewMessage(changeableMsg);
}

void ConversationItem::updateMessageInfo(bool isSuccess, QString tempMsgID, QString receiverUID, QString messageID, int64_t timeStamp, int64_t convSeq)
{
    this->msgManager.updateStatus(tempMsgID, messageID, isSuccess ? Success : Failed, timeStamp, convSeq);
    emit messageStatusChange(tempMsgID, isSuccess ? Success : Failed);
}

void ConversationItem::setSenderInfo(Message &msg)
{
    if(UserInfo::getUserInfo().getUID() == msg.senderUID)
    {
        msg.username = UserInfo::getUserInfo().getUsername();
        msg.avatar = UserInfo::getUserInfo().getAvatar();
    }
    else
    {
        FriendManage::FriendInfo info = FriendManage::getFriendManage().getFriendInfo(msg.senderUID);
        if(info.uid.isEmpty())
        {
            msg.username = "群聊成员";
            msg.avatar = QPixmap(":/default/images/defaultAvatar.png");
        }
        else
        {
            msg.username = info.username;
            msg.avatar = info.avatar;
        }
    }
}
