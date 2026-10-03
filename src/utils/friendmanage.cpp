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

QPixmap FriendManage::getFriendAvatar(const QString &uid, const QSize &wantedSize)
{
    QString url = getFriendInfo(uid).avatarUrl;
    QString loadUrl = url.isEmpty() ? ":/default/images/defaultAvatar.png" : url;
    QString key = uid + QString("_%1x%2_%3").arg(wantedSize.width()).arg(wantedSize.height()).arg(loadUrl);

    //查询头像是否已存在
    QPixmap pix = ImageCacheManager::getManager().fastLoadImage(loadUrl, 0, 0, wantedSize);
    if(!pix.isNull())
        return pix;

    pix = ImageCacheManager::getManager().fastLoadImage(":/default/images/defaultAvatar.png", 0, 0, wantedSize);

    //任务去重
    {
        QWriteLocker locker(&(this->lock));
        if(this->set_paddingAvatarSize.contains(key))
        {
            if(!pix.isNull())
                return pix;
            return QPixmap(":/default/images/defaultAvatar.png");
        }
        else
            this->set_paddingAvatarSize.insert(key);
    }

    if(!url.isEmpty())
    {
        QPixmap original = ImageCacheManager::getManager().fastLoadImage(url, 1.0);
        if(!original.isNull())
        {
            ImageCacheManager::getManager().loadImage(url, [this, uid, key, url, wantedSize](const QPixmap& pix){
                QString currentUrl = getFriendInfo(uid).avatarUrl;
                {
                    QWriteLocker locker(&(this->lock));
                    this->set_paddingAvatarSize.remove(key);
                }
                if(currentUrl == url)
                    emit friendAvatarUpdate(uid, pix, wantedSize);
            }, false, 0, 0, wantedSize);

            return original;
        }
    }

    //url为空 或 默认头像处理
    QPointer<FriendManage> pointer(this);
    ImageCacheManager::getManager().loadImage(loadUrl, [this, pointer, uid, key, loadUrl, wantedSize](const QPixmap& pix){
        if(!pointer)
            return;

        {
            QWriteLocker locker(&(this->lock));
            pointer->set_paddingAvatarSize.remove(key);
        }

        QString current = pointer->getFriendInfo(uid).avatarUrl;
        bool needUpdate = current.isEmpty() ? loadUrl == ":/default/images/defaultAvatar.png" : current == loadUrl;

        if(needUpdate)
        {
            if(pix.isNull())
            {
                ImageCacheManager::getManager().loadImage(":/default/images/defaultAvatar.png", [this, uid, wantedSize](const QPixmap& pix){
                    if(!pix.isNull())
                    {
                        emit friendAvatarUpdate(uid, pix, wantedSize);
                    }
                }, false, 0, 0, wantedSize);
                return;
            }
            emit friendAvatarUpdate(uid, pix, wantedSize);
        }
    }, false, 0, 0, wantedSize);

    if(!pix.isNull())
        return pix;
    return QPixmap(":/default/images/defaultAvatar.png");
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
    this->set_paddingAvatarSize.clear();
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
            info.avatarUrl = avatar_url;
            info.isOnline = isOnline;
        }
        emit allFriendList();

        QString loadUrl = avatar_url.isEmpty() ? ":/default/images/defaultAvatar.png" : avatar_url;

        ImageCacheManager::getManager().loadImage(loadUrl, [this, uid](const QPixmap& pix){
            emit friendAvatarUpdate(uid, pix, QSize());
        });
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

            QString pre = uid + "_";
            for(auto it = this->set_paddingAvatarSize.begin(); it != this->set_paddingAvatarSize.end();)
            {
                if(it->startsWith(pre))
                    it = this->set_paddingAvatarSize.erase(it);
                else
                    it++;
            }
        }

        QString loadUrl = avatarUrl.isEmpty() ? ":/default/images/defaultAvatar.png" : avatarUrl;

        ImageCacheManager::getManager().loadImage(loadUrl, [this, uid](const QPixmap& pix){
            emit friendAvatarUpdate(uid, pix, QSize());
        });
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
            this->set_paddingAvatarSize.clear();

            for(auto& var : std::as_const(list))
            {
                FriendInfo info;
                info.uid = var.value("UID").toString();
                info.sid = var.value("SID").toString();
                info.username = var.value("Username").toString();
                info.email = var.value("Email").toString();
                info.avatarUrl = var.value("AvatarUrl").toString();
                info.isOnline = var.value("IsOnline").toBool();
                this->map_friend[info.uid] = info;

                if(!info.avatarUrl.isEmpty())
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

            QString loadUrl = it.value().isEmpty() ? ":/default/images/defaultAvatar.png" : it.value();
            ImageCacheManager::getManager().loadImage(loadUrl, [this, uid](const QPixmap& pix){
                emit friendAvatarUpdate(uid, pix, QSize());
            });
        }
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &FriendManage::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &FriendManage::cleanAll);
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, &FriendManage::cleanAll);
}
