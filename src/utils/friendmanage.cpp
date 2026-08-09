#include "friendmanage.h"
#include "../network/tcplongconnection.h"

FriendManage &FriendManage::getFriendManage()
{
    static FriendManage fm;
    return fm;
}

int64_t FriendManage::getFriendCount() const
{
    QReadLocker locker(&(this->lock));
    return this->map_friend.count();
}

FriendManage::FriendInfo FriendManage::getFriendInfo(const QString &uid) const
{
    QReadLocker locker(&(this->lock));
    auto it = this->map_friend.find(uid);
    if(it == this->map_friend.end())
        return {};
    return it.value();
}

QList<FriendManage::FriendInfo> FriendManage::getAllFriend() const
{
    QReadLocker locker(&(this->lock));
    return this->map_friend.values();
}

void FriendManage::cleanAll()
{
    QWriteLocker locker(&(this->lock));
    this->map_friend.clear();
    this->isFirstLoad = false;
}

FriendManage::FriendManage(QObject *parent)
    : QObject{parent}
{
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newFriend, this, [this](QString uid, QString sid, QString username, QString avatar_url, QString email, bool isOnline){
        //防止编译器瞎jb警告
        QPointer<FriendManage> pointer(this);
        if(!pointer)
            return;
        {
            QWriteLocker locker(&(this->lock));

            auto& info = this->map_friend[uid];

            info.uid = uid;
            info.sid = sid;
            info.username = username;
            info.email = email;
            info.avatar = QPixmap(":/default/images/defaultAvatar.png");
            info.avatarUrl = avatar_url;
            info.isOnline = isOnline;
        }
        emit allFriendList();

        HttpShortConnection::getHttpClient().getAvatar(avatar_url, 3, [this, uid](const QPixmap& avatar){
            QPointer<FriendManage> pointer(this);
            if(!pointer)
                return;
            {
                QWriteLocker locker(&(this->lock));
                auto it = this->map_friend.find(uid);
                if(it == this->map_friend.end())
                    return;

                it.value().avatar = avatar;
            }
            emit friendAvatarUpdate(uid, avatar);
        }, false);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::FriendStatus, this, [this](QString UID, bool isOnline){
        QPointer<FriendManage> pointer(this);
        if(!pointer)
            return;
        {
            QWriteLocker locker(&(this->lock));
            auto it = this->map_friend.find(UID);
            if(it == this->map_friend.end())
                return;

            it.value().isOnline = isOnline;
        }
        emit friendStatusUpdate(UID, isOnline);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::FriendUsername, this, [this](QString uid, QString username){
        QPointer<FriendManage> pointer(this);
        if(!pointer)
            return;
        {
            QWriteLocker locker(&(this->lock));
            auto it = this->map_friend.find(uid);
            if(it == this->map_friend.end())
                return;

            it.value().username = username;
        }
        emit friendUsernameUpdate(uid, username);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::FriendAvatar, this, [this](QString uid, QString avatarUrl){
        QPointer<FriendManage> pointer(this);
        if(!pointer)
            return;
        {
            QWriteLocker locker(&(this->lock));
            auto it = this->map_friend.find(uid);
            if(it == this->map_friend.end() || it.value().avatarUrl == avatarUrl)
                return;
            it.value().avatarUrl = avatarUrl;
        }
        HttpShortConnection::getHttpClient().getAvatar(avatarUrl, 3, [this, uid](const QPixmap& avatar){
            QPointer<FriendManage> pointer(this);
            if(!pointer)
                return;
            {
                QWriteLocker locker(&(this->lock));
                auto it = this->map_friend.find(uid);
                if(it == this->map_friend.end())
                    return;

                    it.value().avatar = avatar;
            }
            emit friendAvatarUpdate(uid, avatar);
        }, false);
    });

    //全量更新
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::allFriendList, this, [this](const QList<QVariantMap>& list){
        QMap<QString, QString> avatarTasks;
        QPointer<FriendManage> pointer(this);
        if(!pointer)
            return;
        {
            QWriteLocker locker(&(this->lock));
            this->map_friend.clear();

            for(auto& var : std::as_const(list))
            {
                FriendInfo info;
                info.uid = var.value("UID").toString();
                info.sid = var.value("SID").toString();
                info.username = var.value("Username").toString();
                info.email = var.value("Email").toString();
                info.avatar = QPixmap(":/default/images/defaultAvatar.png");
                info.avatarUrl = var.value("AvatarUrl").toString();
                info.isOnline = var.value("IsOnline").toBool();
                this->map_friend[info.uid] = info;

                avatarTasks[info.uid] = info.avatarUrl;
            }
        }
        emit allFriendList();
        if(!this->isFirstLoad)
        {
            this->isFirstLoad = true;
            emit loadFirstAllFriendList();
        }

        for(auto it = avatarTasks.begin(); it != avatarTasks.end(); it++)
        {
            QString uid = it.key();
            HttpShortConnection::getHttpClient().getAvatar(it.value(), 3, [this, uid](const QPixmap& avatar){
                QPointer<FriendManage> pointer(this);
                if(!pointer)
                    return;
                {
                    QWriteLocker locker(&lock);
                    auto it = this->map_friend.find(uid);
                    if(it == this->map_friend.end())
                        return;
                    it.value().avatar = avatar;
                }
                emit friendAvatarUpdate(uid, avatar);
            }, false);
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &FriendManage::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &FriendManage::cleanAll);
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, &FriendManage::cleanAll);
}
