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
#include "../utils/imagecachemanager.h"

class ConversationDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ConversationDelegate(int* loadingAngle, QObject *parent = nullptr);
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
        MediaHeight,
        MediaSize,
        VideoThumbnail,
        VideoDuration,
        FileName
    };

    enum ContentType
    {
        Text = 0,
        Image,
        Video,
        File
    };

    enum Status
    {
        Sending = 0,
        Success,
        Failed
    };

signals:
    void ReSendClicked(const QString& tempMsgID);
    void ReloadImageClicked(const QString& msgID);
    void previewImageClicked(const QString& url);
    void previewVideoClicked(const QString& url);

protected:
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

private:
    QString formatTimestamp(int64_t timestamp) const;
    int getViewportWidth(const QStyleOptionViewItem& option) const;

    void getLayout(const QStyleOptionViewItem &option, const QModelIndex& index, QRect& timestamp, QRect& contain, QRect& avatarRect, QRect& textRegionRect, QRect& statusRect, int& textTotalHeight) const;
    int* loadingAngle = nullptr;



signals:
};

#endif // CONVERSATIONDELEGATE_H
