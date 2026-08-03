#ifndef CONVERSATIONMANAGER_H
#define CONVERSATIONMANAGER_H

#include <QObject>
#include "conversationitem.h"
#include "../network/tcplongconnection.h"
#include "../utils/userinfo.h"
#include "../utils/friendmanage.h"

class ConversationManager : public QObject
{
    Q_OBJECT
public:
    static ConversationManager& getConversationManager();
    ConversationItem* getConversationItem(const QString& conversationID);

signals:
    void conversationCreated(ConversationItem* item);

private:
    explicit ConversationManager(QObject *parent = nullptr);

    QHash<QString, ConversationItem*> conversations;

};

#endif // CONVERSATIONMANAGER_H
