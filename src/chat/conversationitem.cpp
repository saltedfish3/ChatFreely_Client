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

int ConversationItem::getUnReadCount() const
{
    return this->unRead;
}

void ConversationItem::addNewMessage(const Message &msg)
{
    Message changeableMsg = msg;
    setSenderInfo(changeableMsg);

    const auto& msgs = this->msgManager.getMessages();
    if(msgs.isEmpty())
        changeableMsg.showTimestamp = true;
    else
    {
        const Message& lastMsg = msgs.last();
        if(lastMsg.timeStamp <= 0)
            changeableMsg.showTimestamp = false;
        else
            changeableMsg.showTimestamp = (changeableMsg.timeStamp - lastMsg.timeStamp) > 300;
    }

    this->msgManager.append(changeableMsg);

    emit LastMessageChange(changeableMsg);
    emit PushNewMessage(changeableMsg);
}

void ConversationItem::updateMessageInfo(bool isSuccess, QString tempMsgID, QString receiverUID, QString messageID, int64_t timeStamp, int64_t convSeq)
{
    this->msgManager.updateStatus(tempMsgID, messageID, isSuccess ? Success : Failed, timeStamp, convSeq);
    emit messageStatusChange(tempMsgID, isSuccess ? Success : Failed);
}

void ConversationItem::updateSenderAvatar(const QString &senderUID, const QPixmap &avatar)
{
    bool isChanged = false;
    QList<Message>& msgs = this->msgManager.getMessages();
    for(auto& msg : msgs)
    {
        if(msg.senderUID == senderUID)
        {
            msg.avatar = avatar;
            isChanged = true;
        }
    }
    if(isChanged)
        emit senderAvatarUpdate(senderUID);
}

bool ConversationItem::isActive() const
{
    return this->active;
}

void ConversationItem::clearUnRead()
{
    if(this->unRead != 0)
    {
        this->unRead = 0;
        emit UnReadCountChange(0);
    }
}

void ConversationItem::addUnReadCount()
{
    this->unRead++;
    emit UnReadCountChange(this->unRead);
}

void ConversationItem::setActive(bool isActive)
{
    this->active = isActive;
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
