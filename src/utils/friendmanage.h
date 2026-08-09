#ifndef FRIENDMANAGE_H
#define FRIENDMANAGE_H

#include <QObject>
#include <QMap>
#include <QPixmap>
#include <QList>
#include <QVariantMap>
#include <QReadWriteLock>
#include <QPointer>
#include "../network/httpshortconnection.h"

class TcpLongConnection;

class FriendManage : public QObject
{
    Q_OBJECT
public:
    struct FriendInfo
    {
        QString uid;
        QString sid;
        QString username;
        QPixmap avatar;
        QString avatarUrl;
        QString email;
        bool isOnline;
    };

    static FriendManage& getFriendManage();
    FriendManage& operator=(const FriendManage&) = delete;
    FriendManage(const FriendManage&) = delete;

    int64_t getFriendCount() const;
    FriendManage::FriendInfo getFriendInfo(const QString& uid) const;

    QList<FriendManage::FriendInfo> getAllFriend() const;

signals:
    //用于更新全部好友列表
    void allFriendList();
    //用于登录后首次好友列表完成更新后
    void loadFirstAllFriendList();

    //用于专门的更新
    void friendAvatarUpdate(const QString& uid, const QPixmap& avatar);
    void friendStatusUpdate(const QString& uid, bool isOnline);
    void friendUsernameUpdate(const QString& uid, const QString& username);

private slots:
    void cleanAll();

private:
    explicit FriendManage(QObject *parent = nullptr);
    QMap<QString, FriendInfo> map_friend;
    mutable QReadWriteLock lock;

    bool isFirstLoad = false;
};

#endif // FRIENDMANAGE_H
