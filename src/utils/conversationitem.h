#ifndef CONVERSATIONITEM_H
#define CONVERSATIONITEM_H

#include <QObject>

class ConversationItem : public QObject
{
    Q_OBJECT
public:
    explicit ConversationItem(QObject *parent = nullptr);

signals:
};

#endif // CONVERSATIONITEM_H
