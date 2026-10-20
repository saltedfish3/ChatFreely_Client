#ifndef GROUPMEMBERDELEGATE_H
#define GROUPMEMBERDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QRect>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QMouseEvent>
#include "../utils/GlobalVariable.h"

class GroupMemberDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum Role
    {
        UsernameRole = Qt::UserRole + 1,
        SIDRole,
        AvatarRole,
        UIDRole
    };

    explicit GroupMemberDelegate(QObject *parent = nullptr);

signals:

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

};

#endif // GROUPMEMBERDELEGATE_H
