#ifndef SEARCHRESULTLISTDELEGATE_H
#define SEARCHRESULTLISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QPainterPath>
#include <QDateTime>
#include "../utils/GlobalVariable.h"

class SearchResultListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum Role
    {
        AvatarRole = Qt::UserRole + 1,
        UsernameRole,
        ContentRole,
        TimestampRole,
        MsgIDRole,
        ConvSeqRole,
        IsReturnBackItemRole,
        HighLightKeywordRole,
        SenderUIDRole
    };

    explicit SearchResultListDelegate(QObject *parent = nullptr);

signals:
    void itemClicked(const QModelIndex& index);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    QString formatTimestamp(int64_t timestamp) const;

};

#endif // SEARCHRESULTLISTDELEGATE_H
