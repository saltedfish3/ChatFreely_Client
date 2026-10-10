#include "searchresultlistdelegate.h"

SearchResultListDelegate::SearchResultListDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{}

void SearchResultListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QRect contain = option.rect.adjusted(-1, 0, 1, 0);

    bool isHovered = option.state & QStyle::State_MouseOver;

    painter->setBrush(Qt::NoBrush);
    if(isHovered)
        painter->setBrush(QColor(243, 244, 246));
    painter->drawRect(contain);

    if(index.data(IsReturnBackItemRole).toBool())
    {
        QFont font = option.font;
        font.setPointSize(10);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(QColor(70, 72, 212));

        QRect textRect = contain.adjusted(16, 0, -16, 0);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, "← 返回搜索结果");
    }
    else
    {
        QSize avatarSize(36, 36);
        QRect avatar(contain.topLeft() + QPoint(10, (contain.height() - avatarSize.height())/2), avatarSize);
        QPixmap avatarIcon = index.data(AvatarRole).value<QPixmap>();
        if(avatarIcon.isNull())
            avatarIcon = QPixmap(":/default/images/defaultAvatar.png");

        painter->save();
        QPainterPath path;
        path.addRoundedRect(avatar, avatarSize.width()/2, avatarSize.height()/2);
        painter->setClipPath(path);
        painter->drawPixmap(avatar, avatarIcon);
        painter->restore();

        QFont font = option.font;
        font.setPointSize(10);
        font.setBold(true);

        painter->setFont(font);

        QString text = index.data(UsernameRole).toString();
        QFontMetrics fm = painter->fontMetrics();

        int textHeight = fm.height();
        QRect username(avatar.topLeft() + QPoint(avatarSize.width() + 8, 0), QSize(85, textHeight));

        QString drawText = fm.elidedText(text, Qt::ElideRight, username.width());
        painter->setPen(Qt::black);
        painter->drawText(username, Qt::AlignVCenter, drawText);

        font.setPointSizeF(7.5);
        font.setBold(false);
        painter->setFont(font);
        painter->setPen(QColor(128, 128, 128));
        fm = painter->fontMetrics();
        QRect timestampRect(username.topRight(), QSize(contain.topRight().x() - username.topRight().x() - 5, fm.height()));
        QString drawTime = formatTimestamp(index.data(TimestampRole).toLongLong());
        painter->drawText(timestampRect, Qt::AlignVCenter | Qt::AlignRight, drawTime);

        QString content = index.data(ContentRole).toString();
        QString keyword = index.data(HighLightKeywordRole).toString();

        font.setPointSize(8);
        font.setBold(false);
        painter->setFont(font);
        painter->setPen(QColor(85, 85, 85));
        fm = painter->fontMetrics();

        int contentHeight = fm.height();
        QRect contentRect(username.bottomLeft() + QPoint(0, 5), QSize(140, contentHeight));
        int availableWidth = contentRect.width();

        QString visible;
        int hightLightStartInVis = -1;
        int hightLightLen = 0;

        if(availableWidth > 0)
        {
            int keywordPos = keyword.isEmpty() ? -1 : content.indexOf(keyword, 0, Qt::CaseInsensitive);

            if(keywordPos < 0)
                visible = fm.elidedText(content, Qt::ElideRight, availableWidth);
            else if(fm.horizontalAdvance(content) <= availableWidth)
            {
                visible = content;
                hightLightStartInVis = keywordPos;
                hightLightLen = keyword.length();
            }
            else
            {
                int keywordWidth = fm.horizontalAdvance(keyword);
                int remainWidth = availableWidth - keywordWidth;

                if(remainWidth <= 0)
                {
                    visible = fm.elidedText(keyword, Qt::ElideRight, availableWidth);
                    hightLightStartInVis = 0;
                    hightLightLen = visible.length();
                }
                else
                {
                    int frontWidth = remainWidth / 2;
                    int backWidth = remainWidth - frontWidth;

                    QString pre = content.left(keywordPos);
                    QString back = content.mid(keywordPos + keyword.length());

                    QString preText = fm.elidedText(pre, Qt::ElideLeft, frontWidth);
                    QString backText = fm.elidedText(back, Qt::ElideRight, backWidth);

                    visible = preText + keyword + backText;
                    hightLightStartInVis = preText.length();
                    hightLightLen = keyword.length();
                }
            }
        }

        if(visible.isEmpty())
        {}
        else if(hightLightStartInVis < 0)
        {
            painter->setPen(QColor(85, 85, 85));
            painter->drawText(contentRect, Qt::AlignVCenter | Qt::AlignLeft, visible);
        }
        else
        {
            QString pre = visible.left(hightLightStartInVis);
            QString key = visible.mid(hightLightStartInVis, hightLightLen);
            QString back = visible.mid(hightLightStartInVis + hightLightLen);

            int y = contentRect.top() + (contentRect.height() - fm.height()) / 2 + fm.ascent();
            int x = contentRect.left();

            if(!pre.isEmpty())
            {
                painter->setPen(QColor(85, 85, 85));
                painter->drawText(x, y, pre);
                x += fm.horizontalAdvance(pre);
            }

            if(!key.isEmpty())
            {
                int keyWidth = fm.horizontalAdvance(key);
                int topY = y - fm.ascent();
                painter->fillRect(QRect(x, topY, keyWidth, fm.height()), QColor(255, 235, 59));
                painter->setPen(Qt::black);
                painter->drawText(x, y, key);
                x += keyWidth;
            }

            if(!back.isEmpty())
            {
                painter->setPen(QColor(85, 85, 85));
                painter->drawText(x, y, back);
            }
        }
    }

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QColor(240, 240, 240));
    painter->drawLine(contain.bottomLeft() + QPoint(contain.width()*0.1, 0), contain.bottomRight() - QPoint(contain.width()*0.1, 0));

    painter->restore();
}

QSize SearchResultListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if(index.data(IsReturnBackItemRole).toBool())
        return QSize(-1, 44);
    return QSize(-1, 60);
}

bool SearchResultListDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if(event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* mouse = static_cast<QMouseEvent*>(event);
        if(option.rect.contains(mouse->pos()))
        {
            emit itemClicked(index);
            return true;
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QString SearchResultListDelegate::formatTimestamp(int64_t timestamp) const
{
    if(timestamp <= 0)
        return {};

    //插入时间戳
    QDateTime now = QDateTime::currentDateTime();
    QDateTime msgTime = QDateTime::fromSecsSinceEpoch(timestamp);
    QString timeStr;
    //非本年
    if(msgTime.date().year() != now.date().year())
    {
        timeStr = msgTime.toString("yyyy/M/d");
    }
    else if(msgTime.date() == now.date())
    {
        //今天
        timeStr = msgTime.toString("hh:mm");
    }
    else if(msgTime.date() == now.date().addDays(-1))
    {
        //昨天
        timeStr = QString("昨天 %1").arg(msgTime.toString("hh:mm"));
    }
    else if(msgTime.date() == now.date().addDays(-2))
    {
        //前天
        timeStr = QString("前天 %1").arg(msgTime.toString("hh:mm"));
    }
    else
    {
        int mondayOfTime = now.date().dayOfWeek() - 1;
        QDate thisMonday = now.date().addDays(-mondayOfTime);
        QDate thisSunday = thisMonday.addDays(6);
        if(msgTime.date() >= thisMonday && msgTime.date() <= thisSunday)
        {
            QStringList weekDays = {"","星期一","星期二","星期三","星期四","星期五","星期六","星期日"};
            timeStr = QString("%1").arg(weekDays.at(msgTime.date().dayOfWeek()));
        }
        else
        {
            timeStr = msgTime.toString("M/d");
        }
    }
    return timeStr;
}
