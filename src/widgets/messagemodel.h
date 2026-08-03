#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "../chat/conversationitem.h"

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role
    {
        IsMyselfRole = Qt::UserRole + 1,
        IsNeedShowTime,
        ContentRole,
        AvatarRole,
        TimeStamp,
        MessageIDRole,
        ConvSeqRole,
        MessageStatusRole
    };

    explicit MessageModel(ConversationItem* item, QObject *parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;

public slots:
    void onNewMessage(const Message& msg);
    void onMessageStatusChanged(const QString& tempMsgID, Status status);
    void loadHistoryMessages(const QList<Message>& msgs);

signals:

private:
    ConversationItem* item;

};

#endif // MESSAGEMODEL_H
