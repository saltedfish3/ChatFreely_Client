#include "userinfo.h"
#include "../network/httpshortconnection.h"
#include "../network/tcplongconnection.h"

UserInfo &UserInfo::getUserInfo()
{
    static UserInfo uinfo;
    return uinfo;
}

void UserInfo::setUsername(const QString &username)
{
    QWriteLocker locker(&(this->rwLock));
    if(username.isEmpty())
        return;
    this->username = username;
}

QString UserInfo::getUsername()
{
    QReadLocker locker(&(this->rwLock));
    return this->username;
}

QPixmap UserInfo::getAvatar(const QSize &wantedSize)
{
    QString loadUrl;
    {
        QReadLocker locker(&(this->rwLock));
        loadUrl = this->avatarUrl.isEmpty() ? ":/default/images/defaultAvatar.png" : this->avatarUrl;
    }
    QString key = uid + QString("_%1x%2_%3").arg(wantedSize.width()).arg(wantedSize.height()).arg(loadUrl);

    //查询头像是否已存在
    QPixmap pix = ImageCacheManager::getManager().fastLoadImage(loadUrl, -1, 0, 0, wantedSize);
    if(!pix.isNull())
        return pix;

    pix = ImageCacheManager::getManager().fastLoadImage(":/default/images/defaultAvatar.png", -1, 0, 0, wantedSize);

    //任务去重
    {
        QWriteLocker locker(&(this->rwLock));
        if(this->set_paddingAvatarSize.contains(key))
        {
            if(!pix.isNull())
                return pix;
            return QPixmap(":/default/images/defaultAvatar.png");
        }
        else
            this->set_paddingAvatarSize.insert(key);
    }

    if(!loadUrl.isEmpty())
    {
        QPixmap original = ImageCacheManager::getManager().fastLoadImage(loadUrl, 1.0);
        if(!original.isNull())
        {
            QPointer pointer(this);
            ImageCacheManager::getManager().loadImage(loadUrl, [this, key, loadUrl, pointer, wantedSize](const QPixmap& pix){
                if(!pointer)
                    return;

                QString currentUrl;
                {
                    QWriteLocker locker(&(pointer->rwLock));
                    currentUrl = pointer->avatarUrl;
                    this->set_paddingAvatarSize.remove(key);
                }
                if(currentUrl == loadUrl)
                    emit updateAvatar(pix, wantedSize);
            }, false, -1, 0, 0, wantedSize);

            return original;
        }
    }

    //url为空 或 默认头像处理
    QPointer<UserInfo> pointer(this);
    ImageCacheManager::getManager().loadImage(loadUrl, [this, pointer, key, loadUrl, wantedSize](const QPixmap& pix){
        if(!pointer)
            return;

        {
            QWriteLocker locker(&(this->rwLock));
            pointer->set_paddingAvatarSize.remove(key);
        }

        QString current;
        {
            QReadLocker locker(&(this->rwLock));
            current = pointer->avatarUrl;
        }
        bool needUpdate = current.isEmpty() ? loadUrl == ":/default/images/defaultAvatar.png" : current == loadUrl;

        if(needUpdate)
            emit updateAvatar(pix, wantedSize);
    }, false, -1, 0, 0, wantedSize);

    if(!pix.isNull())
        return pix;
    return QPixmap(":/default/images/defaultAvatar.png");
}

QString UserInfo::getAvatarUrl()
{
    QReadLocker locker(&(this->rwLock));
    return this->avatarUrl;
}

QString UserInfo::getAccessToken()
{
    QReadLocker locker(&(this->rwLock));
    // qDebug()<<this->accessToken;
    return this->accessToken;
}

QString UserInfo::getRefreshToken()
{
    QReadLocker locker(&(this->rwLock));
    return this->refreshToken;
}

void UserInfo::updateUsername(const QString &username)
{
    if(username.trimmed() == UserInfo::getUserInfo().getUsername())
        return;
    {
        QWriteLocker locker(&(this->rwLock));
        this->waitingUpdate_username = username;
    }
    TcpLongConnection::getTcpClient().sendUpdateUsername(username);
}

void UserInfo::confirmUsername()
{
    {
        QWriteLocker locker(&(this->rwLock));
        if(this->waitingUpdate_username.isEmpty())
            return;
        this->username = this->waitingUpdate_username;
        this->waitingUpdate_username.clear();
    }
    emit sendUpdateSignal();
}

bool UserInfo::isLogin()
{
    QReadLocker locker(&(this->rwLock));
    return this->is_login;
}

void UserInfo::setEmail(const QString &email)
{
    QWriteLocker locker(&(this->rwLock));
    if(email.isEmpty())
        return;
    this->email = email;
}

void UserInfo::setSID(const QString &SID)
{
    QWriteLocker locker(&(this->rwLock));
    if(SID.isEmpty())
        return;
    this->sid = SID;
}

void UserInfo::setUID(const QString &UID)
{
    QWriteLocker locker(&(this->rwLock));
    if(UID.isEmpty())
        return;
    this->uid = UID;
}

void UserInfo::setLogin(bool islogin)
{
    QWriteLocker locker(&(this->rwLock));
    this->is_login = islogin;
}

void UserInfo::setAccessToken(const QString &accessToken)
{
    QWriteLocker locker(&(this->rwLock));
    this->accessToken = accessToken;
    qDebug()<<this->accessToken;
}

void UserInfo::setRefreshToken(const QString &refreshToken)
{
    QWriteLocker locker(&(this->rwLock));
    this->refreshToken = refreshToken;
}

QString UserInfo::getUID()
{
    QReadLocker locker(&(this->rwLock));
    return this->uid;
}

QString UserInfo::getSID()
{
    QReadLocker locker(&(this->rwLock));
    return this->sid;
}

void UserInfo::setAvatarUrl(const QString &url)
{
    QString loadUrl;
    {
        QWriteLocker locker(&(this->rwLock));
        if(!url.isEmpty() && this->avatarUrl == url)
            return;

        QString pre = uid + "_";
        for(auto it = this->set_paddingAvatarSize.begin(); it != this->set_paddingAvatarSize.end();)
        {
            if(it->startsWith(pre))
                it = this->set_paddingAvatarSize.erase(it);
            else
                it++;
        }
        this->avatarUrl = url;
        loadUrl = this->avatarUrl;
    }

    if(loadUrl.isEmpty())
        loadUrl = ":/default/images/defaultAvatar.png";

    QPointer pointer(this);
    ImageCacheManager::getManager().loadImage(loadUrl, [this, pointer, loadUrl](const QPixmap& pix){
        if(!pointer)
            return;

        QString currentUrl;
        {
            QReadLocker locker(&(pointer->rwLock));
            currentUrl = pointer->avatarUrl.isEmpty() ? ":/default/images/defaultAvatar.png" : pointer->avatarUrl;
        }

        if(currentUrl == loadUrl)
        {
            if(pix.isNull())
            {
                ImageCacheManager::getManager().loadImage(":/default/images/defaultAvatar.png", [this](const QPixmap& pix){
                    if(!pix.isNull())
                        emit updateAvatar(pix, QSize());
                }, false);
                return;
            }
            emit updateAvatar(pix, QSize());
        }
    }, true);
}

void UserInfo::confirmAvatarUrl()
{
    QWriteLocker locker(&(this->rwLock));
    if(this->avatarUrl.isEmpty())
        return;
    this->old_avatarUrl = this->avatarUrl;
}

void UserInfo::rollBackAvatarUrl()
{
    QString loadUrl;
    {
        QWriteLocker locker(&(this->rwLock));
        if(this->old_avatarUrl.isEmpty())
            return;
        this->avatarUrl = this->old_avatarUrl;
        loadUrl = this->avatarUrl;

        QString pre = uid + "_";
        for(auto it = this->set_paddingAvatarSize.begin(); it != this->set_paddingAvatarSize.end();)
        {
            if(it->startsWith(pre))
                it = this->set_paddingAvatarSize.erase(it);
            else
                it++;
        }
    }

    QPointer pointer(this);
    ImageCacheManager::getManager().loadImage(loadUrl, [this, pointer, loadUrl](const QPixmap& pix){
        if(!pointer)
            return;

        QString currentUrl;
        {
            QReadLocker locker(&(pointer->rwLock));
            currentUrl = pointer->avatarUrl;
        }

        if(loadUrl == currentUrl && !pix.isNull())
            emit updateAvatar(pix, QSize());
    }, false);
}

void UserInfo::backupAvatarUrl()
{
    QWriteLocker locker(&(this->rwLock));
    this->old_avatarUrl = this->avatarUrl;
}

void UserInfo::sendUpdateSignal()
{
    QString username, email, sid;
    {
        QWriteLocker locker(&(this->rwLock));
        if(this->username.isEmpty() || this->email.isEmpty() || this->sid.isEmpty())
            return;
        username = this->username;
        email = this->email;
        sid = this->sid;
    }
    emit updateInfo(username, email, sid);
}

void UserInfo::cleanALL()
{
    QWriteLocker locker(&(this->rwLock));
    this->username = QString();
    this->waitingUpdate_username = QString();
    this->set_paddingAvatarSize.clear();
    this->avatarUrl = QString();
    this->old_avatarUrl = QString();
    this->uid = QString();
    this->sid = QString();
    this->accessToken = QString();
    this->refreshToken = QString();
    this->email = QString();
    this->is_login = false;
}

UserInfo::UserInfo(QObject *parent)
    : QObject{parent}
{
    this->is_login = false;

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, [this](){
        cleanALL();
    });
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, [this](){
        cleanALL();
    });

    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, [this](){
        cleanALL();
    });
}
