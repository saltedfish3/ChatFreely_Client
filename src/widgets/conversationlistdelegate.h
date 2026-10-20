#ifndef CONVERSATIONLISTDELEGATE_H
#define CONVERSATIONLISTDELEGATE_H

#include <QObject>
#include <QAbstractItemView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QDateTime>
#include "../utils/GlobalVariable.h"

class ConversationListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum Role
    {
        UsernameRole = Qt::UserRole + 1,
        ConversationIDRole,
        SIDRole,
        EmailRole,
        AvatarRole,
        IsOnlineRole,
        LastMsgRole,
        LastTimestampRole,
        UnReadRole,
        IsGroupRole,
        GroupMemberCountRole,
        IsSelectedRole,
        IsSearchModeRole
    };
    explicit ConversationListDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    QString formatTimestamp(int64_t timestamp) const;

signals:
    void itemClicked(const QModelIndex& index);
};

#endif // CONVERSATIONLISTDELEGATE_H
