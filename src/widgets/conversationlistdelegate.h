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

class ConversationListDelegate : public QStyledItemDelegate
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
        LastMsgRole,
        LastTimestampRole,
        UnReadRole,
        IsSelectedRole
    };
    explicit ConversationListDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    QPixmap setRadius(const QPixmap& pixmap, int hei_wid) const;
    QString formatTimestamp(int64_t timestamp) const;

signals:
    void itemClicked(const QModelIndex& index);
};

#endif // CONVERSATIONLISTDELEGATE_H
