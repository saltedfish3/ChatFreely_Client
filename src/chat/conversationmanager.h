#ifndef CONVERSATIONMANAGER_H
#define CONVERSATIONMANAGER_H

#include <QObject>
#include "conversationitem.h"
#include "../network/tcplongconnection.h"
#include "../network/httpshortconnection.h"
#include "../utils/userinfo.h"
#include "../database/databasemanager.h"

class ConversationManager : public QObject
{
    Q_OBJECT
public:
    static ConversationManager& getConversationManager();
    ConversationItem* getOrCreateConversationItem(const QString& conversationID);

    ConversationManager(const ConversationManager&) = delete;
    ConversationManager& operator=(const ConversationManager&) = delete;

signals:
    void conversationCreated(ConversationItem* item);

private slots:
    void cleanAll();

private:
    explicit ConversationManager(QObject *parent = nullptr);

    QHash<QString, ConversationItem*> conversations;

};

#endif // CONVERSATIONMANAGER_H
