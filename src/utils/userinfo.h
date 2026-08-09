#ifndef USERINFO_H
#define USERINFO_H

#include <QObject>
#include <QString>
#include <QPixmap>
#include <mutex>

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
    QPixmap getAvatar();
    QString getAccessToken();
    QString getRefreshToken();

    void setAvatar(const QPixmap& avatar);
    void confirmAvatar();
    void rollBackAvatar();
    void backupAvatar();

    void sendUpdateSignal();

    void cleanALL();

signals:
    void updateInfo(QString username, QString email, QString sid);
    void updateAvatar(QPixmap avatar);

private:
    explicit UserInfo(QObject *parent = nullptr);

    QString username;
    QString waitingUpdate_username;
    QString email;
    QString sid;
    QString uid;
    QPixmap avatar;
    QPixmap old_avatar;
    QString accessToken;
    QString refreshToken;
    bool is_login;

    std::mutex mutex;
};

#endif // USERINFO_H
