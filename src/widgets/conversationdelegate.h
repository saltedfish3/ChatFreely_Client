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
#include <QDateTime>

class ConversationDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ConversationDelegate(int* loadingAngle, QObject *parent = nullptr);
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

    enum Status
    {
        Sending = 0,
        Success,
        Failed
    };

signals:
    void ReSendClicked(const QString& tempMsgID);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    QString formatTimestamp(int64_t timestamp) const;
    QPixmap setRadius(const QPixmap& pixmap, int hei_wid) const;
    void getLayout(const QStyleOptionViewItem &option, const QModelIndex& index, QRect& timestamp, QRect& contain, QRect& avatarRect, QRect& textRegionRect, QRect& statusRect, int& textTotalHeight) const;
    int* loadingAngle = nullptr;



signals:
};

#endif // CONVERSATIONDELEGATE_H
