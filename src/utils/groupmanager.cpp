#include "groupmanager.h"

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

GroupManager::GroupManager(QObject *parent)
    : QObject{parent}
{}
