#include "userinfo.h"

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

UserInfo::UserInfo(QObject *parent)
    : QObject{parent}
{
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::AvatarReady, this, [this](QPixmap pixmap){
        setAvatar(pixmap);
        emit updateAvatar(this->avatar);
    });

}
