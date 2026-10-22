#include "groupmanager.h"
#include "../network/tcplongconnection.h"

GroupManager &GroupManager::getGroupManage()
{
    static GroupManager man;
    return man;
}

GroupManager::GroupInfo GroupManager::getGroupInfo(const QString &conversationID) const
{
    auto it = this->hash_groups.find(conversationID);
    if(it == this->hash_groups.end())
        return {};
    return it.value();
}

QList<GroupManager::GroupInfo> GroupManager::getAllGroups() const
{
    QReadLocker lock(&this->lock);
    QList<GroupManager::GroupInfo> list;
    for(const auto& item : this->hash_groups)
        list.append(item);

    return list;
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

        {
            QWriteLocker locker(&pointer->lock);
            pointer->hash_groups[conversationID] = info;
        }

        emit pointer->groupCreated(conversationID);
        emit pointer->allGroupList();
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::allGroupList, this, [pointer](const QList<QVariantMap>& list){
        if(!pointer)
            return;

        QMap<QString, QString> avatarTasks;
        {
            QWriteLocker locker(&pointer->lock);
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
        }

        emit pointer->groupInfoUpdate(conversationID);
    });

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::groupInfoFetched, this, [pointer, this](const QVariantMap& info){
        if(!pointer)
            return;

        QString conversationID = info.value("ConversationID").toString();

        {
            QWriteLocker locker(&pointer->lock);
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

    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::exitAccount, this, &GroupManager::cleanAll);
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::refreshExpiredExit, this, &GroupManager::cleanAll);
    connect(&HttpShortConnection::getHttpClient(), &HttpShortConnection::refreshExpiredExit, this, &GroupManager::cleanAll);
}
