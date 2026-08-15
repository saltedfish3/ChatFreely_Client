#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QStandardPaths>
#include <QDir>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtConcurrent/QtConcurrent>
#include <QUuid>
#include <QMutexLocker>
#include <atomic>
#include "../utils/userinfo.h"
#include "../chat/message.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    struct ConversationInfo
    {
        QString conversationID;
        int unReadCount = 0;
        QString lastMsg;
        qint64 lastTimestamp = 0;
    };

    enum Type
    {
        InsertMessage = 0,
        UpdateMessage,
        UpdateUnread
    };

    enum MetaType
    {
        SyncConvSeq = 0
    };

    struct DBTask
    {
        Type type;
        QString conversationID;
        Message msg;
        int unreadCount = 0;
    };

    static DatabaseManager& getDatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void prepareToSwitchUser();
    bool changeToCurrentUser();

    void addInsertMessageTask(const QString& conversationID, const Message& msg);
    void addUpdateMessageTask(const QString& conversationID, const Message& msg);
    void addUpdateUnreadTask(const QString& conversationID, int unreadCount);
    bool setMeta(MetaType type, const QString& value);
    bool getMeta(MetaType type, QString& outValue);
    bool getConversationsLastConvSeq(const QString& conversationsID, qint64& outValue);

    void loadAllConversationsList(std::function<void(const QList<ConversationInfo>&)> callback);
    void loadConversationMessages(const QString& conversationID, int limit = 20, qint64 endConvSeq = -1, std::function<void(const QList<Message>&)> callback = nullptr);

signals:

private:
    explicit DatabaseManager(QObject *parent = nullptr);

    QSqlDatabase openConnection(const QString& connectName);

    QString getDatabasePath() const;
    qint64 toInt64(const QString& str);

    void addTask(const QString& conversationID, const DBTask& task);
    void backTask(const QString& conversationID, const DBTask& task);
    bool getTask(DBTask& outTask);
    bool executeTask();
    bool executeInsertMessageTask(const QSqlDatabase& db, const DBTask& task);
    bool executeUpdateMessageTask(const QSqlDatabase& db, const DBTask& task);
    bool executeUpdateUnreadTask(const QSqlDatabase& db, const DBTask& task);

    QString dbPath;
    QString currentUID;
    QString currentSID;

    QMutex mutex_task;
    QHash<QString, QQueue<DBTask>> taskQueues;
    QQueue<QString> callConversation;
    std::atomic<bool> isRunning = false;
};

#endif // DATABASEMANAGER_H
