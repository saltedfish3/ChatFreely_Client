#include "databasemanager.h"

DatabaseManager &DatabaseManager::getDatabaseManager()
{
    static DatabaseManager db;
    return db;
}

void DatabaseManager::prepareToSwitchUser()
{
    while(this->isRunning || !this->callConversation.isEmpty())
        QCoreApplication::processEvents();

    QMutexLocker locker(&(this->mutex_task));
    this->taskQueues.clear();
    this->callConversation.clear();
    this->isRunning = false;
}

bool DatabaseManager::changeToCurrentUser()
{
    prepareToSwitchUser();
    QString uid = UserInfo::getUserInfo().getUID();
    QString sid = UserInfo::getUserInfo().getSID();

    if(uid.isEmpty() || sid.isEmpty())
    {
        qWarning() << "用户ID获取失败";
        return false;
    }

    if(this->currentUID == uid)
        return true;

    this->currentUID = uid;
    this->currentSID = sid;

    QDir dir;
    if (!dir.mkpath(this->dbPath + "/" + this->currentSID)) {
        qWarning() << "无法创建用户目录:" << dir.absolutePath();
        return false;
    }

    QString dbFilePath = this->dbPath + "/" + this->currentSID + "/chat.db";
    if (QFile::exists(dbFilePath))
    {
        QFileInfo fi(dbFilePath);
        if (fi.size() == 0)
        {
            QFile::remove(dbFilePath);
        }
    }

    QString connName = QUuid::createUuid().toString(QUuid::Id128);
    {
        QSqlDatabase db = openConnection(connName);
        if(!db.isOpen())
        {
            qWarning() << "打开用户专属数据库失败" << sid;
            return false;
        }

        bool isSuccess = true;
        {
            QSqlQuery sqlq(db);
            isSuccess &= sqlq.exec("CREATE TABLE IF NOT EXISTS conversations("
                        "conversation_id INTEGER PRIMARY KEY,"
                        "unread_count INTEGER DEFAULT 0)");

            isSuccess &= sqlq.exec("CREATE TABLE IF NOT EXISTS messages("
                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "conversation_id INTEGER NOT NULL,"
                        "server_msg_id INTEGER UNIQUE,"
                        "temp_msg_id TEXT UNIQUE,"
                        "sender_id INTEGER,"
                        "content TEXT,"
                        "timestamp INTEGER,"
                        "conv_seq INTEGER,"
                        "status INTEGER,"
                        "show_timestamp INTEGER DEFAULT 1,"
                        "is_myself INTEGER)");

            isSuccess &= sqlq.exec("CREATE INDEX IF NOT EXISTS idx_msg_conv_seq ON messages(conversation_id, conv_seq)");

            isSuccess &= sqlq.exec("CREATE TABLE IF NOT EXISTS conversation_checkpoint("
                                  "conversation_id INTEGER PRIMARY KEY,"
                                  "last_stored_seq INTEGER DEFAULT 0)");

            isSuccess &= sqlq.exec("CREATE TABLE IF NOT EXISTS user_meta("
                                   "key TEXT PRIMARY KEY, "
                                   "value TEXT)");

            if(!isSuccess)
            {
                qWarning() << "初始化数据表失败" << sqlq.lastError().text();
                db.close();
                db = QSqlDatabase();
                QSqlDatabase::removeDatabase(connName);
                return false;
            }
            sqlq.finish();
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(connName);
    return true;
}

void DatabaseManager::addInsertMessageTask(const QString &conversationID, const Message &msg)
{
    DBTask task;
    task.type = InsertMessage;
    task.conversationID = conversationID;
    task.msg = msg;
    addTask(conversationID, task);
}

void DatabaseManager::addUpdateMessageTask(const QString &conversationID, const Message &msg)
{
    DBTask task;
    task.type = UpdateMessage;
    task.conversationID = conversationID;
    task.msg = msg;
    addTask(conversationID, task);
}

void DatabaseManager::addUpdateUnreadTask(const QString &conversationID, int unreadCount)
{
    DBTask task;
    task.type = UpdateUnread;
    task.conversationID = conversationID;
    task.unreadCount = unreadCount;
    addTask(conversationID, task);
}

bool DatabaseManager::setMeta(MetaType type, const QString &value)
{
    QString connName = QUuid::createUuid().toString(QUuid::Id128);
    {
        QSqlDatabase db = openConnection(connName);
        if(db.isOpen())
        {
            QSqlQuery q(db);
            QString sql = "INSERT OR REPLACE INTO user_meta(key, value) VALUES(?, ?)";
            q.prepare(sql);
            QString key;
            switch(type)
            {
            case SyncConvSeq:
                key = "SyncConvSeq";
                break;
            };
            q.addBindValue(key);
            q.addBindValue(value);
            if(!q.exec())
            {
                qWarning() << "插入或更新user_meta失败：key:" << key << "value:" << value;
                return false;
            }
        }
        else
        {
            qWarning() << "打开数据库失败";
            return false;
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(connName);
    return true;
}

bool DatabaseManager::getMeta(MetaType type, QString &outValue)
{
    QString connName = QUuid::createUuid().toString(QUuid::Id128);
    {
        QSqlDatabase db = openConnection(connName);
        if(db.isOpen())
        {
            QSqlQuery q(db);
            QString sql = "SELECT value FROM user_meta WHERE key = ?";
            q.prepare(sql);
            QString key;
            switch(type)
            {
            case SyncConvSeq:
                key = "SyncConvSeq";
                break;
            default:
                qWarning() << "未知的MetaType：" << type;
                return false;
            };
            q.addBindValue(key);
            if(!q.exec())
            {
                qWarning() << "查询user_meta失败：key:" << key;
                return false;
            }
            if(q.next())
            {
                outValue = q.value(0).toString();
            }
            else
            {
                outValue = QString();
            }
        }
        else
        {
            qWarning() << "打开数据库失败";
            return false;
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(connName);
    return true;
}

bool DatabaseManager::getConversationsLastConvSeq(const QString &conversationID, qint64& outValue)
{
    QString connName = QUuid::createUuid().toString(QUuid::Id128);
    {
        QSqlDatabase db = openConnection(connName);
        if(db.isOpen())
        {
            QSqlQuery q(db);
            QString sql = "SELECT last_stored_seq FROM conversation_checkpoint WHERE conversation_id = ?";
            q.prepare(sql);

            q.addBindValue(toInt64(conversationID));
            if(!q.exec())
            {
                qWarning() << "查询conversation_checkpoint失败：conversation_id:" << conversationID;
                return false;
            }
            if(q.next())
            {
                outValue = q.value(0).toLongLong();
            }
            else
            {
                outValue = 0;
            }
        }
        else
        {
            qWarning() << "打开数据库失败";
            return false;
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(connName);
    return true;
}

void DatabaseManager::loadAllConversationsList(std::function<void (const QList<ConversationInfo>&)> callback)
{
    auto future = QtConcurrent::run([this, callback](){
        QList<ConversationInfo> list;
        QString connName = QUuid::createUuid().toString(QUuid::Id128);
        {
            QSqlDatabase db = openConnection(connName);
            if(db.isOpen())
            {
                QSqlQuery q(db);
                QString sql = "SELECT cp.conversation_id, COALESCE(c.unread_count, 0), m.content, m.timestamp "
                              "FROM conversation_checkpoint cp "
                              "LEFT JOIN conversations c ON cp.conversation_id = c.conversation_id "
                              "LEFT JOIN messages m ON cp.conversation_id = m.conversation_id "
                              "AND m.conv_seq = ("
                              "SELECT MAX(m2.conv_seq) FROM messages m2 "
                              "WHERE m2.conversation_id = cp.conversation_id) "
                              "GROUP BY cp.conversation_id "
                              "ORDER BY m.timestamp DESC";

                if(q.exec(sql))
                {
                    while(q.next())
                    {
                        ConversationInfo info;
                        info.conversationID = QString::number(q.value(0).toLongLong());
                        info.unReadCount = q.value(1).toInt();
                        info.lastMsg = q.value(2).toString();
                        info.lastTimestamp = q.value(3).toLongLong();
                        list.append(info);
                    }
                }
                else
                    qWarning() << "加载会话列表失败:" << q.lastError().text();
            }
            db.close();
        }
        QSqlDatabase::removeDatabase(connName);
        QMetaObject::invokeMethod(QCoreApplication::instance(), [callback, list](){
            if(callback)
                callback(list);
        }, Qt::QueuedConnection);
    });
}

void DatabaseManager::loadConversationMessages(const QString &conversationID, int limit, qint64 endConvSeq, std::function<void (const QList<Message>&)> callback)
{
    auto future = QtConcurrent::run([this, conversationID, limit, endConvSeq, callback](){
        QString connName = QUuid::createUuid().toString(QUuid::Id128);
        QList<Message> msgs;
        {
            QSqlDatabase db = openConnection(connName);
            QSqlQuery q(db);
            qint64 convID = toInt64(conversationID);
            if(convID == 0)
            {
                qWarning() << "载入历史消息convID转int64失败";
                return;
            }
            QString sql = "SELECT server_msg_id, temp_msg_id, sender_id, content, timestamp, conv_seq, status, show_timestamp "
                          "FROM messages WHERE conversation_id = ?";
            if(endConvSeq > 0)
                sql += " AND conv_seq < ?";
            sql += " ORDER BY conv_seq DESC LIMIT ?";
            q.prepare(sql);

            q.addBindValue(convID);
            if(endConvSeq > 0)
                q.addBindValue(endConvSeq);
            q.addBindValue(limit);

            if(q.exec())
            {
                while(q.next())
                {
                    Message msg;
                    QVariant var_serverMsgID = q.value(0);
                    if(!var_serverMsgID.isNull())
                        msg.serverMsgID = QString::number(var_serverMsgID.toLongLong());

                    msg.tempMsgID = q.value(1).toString();
                    msg.senderUID = QString::number(q.value(2).toLongLong());
                    msg.content = q.value(3).toString();
                    msg.timeStamp = q.value(4).toLongLong();
                    msg.convSeq = q.value(5).toLongLong();
                    Status oldStatus = static_cast<Status>(q.value(6).toInt());
                    msg.status = oldStatus == Sending ? Failed : oldStatus;
                    msg.showTimestamp = q.value(7).toBool();
                    msgs.prepend(msg);

                    //防止Sending状态的消息永远处于sending
                    if(oldStatus == Sending)
                        addUpdateMessageTask(conversationID, msg);
                }
            }
            else
                qWarning() << "查询历史记录失败：" << q.lastError().text();

            db.close();
        }
        QSqlDatabase::removeDatabase(connName);
        QMetaObject::invokeMethod(QCoreApplication::instance(), [callback, msgs](){
            if(callback)
                callback(msgs);
        }, Qt::QueuedConnection);
    });
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject{parent}
{
    this->dbPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/Record";
    QDir().mkpath(this->dbPath);
}

QSqlDatabase DatabaseManager::openConnection(const QString &connectName)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectName);
    db.setDatabaseName(getDatabasePath());

    if(!db.open())
    {
        qWarning()<< "打开数据库失败" << db.lastError().text();
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectName);
    }
    return db;
}

QString DatabaseManager::getDatabasePath() const
{
    if(this->currentSID.isEmpty())
        return this->dbPath + "/_temp.db";
    return this->dbPath + "/" + this->currentSID + "/chat.db";
}

qint64 DatabaseManager::toInt64(const QString &str)
{
    bool isSuccess = false;
    qint64 val = str.toLongLong(&isSuccess);
    if(!isSuccess)
        return 0;
    return val;
}

void DatabaseManager::addTask(const QString &conversationID, const DBTask &task)
{
    QMutexLocker locker(&(this->mutex_task));
    auto& queue = this->taskQueues[conversationID];
    queue.enqueue(task);

    if(!this->callConversation.contains(conversationID))
        this->callConversation.enqueue(conversationID);

    if(!this->isRunning.exchange(true))
    {
        auto future = QtConcurrent::run([this](){
            while(!this->callConversation.isEmpty() && this->isRunning)
            {
                if(!executeTask())
                    break;
            }
            this->isRunning = false;
        });
    }
}

void DatabaseManager::backTask(const QString &conversationID, const DBTask &task)
{
    QMutexLocker locker(&(this->mutex_task));
    auto& queue = this->taskQueues[conversationID];
    queue.push_front(task);

    if(!this->callConversation.contains(conversationID))
        this->callConversation.push_front(conversationID);
}

bool DatabaseManager::getTask(DBTask &outTask)
{
    QMutexLocker locker(&(this->mutex_task));
    if(this->callConversation.isEmpty())
        return false;

    QString callingConversationID = this->callConversation.dequeue();
    auto& queue = this->taskQueues[callingConversationID];

    if(queue.isEmpty())
        return false;

    outTask = queue.dequeue();

    if(!queue.isEmpty())
        this->callConversation.enqueue(callingConversationID);

    return true;
}

bool DatabaseManager::executeTask()
{
    DBTask task;
    QString connName = QUuid::createUuid().toString(QUuid::Id128);
    {
        QSqlDatabase db = openConnection(connName);
        if(!db.isOpen())
        {
            db.close();
            db = QSqlDatabase();
            QSqlDatabase::removeDatabase(connName);
            return false;
        }

        while(true)
        {
            if(getTask(task))
            {
                if(task.type == InsertMessage)
                {
                    executeInsertMessageTask(db, task);
                }
                else if(task.type == UpdateMessage)
                {
                    executeUpdateMessageTask(db, task);
                }
                else if(task.type == UpdateUnread)
                {
                    executeUpdateUnreadTask(db, task);
                }
            }
            else
            {
                QMutexLocker locker(&this->mutex_task);
                if(this->callConversation.isEmpty())
                    break;
            }
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(connName);
    return true;
}

bool DatabaseManager::executeInsertMessageTask(const QSqlDatabase& db, const DBTask &task)
{
    QSqlQuery q_msg(db);
    q_msg.prepare("INSERT OR REPLACE INTO messages(conversation_id, server_msg_id, temp_msg_id, sender_id, content, timestamp, conv_seq, status, show_timestamp, is_myself) "
                  "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    q_msg.addBindValue(toInt64(task.conversationID));
    q_msg.addBindValue(task.msg.serverMsgID.isEmpty() ? QVariant() : QVariant(toInt64(task.msg.serverMsgID)));
    q_msg.addBindValue(task.msg.tempMsgID);
    q_msg.addBindValue(toInt64(task.msg.senderUID));
    q_msg.addBindValue(task.msg.content);
    q_msg.addBindValue(task.msg.timeStamp);
    q_msg.addBindValue(task.msg.convSeq);
    q_msg.addBindValue(static_cast<int>(task.msg.status));
    q_msg.addBindValue(static_cast<int>(task.msg.showTimestamp));
    q_msg.addBindValue(static_cast<int>(task.msg.senderUID == UserInfo::getUserInfo().getUID()));

    if(!q_msg.exec())
    {
        qWarning() << "插入Message失败"<< q_msg.lastError().driverText() << q_msg.lastError().databaseText();
        return false;
    }

    QSqlQuery q_checkpoint(db);
    q_checkpoint.prepare("INSERT INTO conversation_checkpoint(conversation_id, last_stored_seq) VALUES(?, ?) "
                         "ON CONFLICT(conversation_id) DO UPDATE SET last_stored_seq = MAX(last_stored_seq, excluded.last_stored_seq)");
    q_checkpoint.addBindValue(toInt64(task.conversationID));
    q_checkpoint.addBindValue(task.msg.convSeq);

    if(!q_checkpoint.exec())
    {
        qWarning() << "插入或更新conversation_checkpoint失败"<< q_checkpoint.lastError().driverText() << q_checkpoint.lastError().databaseText();
    }
    return true;
}

bool DatabaseManager::executeUpdateMessageTask(const QSqlDatabase &db, const DBTask &task)
{
    QSqlQuery q(db);
    q.prepare("INSERT INTO messages(conversation_id, server_msg_id, temp_msg_id, sender_id, content, timestamp, conv_seq, status, show_timestamp, is_myself) "
              "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
              "ON CONFLICT(temp_msg_id) DO UPDATE SET "
              "server_msg_id = excluded.server_msg_id, "
              "timestamp = excluded.timestamp, "
              "conv_seq = excluded.conv_seq, "
              "status = excluded.status, "
              "show_timestamp = excluded.show_timestamp");
    q.addBindValue(toInt64(task.conversationID));
    q.addBindValue(task.msg.serverMsgID.isEmpty() ? QVariant() : QVariant(toInt64(task.msg.serverMsgID)));
    q.addBindValue(task.msg.tempMsgID);
    q.addBindValue(toInt64(task.msg.senderUID));
    q.addBindValue(task.msg.content);
    q.addBindValue(task.msg.timeStamp);
    q.addBindValue(task.msg.convSeq);
    q.addBindValue(static_cast<int>(task.msg.status));
    q.addBindValue(static_cast<int>(task.msg.showTimestamp));
    q.addBindValue(static_cast<int>(task.msg.senderUID == UserInfo::getUserInfo().getUID()));

    if(!q.exec())
    {
        qWarning() << "更新Messages失败"<< q.lastError().driverText() << q.lastError().databaseText();
        return false;
    }
    return true;
}

bool DatabaseManager::executeUpdateUnreadTask(const QSqlDatabase &db, const DBTask &task)
{
    QSqlQuery q(db);
    q.prepare("INSERT INTO conversations(conversation_id, unread_count) VALUES(?, ?)"
              " ON CONFLICT(conversation_id) DO UPDATE SET unread_count = excluded.unread_count");
    q.addBindValue(toInt64(task.conversationID));
    q.addBindValue(task.unreadCount);

    if(!q.exec())
    {
        qWarning() << "更新Conversations失败"<< q.lastError().driverText() << q.lastError().databaseText();
        return false;
    }
    return true;
}
