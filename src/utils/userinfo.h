#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <QString>
#include <QPixmap>
#include <QReadWriteLock>
#include <QSet>

class HttpShortConnection;
class TcpLongConnection;

class UserInfo : public QObject
{
    Q_OBJECT
public:
    static UserInfo& getUserInfo();

    UserInfo(const UserInfo&) = delete;
    UserInfo& operator=(const UserInfo&) = delete;

    void setUsername(const QString& username);
    void updateUsername(const QString& username);
    void confirmUsername();
    bool isLogin();

    void setEmail(const QString& email);
    void setSID(const QString& SID);
    void setUID(const QString& UID);
    void setLogin(bool islogin);
    void setAccessToken(const QString& accessToken);
    void setRefreshToken(const QString& refreshToken);

    QString getUID();
    QString getSID();
    QString getUsername();
    QPixmap getAvatar(const QSize &wantedSize);
    QString getAvatarUrl();
    QString getAccessToken();
    QString getRefreshToken();

    void setAvatarUrl(const QString& url);
    void confirmAvatarUrl();
    void rollBackAvatarUrl();
    void backupAvatarUrl();

    void sendUpdateSignal();

    void cleanALL();

signals:
    void updateInfo(QString username, QString email, QString sid);
    void updateAvatar(const QPixmap& avatar, const QSize& size);

private:
    explicit UserInfo(QObject *parent = nullptr);

    QString username;
    QString waitingUpdate_username;
    QString email;
    QString sid;
    QString uid;
    QString avatarUrl;
    QString old_avatarUrl;
    QString accessToken;
    QString refreshToken;
    bool is_login;

    QSet<QString> set_paddingAvatarSize;
    QReadWriteLock rwLock;
};

#endif // USERINFO_H
