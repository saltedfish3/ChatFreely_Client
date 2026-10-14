#ifndef GROUPMANAGER_H
#define GROUPMANAGER_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QPixmap>
#include <QReadWriteLock>
#include "../utils/imagecachemanager.h"
#include "../utils/GlobalVariable.h"

class GroupManager : public QObject
{
    Q_OBJECT
public:
    struct GroupInfo
    {
        QString groupID;
        QString groupName;
        QString avatarUrl;
        QString ownerUID;
        int memberCount;
    };

    static GroupManager& getGroupManage();
    GroupManager& operator=(const GroupManager&) = delete;
    GroupManager(const GroupManager&) = delete;

    GroupManager::GroupInfo getGroupInfo(const QString& conversationID) const;
    QPixmap getGroupAvatar(const QString& conversationID, const QSize& wantedSize);

signals:
    void groupAvatarUpdate(const QString& conversationID, const QPixmap& avatar, const QSize& size);

private:
    explicit GroupManager(QObject *parent = nullptr);

    QHash<QString, GroupInfo> hash_groups;
    QSet<QString> set_paddingAvatarSize;

    mutable QReadWriteLock lock;
};

#endif // GROUPMANAGER_H
