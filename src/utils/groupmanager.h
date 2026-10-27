#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QPixmap>
#include <QReadWriteLock>
#include "../utils/imagecachemanager.h"
#include "../utils/GlobalVariable.h"
#include "../database/databasemanager.h"

class TcpLongConnection;

class GroupManager : public QObject
{
    Q_OBJECT
public:
    struct GroupMemberInfo
    {
        QString username;
        QString uid;
        QString sid;
        QString avatarUrl;
    };

    struct GroupInfo
    {
        QString conversationID;
        QString groupID;
        QString groupName;
        QString avatarUrl;
        QString ownerUID;
        QHash<QString, GroupMemberInfo> memberInfos;//UID -> Info
        bool isDeleted = false;
    };

    static GroupManager& getGroupManage();
    GroupManager& operator=(const GroupManager&) = delete;
    GroupManager(const GroupManager&) = delete;

    GroupManager::GroupInfo getGroupInfo(const QString& conversationID) const;
    QList<GroupManager::GroupInfo> getAllGroups() const;

    void setGroupAvatarUrl(const QString& conversationID, const QString& avatarUrl);
    QPixmap getGroupAvatar(const QString& conversationID, const QSize& wantedSize);
    int64_t getGroupsCount();

    void initLeftFromDatabase(const QList<DatabaseManager::DeletedConversationIDInfo>& list);

    void resetLoadState();

signals:
    void groupCreated(const QString& conversatinonID);
    void allGroupList();
    void loadFirstAllGroupList();

    void groupAvatarUpdate(const QString& conversationID, const QPixmap& avatar, const QSize& size);
    void groupInfoUpdate(const QString& conversationID);

private slots:
    void cleanAll();

private:
    explicit GroupManager(QObject *parent = nullptr);

    QHash<QString, GroupInfo> hash_groups;//conversationID
    QSet<QString> set_paddingAvatarSize;
    QHash<QString, GroupInfo> hash_left;//conversationID

    mutable QReadWriteLock lock;
    bool isFirstLoad = false;
};

#endif // GROUPMANAGER_H
