#include "groupmanager.h"
#include "../network/tcplongconnection.h"

GroupManager &GroupManager::getGroupManage()
{
    static GroupManager man;
    return man;
}

GroupManager::GroupInfo GroupManager::getGroupInfo(const QString &conversationID) const
{
    QReadLocker lock(&this->lock);

    auto it = this->hash_groups.find(conversationID);
    if(it != this->hash_groups.end())
        return it.value();

    auto lit = this->hash_left.find(conversationID);
    if(lit != this->hash_left.end())
        return lit.value();

    return {};
}

QList<GroupManager::GroupInfo> GroupManager::getAllGroups() const
{
    QReadLocker lock(&this->lock);
    QList<GroupManager::GroupInfo> list;
    for(const auto& item : this->hash_groups)
        list.append(item);

    return list;
}

void GroupManager::setGroupAvatarUrl(const QString &conversationID, const QString &avatarUrl)
{
    if(conversationID.isEmpty() || !this->hash_groups.contains(conversationID))
        return;

    {
        QWriteLocker locker(&(this->lock));
        GroupInfo& info = this->hash_groups[conversationID];
        info.avatarUrl = avatarUrl;
    }

    emit groupInfoUpdate(conversationID);
}

QPixmap GroupManager::getGroupAvatar(const QString &conversationID, const QSize &wantedSize)
{
    QString url = getGroupInfo(conversationID).avatarUrl;
    QString loadUrl = url.isEmpty() ? ":/default/images/defaultAvatar.png" : url;
    QString key = conversationID + QString("_%1x%2_%3").arg(wantedSize.width()).arg(wantedSize.height()).arg(loadUrl);

    //查询头像是否已存在
    QPixmap pix = ImageCacheManager::getManager().fastLoadImage(loadUrl, 0, 0, wantedSize);
    if(!pix.isNull())
        return pix;

    pix = ImageCacheManager::getManager().fastLoadImage(":/default/images/defaultAvatar.png", 0, 0, wantedSize);

    //任务去重
    {
        QWriteLocker locker(&(this->lock));
        if(this->set_paddingAvatarSize.contains(key))
        {
            if(!pix.isNull())
                return pix;
            pix = QPixmap(":/default/images/defaultAvatar.png");
            pix.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
            return pix;
        }
        else
            this->set_paddingAvatarSize.insert(key);
    }

    if(!url.isEmpty())
    {
        QPixmap original = ImageCacheManager::getManager().fastLoadImage(url, 1.0);
        if(!original.isNull())
        {
            ImageCacheManager::getManager().loadImage(url, [this, conversationID, key, url, wantedSize](const QPixmap& pix){
                QString currentUrl = getGroupInfo(conversationID).avatarUrl;
                {
                    QWriteLocker locker(&(this->lock));
                    this->set_paddingAvatarSize.remove(key);
                }
                if(currentUrl == url)
                    emit groupAvatarUpdate(conversationID, pix, wantedSize);
            }, false, 0, 0, wantedSize);

            return original;
        }
    }

    //url为空 或 默认头像处理
    QPointer<GroupManager> pointer(this);
    ImageCacheManager::getManager().loadImage(loadUrl, [this, pointer, conversationID, key, loadUrl, wantedSize](const QPixmap& pix){
        if(!pointer)
            return;

        {
            QWriteLocker locker(&(this->lock));
            pointer->set_paddingAvatarSize.remove(key);
        }

        QString current = pointer->getGroupInfo(conversationID).avatarUrl;
        bool needUpdate = current.isEmpty() ? loadUrl == ":/default/images/defaultAvatar.png" : current == loadUrl;

        if(needUpdate)
        {
            if(pix.isNull())
            {
                ImageCacheManager::getManager().loadImage(":/default/images/defaultAvatar.png", [this, conversationID, wantedSize](const QPixmap& pix){
                    if(!pix.isNull())
                    {
                        emit groupAvatarUpdate(conversationID, pix, wantedSize);
                    }
                }, false, 0, 0, wantedSize);
                return;
            }
            emit groupAvatarUpdate(conversationID, pix, wantedSize);
        }
    }, false, 0, 0, wantedSize);

    if(!pix.isNull())
        return pix;
    pix = QPixmap(":/default/images/defaultAvatar.png");
    pix.setDevicePixelRatio(GlobalVariable::getMaxDevicePixelRatio());
    return pix;
}

int64_t GroupManager::getGroupsCount()
{
    QReadLocker lock(&this->lock);
    return this->hash_groups.size();
}

void GroupManager::initLeftFromDatabase(const QList<DatabaseManager::DeletedConversationIDInfo> &list)
{
    QWriteLocker locker(&(this->lock));
    this->hash_left.clear();

    for(const auto& in : list)
    {
        GroupInfo info;
        info.conversationID = in.conversationID;
        info.groupID = in.peerID;
        info.groupName = in.displayName;
        info.avatarUrl = in.avatarUrl;
        info.isDeleted = true;

        this->hash_left[in.conversationID] = info;
    }
}

void GroupManager::resetLoadState()
{
    QWriteLocker locker(&(this->lock));
    this->isFirstLoad = false;
}

void GroupManager::cleanAll()
{
    QWriteLocker locker(&(this->lock));
    this->hash_groups.clear();
    this->set_paddingAvatarSize.clear();
    this->hash_left.clear();
    this->isFirstLoad = false;
}

GroupManager::GroupManager(QObject *parent)
    : QObject{parent}
{
    QPointer pointer(this);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::createGroupSuccess, this, [pointer](const QString& groupID, const QString& conversationID, const QString& groupName){
        if(!pointer)
            return;

        GroupInfo info;
        info.conversationID = conversationID;
        info.groupID = groupID;
        info.groupName = groupName;
        info.ownerUID = UserInfo::getUserInfo().getUID();

        GroupMemberInfo ownerMember;
        ownerMember.uid = UserInfo::getUserInfo().getUID();
        ownerMember.sid = UserInfo::getUserInfo().getSID();
        ownerMember.username = UserInfo::getUserInfo().getUsername();
        ownerMember.avatarUrl = UserInfo::getUserInfo().getAvatarUrl();
        info.memberInfos[ownerMember.uid] = ownerMember;

        {
            QWriteLocker locker(&pointer->lock);
            pointer->hash_groups[conversationID] = info;
            pointer->hash_left.remove(conversationID);
        }

        emit pointer->groupCreated(conversationID);
        emit pointer->groupInfoUpdate(conversationID);
        emit pointer->allGroupList();
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::allGroupList, this, [pointer](const QList<QVariantMap>& list, bool isSuccess){
        if(!pointer)
            return;

        if(!isSuccess)
        {
            if(!pointer->isFirstLoad)
            {
                pointer->isFirstLoad = true;
                emit pointer->loadFirstAllGroupList();
            }
            return;
        }

        QList<DatabaseManager::DeletedConversationIDInfo> paddingDeleted;
        QMap<QString, QString> avatarTasks;
        {
            QWriteLocker locker(&pointer->lock);

            QSet<QString> newMembers;
            for(const auto& var : list)
                newMembers.insert(var.value("ConversationID").toString());

            for(auto it = pointer->hash_groups.begin(); it != pointer->hash_groups.end(); it++)
            {
                if(newMembers.contains(it.key()))
                    continue;

                if(pointer->hash_left.contains(it.key()))
                    continue;

                GroupInfo info = it.value();
                info.isDeleted = true;
                pointer->hash_left[it.key()] = info;

                DatabaseManager::DeletedConversationIDInfo in;
                in.conversationID = it.key();
                in.conversationType = 1;
                in.peerID = info.groupID;
                in.displayName = info.groupName;
                in.avatarUrl = info.avatarUrl;

                paddingDeleted.append(in);
            }

            pointer->hash_groups.clear();
            pointer->set_paddingAvatarSize.clear();

            for(auto& var : std::as_const(list))
            {
                GroupInfo info;
                info.conversationID = var.value("ConversationID").toString();
                info.groupID = var.value("GroupID").toString();
                info.groupName = var.value("GroupName").toString();
                info.avatarUrl = var.value("Avatar_Url").toString();
                info.ownerUID = var.value("OwnerUID").toString();

                QVariantList memberInfos = var.value("MemberInfos").toList();
                for(const auto& mv : std::as_const(memberInfos))
                {
                    QVariantMap m = mv.toMap();
                    GroupMemberInfo in;
                    in.uid = m.value("UID").toString();
                    in.sid = m.value("SID").toString();
                    in.username = m.value("Username").toString();
                    in.avatarUrl = m.value("AvatarUrl").toString();
                    info.memberInfos[m.value("UID").toString()] = in;
                }

                pointer->hash_groups[info.conversationID] = info;
                pointer->hash_left.remove(info.conversationID);

                if(!info.avatarUrl.isEmpty())
                    avatarTasks[info.conversationID] = info.avatarUrl;
            }
        }

        emit pointer->allGroupList();
        if(!pointer->isFirstLoad)
        {
            pointer->isFirstLoad = true;
            emit pointer->loadFirstAllGroupList();
        }

        for(auto it = avatarTasks.begin(); it != avatarTasks.end(); it++)
        {
            QString conversationID = it.key();

            QString loadUrl = it.value().isEmpty() ? ":/default/images/defaultAvatar.png" : it.value();
            ImageCacheManager::getManager().loadImage(loadUrl, [pointer, conversationID](const QPixmap& pix){
                emit pointer->groupAvatarUpdate(conversationID, pix, QSize());
            });
        }

        for(const auto& in : paddingDeleted)
            DatabaseManager::getDatabaseManager().upsertDeletedConversation(in);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newGroup, this, [pointer](const QString& conversationID, const QString& groupID, const QString& groupName,
                                                                                           const QString& avatarUrl, const QString& ownerUID, const QVariantList& memberInfos){
        if(!pointer)
            return;

        {
            QWriteLocker locker(&pointer->lock);
            GroupInfo& info = pointer->hash_groups[conversationID];
            info.groupID = groupID;
            info.groupName = groupName;
            info.avatarUrl = avatarUrl;
            info.ownerUID = ownerUID;
            info.conversationID = conversationID;
            info.memberInfos.clear();
            info.isDeleted = false;

            for(const auto& mv : std::as_const(memberInfos))
            {
                QVariantMap m = mv.toMap();
                GroupMemberInfo in;
                in.uid = m.value("UID").toString();
                in.sid = m.value("SID").toString();
                in.username = m.value("Username").toString();
                in.avatarUrl = m.value("AvatarUrl").toString();
                info.memberInfos[m.value("UID").toString()] = in;
            }

            pointer->hash_left.remove(conversationID);
        }

        emit pointer->groupInfoUpdate(conversationID);
        emit pointer->allGroupList();
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::groupInfoFetched, this, [pointer, this](const QVariantMap& info){
        if(!pointer)
            return;

        QString conversationID = info.value("ConversationID").toString();

        {
            QWriteLocker locker(&pointer->lock);

            if(pointer->hash_left.contains(conversationID))
                return;

            GroupInfo& info_g = pointer->hash_groups[conversationID];
            info_g.groupID = info.value("GroupID").toString().isEmpty() ? info_g.groupID : info.value("GroupID").toString();
            info_g.groupName = info.value("GroupName").toString().isEmpty() ? info_g.groupName : info.value("GroupName").toString();
            info_g.avatarUrl = info.value("Avatar_Url").toString();
            info_g.ownerUID = info.value("OwnerUID").toString().isEmpty() ? info_g.ownerUID : info.value("OwnerUID").toString();;
            info_g.conversationID = conversationID;
            info_g.memberInfos.clear();

            QVariantList memberInfos = info.value("MemberInfos").toList();
            info_g.memberInfos.clear();
            for(const auto& mv : std::as_const(memberInfos))
            {
                QVariantMap m = mv.toMap();
                GroupMemberInfo in;
                in.uid = m.value("UID").toString();
                in.sid = m.value("SID").toString();
                in.username = m.value("Username").toString();
                in.avatarUrl = m.value("AvatarUrl").toString();
                info_g.memberInfos[m.value("UID").toString()] = in;
            }
        }

        emit pointer->groupInfoUpdate(conversationID);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::updateGroupName, this, [pointer](const QString& conversationID, const QString& groupName){
        if(!pointer)
            return;

        {
            QWriteLocker locker(&pointer->lock);
            GroupInfo& info = pointer->hash_groups[conversationID];
            info.groupName = groupName;
        }

        emit pointer->groupInfoUpdate(conversationID);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::updateGroupAvatar, this, [pointer](const QString& conversationID, const QString& avatarUrl){
        if(!pointer)
            return;

        {
            QWriteLocker locker(&pointer->lock);
            GroupInfo& info = pointer->hash_groups[conversationID];
            info.avatarUrl = avatarUrl;
        }

        emit pointer->groupInfoUpdate(conversationID);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitGroup, this, [pointer](const QString& conversationID){
        if(!pointer)
            return;

        GroupInfo info;
        {
            QWriteLocker locker(&(pointer->lock));

            auto it = pointer->hash_groups.find(conversationID);
            if(it == pointer->hash_groups.end())
                return;

            info = it.value();
            info.isDeleted = true;

            pointer->hash_groups.remove(conversationID);
            pointer->hash_left[conversationID] = info;
        }

        DatabaseManager::DeletedConversationIDInfo in;
        in.conversationID = conversationID;
        in.conversationType = 1;
        in.peerID = info.groupID;
        in.peerSID = QString();
        in.displayName = info.groupName;
        in.avatarUrl = info.avatarUrl;
        DatabaseManager::getDatabaseManager().upsertDeletedConversation(in);

        emit pointer->groupInfoUpdate(conversationID);
        emit pointer->allGroupList();
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::groupDeletedConversationInfo, this, [pointer](const QList<QVariantMap>& list, bool isSuccess){
        if(!pointer || !isSuccess || list.isEmpty())
            return;

        QList<DatabaseManager::DeletedConversationIDInfo> list_deleted;
        {
            QWriteLocker locker(&(pointer->lock));
            for(const auto& var : list)
            {
                QString conversationID = var.value("ConversationID").toString();
                if(conversationID.isEmpty())
                    continue;

                if(pointer->hash_groups.contains(conversationID))
                    continue;

                GroupInfo info = pointer->hash_left[conversationID];
                info.conversationID = conversationID;
                info.groupName = var.value("GroupName").toString();
                info.avatarUrl = var.value("AvatarUrl").toString();
                info.groupID = var.value("GroupID").toString();
                info.isDeleted = true;
                pointer->hash_left[conversationID] = info;

                DatabaseManager::DeletedConversationIDInfo in;
                in.conversationID = conversationID;
                in.conversationType = 1;
                in.peerID = info.groupID;
                in.displayName = info.groupName;
                in.avatarUrl = info.avatarUrl;

                list_deleted.append(in);
            }
        }

        for(const auto& in : list_deleted)
            DatabaseManager::getDatabaseManager().upsertDeletedConversation(in);

        for(const auto& in : list_deleted)
            emit pointer->groupInfoUpdate(in.conversationID);

        if(!list_deleted.isEmpty())
            emit pointer->allGroupList();
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &GroupManager::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &GroupManager::cleanAll);
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, &GroupManager::cleanAll);
}
