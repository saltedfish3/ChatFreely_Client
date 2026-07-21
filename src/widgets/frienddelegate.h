#ifndef FRIENDDELEGATE_H
#define FRIENDDELEGATE_H

#include <QObject>
#include <QStyledItemDelegate>
#include <QRect>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QMouseEvent>

class FriendDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum Role
    {
        UsernameRole = Qt::UserRole + 1,
        UIDRole,
        SIDRole,
        EmailRole,
        AvatarRole,
        IsOnlineRole,
        GroupRole
    };

    explicit FriendDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    QPixmap setRadius(const QPixmap& pixmap, int hei_wid) const;

signals:
    void itemClicked(const QModelIndex& index);

};

#endif // FRIENDDELEGATE_H
