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
    std::lock_guard<std::mutex> lock(this->mutex);
    if(username.isEmpty())
        return;
    this->username = username;
}

QString UserInfo::getUsername()
{
    return this->username;
}

QPixmap UserInfo::getAvatar()
{
    return this->avatar;
}

QString UserInfo::getAccessToken()
{
    return this->accessToken;
}

QString UserInfo::getRefreshToken()
{
    return this->refreshToken;
}

void UserInfo::updateUsername(const QString &username)
{
    if(username.trimmed() == UserInfo::getUserInfo().getUsername())
        return;
    this->waitingUpdate_username = username;
    TcpLongConnection::getTcpClient().sendUpdateUsername(username);
}

void UserInfo::confirmUsername()
{
    if(this->waitingUpdate_username.isEmpty())
        return;
    this->username = this->waitingUpdate_username;
    this->waitingUpdate_username.clear();
    emit sendUpdateSignal();
}

bool UserInfo::isLogin()
{
    return this->is_login;
}

void UserInfo::setEmail(const QString &email)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    if(email.isEmpty())
        return;
    this->email = email;
}

void UserInfo::setSID(const QString &SID)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    if(SID.isEmpty())
        return;
    this->sid = SID;
}

void UserInfo::setUID(const QString &UID)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    if(UID.isEmpty())
        return;
    this->uid = UID;
}

void UserInfo::setLogin(bool islogin)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    this->is_login = islogin;
}

void UserInfo::setAccessToken(const QString &accessToken)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    this->accessToken = accessToken;
}

void UserInfo::setRefreshToken(const QString &refreshToken)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    this->refreshToken = refreshToken;
}

QString UserInfo::getUID()
{
    return this->uid;
}

void UserInfo::setAvatar(const QPixmap &avatar)
{
    std::lock_guard<std::mutex> lock(this->mutex);
    if(avatar.isNull())
        return;
    this->avatar = avatar;
    emit updateAvatar(this->avatar);
}

void UserInfo::confirmAvatar()
{
    std::lock_guard<std::mutex> lock(this->mutex);
    if(this->avatar.isNull())
        return;
    this->old_avatar = this->avatar;
}

void UserInfo::rollBackAvatar()
{
    std::lock_guard<std::mutex> lock(this->mutex);
    if(this->old_avatar.isNull())
        return;
    this->avatar = this->old_avatar;
    emit updateAvatar(this->avatar);
}

void UserInfo::backupAvatar()
{
    std::lock_guard<std::mutex> lock(this->mutex);
    this->old_avatar = this->avatar;
}

void UserInfo::sendUpdateSignal()
{
    std::lock_guard<std::mutex> lock(this->mutex);
    if(this->username.isEmpty() || this->email.isEmpty() || this->sid.isEmpty())
        return;
    emit updateInfo(this->username, this->email, this->sid);
}

void UserInfo::cleanALL()
{
    std::lock_guard<std::mutex> lock(this->mutex);
    this->username = QString();
    this->waitingUpdate_username = QString();
    this->avatar = QString();
    this->old_avatar = QString();
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
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::AvatarReady, this, [this](QPixmap pixmap){
        setAvatar(pixmap);
        emit updateAvatar(this->avatar);
    });
    this->is_login = false;
}
