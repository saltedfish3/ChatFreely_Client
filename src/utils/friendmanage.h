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
#include "../utils/imagecachemanager.h"

class TcpLongConnection;

class FriendManage : public QObject
{
    Q_OBJECT
public:
    struct FriendInfo
    {
        QString conversationID;
        QString uid;
        QString sid;
        QString username;
        QString avatarUrl;
        QString email;
        bool isOnline;
    };

    static FriendManage& getFriendManage();
    FriendManage& operator=(const FriendManage&) = delete;
    FriendManage(const FriendManage&) = delete;

    int64_t getFriendCount() const;
    FriendManage::FriendInfo getFriendInfo(const QString& conversationID) const;
    QPixmap getFriendAvatar(const QString& conversationID, const QSize& wantedSize);
    QString getFriendConversationIDByUID(const QString& uid);
    QString getFriendUIDBySID(const QString& SID);

    QList<FriendManage::FriendInfo> getAllFriend() const;

    void resetLoadState();

signals:
    //用于更新全部好友列表
    void allFriendList();
    //用于登录后首次好友列表完成更新后
    void loadFirstAllFriendList();

    //用于专门的更新
    void friendAvatarUpdate(const QString& uid, const QPixmap& avatar, const QSize& size);
    void friendStatusUpdate(const QString& uid, bool isOnline);
    void friendUsernameUpdate(const QString& uid, const QString& username);

private slots:
    void cleanAll();

private:
    explicit FriendManage(QObject *parent = nullptr);
    QHash<QString, FriendInfo> hash_friend;//conversationID
    QHash<QString, QString> hash_UIDToConversatioID;//UID - conversationID
    QSet<QString> set_paddingAvatarSize;

    mutable QReadWriteLock lock;

    bool isFirstLoad = false;
};

#endif // FRIENDMANAGE_H
