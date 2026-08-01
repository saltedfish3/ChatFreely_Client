#ifndef CONVERSATIONDELEGATE_H
#define CONVERSATIONDELEGATE_H

#include <QObject>
#include <QAbstractItemView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QTimer>
#include <QListView>
#include <QPointer>
#include <QTextLayout>

class ConversationDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ConversationDelegate(int* loadingAngle, QObject *parent = nullptr);
    enum Role
    {
        IsMyselfRole = Qt::UserRole + 1,
        IsTimestampRole,
        IsBottomSpaceRole,
        ContentRole,
        AvatarRole,
        TimeStamp,
        MessageIDRole,
        ConvSeqRole,
        MessageStatusRole
    };
    enum Status
    {
        Success = 0,
        Sending,
        Failed
    };

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    QPixmap setRadius(const QPixmap& pixmap, int hei_wid) const;
    int* loadingAngle = nullptr;

signals:
};

#endif // CONVERSATIONDELEGATE_H
