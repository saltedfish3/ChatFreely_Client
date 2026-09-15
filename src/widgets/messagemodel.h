#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "../chat/messagesmanager.h"
#include "../utils/userinfo.h"
#include "../utils/friendmanage.h"
#include "../utils/imagecachemanager.h"

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role
    {
        IsMyselfRole = Qt::UserRole + 1,
        IsNeedShowTime,
        ContentTypeRole,
        ContentRole,
        AvatarRole,
        TimeStamp,
        MessageIDRole,
        ConvSeqRole,
        MessageStatusRole,
        ImageRole,
        ImageStateRole,
        MediaUrl,
        MediaWidth,
        MediaHeight
    };

    explicit MessageModel(MessagesManager* manager, QObject *parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

public slots:
    void onMessageAdd(int row);
    void onMessagesAdd(int first, int end);
    void onMessageUpdate(int row);
    void onMessagesUpdate(int first, int end);
    void onMessagePrepend(int count);
    void onMessageRemove(int row);
    void onMessageMove(int oldRow, int newRow);
    void onMessageFriendAvatarUpdate(const QString& uid, const QPixmap& avatar, const QSize& size);
    void onMessageMyselfAvatarUpdate(const QPixmap& avatar, const QSize& size);
    void onResetModel();

signals:

private:
    MessagesManager* manager;

};

#endif // MESSAGEMODEL_H
