#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <QString>
#include <QPixmap>
#include <mutex>
#include "../network/httpshortconnection.h"

class UserInfo : public QObject
{
    Q_OBJECT
public:
    static UserInfo& getUserInfo();
    void setUsername(const QString& username);
    void setEmail(const QString& email);
    void setSID(const QString& SID);
    void setUID(const QString& UID);

    void setAvatar(const QPixmap& avatar);
    void confirmAvatar();
    void rollBackAvatar();
    void backupAvatar();

    void sendUpdateSignal();

signals:
    void updateInfo(QString username, QString email, QString sid);
    void updateAvatar(QPixmap avatar);

private:
    explicit UserInfo(QObject *parent = nullptr);

    QString username;
    QString email;
    QString sid;
    QString uid;
    QPixmap avatar;
    QPixmap old_avatar;

    std::mutex mutex;
};

#endif // USERINFO_H
