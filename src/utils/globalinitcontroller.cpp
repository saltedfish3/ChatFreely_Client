#include "globalinitcontroller.h"

GlobalInitController &GlobalInitController::getController()
{
    static GlobalInitController gic;
    return gic;
}

void GlobalInitController::start()
{
    if(this->nowStep != Step::Free)
        return;

    loadFriendList();
}

bool GlobalInitController::restartFromLogin()
{
    checkUnFinishMigrate();

    this->waitingSyncConversationID.clear();
    this->syncingConversationID.clear();
    this->retryCounts.clear();

    this->nowStep = Step::Free;

    if(!DatabaseManager::getDatabaseManager().changeToCurrentUser())
    {
        this->nowStep = Step::Free;
        emit loginResult(false, "", "本地存储初始化失败，请检查内存是否充足", false);
        TcpLongConnection::getTcpClient().sendUnLogin();
        return false;
    }

    start();
    return true;
}

void GlobalInitController::loadFriendList()
{
    if(this->nowStep == Step::LoadingFriends)
        return;

    this->nowStep = Step::LoadingFriends;
    TcpLongConnection::getTcpClient().getFriendList();
}

void GlobalInitController::loadNewFriendRequests()
{
    if(this->nowStep == Step::LoadingNewFriendRequests)
        return;

    this->nowStep = Step::LoadingNewFriendRequests;
    TcpLongConnection::getTcpClient().getNewFriendRequestsList();
}

void GlobalInitController::loadLocalConversations()
{
    if(this->nowStep == Step::LoadingLocalConversations)
        return;

    this->nowStep = Step::LoadingLocalConversations;
    DatabaseManager::getDatabaseManager().loadAllConversationsList([this](const QList<DatabaseManager::ConversationInfo>& list){
        QList<QPair<ConversationItem*, DatabaseManager::ConversationInfo>> ls;
        for(const auto& info : list)
        {
            ConversationItem* item = ConversationManager::getConversationManager().getOrCreateConversationItem(info.conversationID);
            if(!item)
                continue;
            ls.append({item, info});
        }
        emit allConversationsLoaded(ls);
        syncGlobalSeq();
    });
}

void GlobalInitController::syncGlobalSeq()
{
    if(this->nowStep == Step::SyncingGlobalSeq)
        return;

    this->nowStep = Step::SyncingGlobalSeq;

    QString syncConvSeq;
    DatabaseManager::getDatabaseManager().getMeta(DatabaseManager::SyncConvSeq, syncConvSeq);
    TcpLongConnection::getTcpClient().getConversationsFriendUIDListAndSeq(syncConvSeq.isEmpty() ? 0 : syncConvSeq.toLongLong());
}

void GlobalInitController::syncNewMessages()
{
    if(this->waitingSyncConversationID.isEmpty() && this->syncingConversationID.isEmpty())
    {
        this->nowStep = Step::Free;
        return;
    }

    this->nowStep = Step::SyncingMessages;

    while(!this->waitingSyncConversationID.isEmpty())
    {
        QString convID = *(this->waitingSyncConversationID.begin());
        this->waitingSyncConversationID.remove(convID);
        this->syncingConversationID.insert(convID);

        ConversationItem* item = ConversationManager::getConversationManager().getOrCreateConversationItem(convID);

        if(!item)
        {
            int retry = this->retryCounts.value(convID, 0);
            if(retry < 3)
            {
                this->retryCounts.insert(convID, retry + 1);
                this->waitingSyncConversationID.insert(convID);
                this->syncingConversationID.remove(convID);
                QTimer::singleShot(100, this, [this](){
                    syncNewMessages();
                });
            }
            else
            {
                this->retryCounts.remove(convID);
                this->syncingConversationID.remove(convID);
                qWarning() << "ConversationItem获取失败，放弃该会话最新消息：" << convID;
                if(this->waitingSyncConversationID.isEmpty() && this->syncingConversationID.isEmpty())
                {
                    this->nowStep = Step::Free;
                }
            }
            continue;
        }

        qint64 maxConvSeq = -1;
        if(DatabaseManager::getDatabaseManager().getConversationsLastConvSeq(convID, maxConvSeq))
        {
            TcpLongConnection::getTcpClient().sendSyncNewMessages(convID, maxConvSeq);
        }
        else
        {
            qWarning() << "接收Tcp newSyncConvStatus信号getConversationsLastConvSeq失败：进行全量获取";
            TcpLongConnection::getTcpClient().sendSyncNewMessages(convID, 0);
        }
    }
}

void GlobalInitController::startMigrateFile(const QString &newDBPath)
{
    if(this->nowStep == Step::MigratingFile)
        return;

    Step oldStep = this->nowStep;
    this->nowStep = Step::MigratingFile;

    DatabaseManager::getDatabaseManager().stopHandleTask();

    bool success = false;
    QString oldDBPath = GlobalVariable::getPosOfChatRecord();
    QString fullyPath = newDBPath + "/record";
    QString tempNewPath = fullyPath + ".tmp";

    GlobalVariable::setMigrationState(true, oldDBPath, fullyPath);

    if(oldDBPath != fullyPath)
    {
        success = migrateDirectory(oldDBPath, tempNewPath);
        if(success)
        {
            if(!QDir().rename(tempNewPath, fullyPath))
            {
                QDir(tempNewPath).removeRecursively();
                success = false;
            }
        }
        else
            QDir(tempNewPath).removeRecursively();

        if(success)
            QDir(oldDBPath).removeRecursively();
    }
    else
        success = true;

    if(success)
    {
        GlobalVariable::setPosOfChatRecord(newDBPath);
        DatabaseManager::getDatabaseManager().setNewDatabasePath(GlobalVariable::getPosOfChatRecord());

        GlobalVariable::setMigrationState(false);
    }

    DatabaseManager::getDatabaseManager().startHandleTask();

    if(success)
    {
        if(oldStep == Step::SyncingMessages || oldStep == Step::SyncingGlobalSeq)
            syncGlobalSeq();
        else
            restartFromLogin();

        emit chatRecordPathChanged();
        emit migrateFinished(true);
    }
    else
    {
        this->nowStep = oldStep;
        GlobalVariable::setMigrationState(false);
        emit migrateFinished(false);
    }
    qDebug()<<success;
}

GlobalInitController::Step GlobalInitController::step() const
{
    return this->nowStep;
}

GlobalInitController::GlobalInitController(QObject *parent)
    : QObject{parent}
{
    //登录响应处理
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::LoginState, this, [this]
            (bool isSuccess, const QString& from, const QString& info, bool reLogin){
        if(isSuccess)
        {
            if(restartFromLogin())
                emit loginResult(true, "", "", false);
        }
        else
            emit loginResult(false, from, info, reLogin);
    });

    //好友列表加载完成响应处理
    connect(&FriendManage::getFriendManage(), &FriendManage::loadFirstAllFriendList, this, [this](){
        if(this->nowStep == Step::LoadingFriends)
        {
            emit friendListFirstLoaded();
        }
    });

    //好友列表加载完成响应处理
    connect(this, &GlobalInitController::friendListFirstLoaded, this, [this](){
        if(this->nowStep == Step::LoadingFriends)
        {
            loadNewFriendRequests();
        }
    });

    //新好友申请列表加载完成响应处理
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newFriendRequestsState, this, [this](bool isEmpty){
        if(this->nowStep == Step::LoadingNewFriendRequests)
        {
            loadLocalConversations();
        }
    });

    //同步全局Seq完成响应处理
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::newSyncConvStatus, this, [this]
            (bool isSuccess, const QList<QString>& conversations, int64_t newSyncConvSeq){
        if(this->nowStep != Step::SyncingGlobalSeq)
            return;

        if(!isSuccess)
        {
            this->nowStep = Step::Free;
            return;
        }

        if(conversations.isEmpty())
        {
            this->nowStep = Step::Free;
            return;
        }

        DatabaseManager::getDatabaseManager().setMeta(DatabaseManager::SyncConvSeq, QString::number(newSyncConvSeq));

        this->waitingSyncConversationID.clear();
        this->syncingConversationID.clear();

        for(const QString& convID : conversations)
            this->waitingSyncConversationID.insert(convID);

        syncNewMessages();
    });

    //同步会话最新消息完成响应处理
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::syncMessagesStatus, this, [this]
            (bool isSuccess, const QString& conversationID, const QList<Message>& msgs){
        if(this->nowStep != Step::SyncingMessages)
            return;

        if(!isSuccess)
        {
            int retry = this->retryCounts.value(conversationID, 0);
            if(retry < 3)
            {
                this->retryCounts.insert(conversationID, retry + 1);
                this->waitingSyncConversationID.insert(conversationID);
                this->syncingConversationID.remove(conversationID);
                QTimer::singleShot(100, this, [this](){
                    syncNewMessages();
                });
            }
            else
            {
                this->retryCounts.remove(conversationID);
                this->syncingConversationID.remove(conversationID);
                qWarning() << "消息同步失败，放弃该会话最新消息：" << conversationID;
                if(this->waitingSyncConversationID.isEmpty() && this->syncingConversationID.isEmpty())
                {
                    this->nowStep = Step::Free;
                }
            }
            return;
        }

        ConversationItem* item = ConversationManager::getConversationManager().getOrCreateConversationItem(conversationID);

        if(!item)
        {
            int retry = this->retryCounts.value(conversationID, 0);
            if(retry < 3)
            {
                this->retryCounts.insert(conversationID, retry + 1);
                this->waitingSyncConversationID.insert(conversationID);
                this->syncingConversationID.remove(conversationID);
                QTimer::singleShot(100, this, [this](){
                    syncNewMessages();
                });
            }
            else
            {
                this->retryCounts.remove(conversationID);
                this->syncingConversationID.remove(conversationID);
                qWarning() << "ConversationItem获取失败，放弃该会话最新消息：" << conversationID;
                if(this->waitingSyncConversationID.isEmpty() && this->syncingConversationID.isEmpty())
                {
                    this->nowStep = Step::Free;
                }
            }
            return;
        }

        if(msgs.isEmpty())
        {
            this->retryCounts.remove(conversationID);
            this->syncingConversationID.remove(conversationID);
            if(this->waitingSyncConversationID.isEmpty() && this->syncingConversationID.isEmpty())
            {
                this->nowStep = Step::Free;
            }
        }
        else
        {
            for(const auto& msg : msgs)
            {
                item->addNewMessage(msg);
            }
        }
    });

    //断线重连成功响应处理
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::reconnectSuccess, this, [this](){
        if(!UserInfo::getUserInfo().isLogin())
            return;

        if(this->nowStep == Step::SyncingGlobalSeq || this->nowStep == Step::SyncingMessages)
        {
            this->waitingSyncConversationID.clear();
            this->syncingConversationID.clear();
            this->retryCounts.clear();
        }

        syncGlobalSeq();
    });

    //令牌登录成功响应处理
    connect(&TcpLongConnection::getTcpClient(), &TcpLongConnection::accessTokenLoginSuccess, this, [this](){
        restartFromLogin();
    });
}

bool GlobalInitController::migrateDirectory(const QString &oldDir, const QString &newDir)
{
    QDir oldD(oldDir);
    if(!oldD.exists())
        return false;

    if(!QDir().mkpath(newDir))
        return false;

    QDirIterator it(oldDir, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        it.next();
        QString relativePath = oldD.relativeFilePath(it.filePath());
        QString endPath = newDir + "/" + relativePath;
        if(it.fileInfo().isDir())
            QDir().mkpath(endPath);
        else
        {
            if(!QFile::copy(it.filePath(), endPath))
                return false;
        }
    }

    return true;
}

void GlobalInitController::checkUnFinishMigrate()
{
    if(!GlobalVariable::isMigrating())
        return;

    QString oldPath = GlobalVariable::getMigratingOldPath();
    QString newPath = GlobalVariable::getMigratingNewPath();
    QString tempPath = newPath + ".tmp";

    if(!newPath.isEmpty() && QDir(newPath).exists())
    {
        QString baseDir = newPath;
        const QString suffix = "/record";
        if(baseDir.endsWith(suffix))
        {
            baseDir.chop(suffix.length());
        }

        GlobalVariable::setPosOfChatRecord(baseDir);
        emit chatRecordPathChanged();

        DatabaseManager::getDatabaseManager().setNewDatabasePath(newPath);
        QDir(tempPath).removeRecursively();
        if(!oldPath.isEmpty() && QDir(oldPath).exists())
            QDir(oldPath).removeRecursively();
    }
    else if(!oldPath.isEmpty() && QDir(oldPath).exists())
        QDir(tempPath).removeRecursively();
    else
    {
        qWarning() << "聊天记录迁移恢复失败，严重错误";
        QString defaultDir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        GlobalVariable::setPosOfChatRecord(defaultDir);
        DatabaseManager::getDatabaseManager().setNewDatabasePath(GlobalVariable::getPosOfChatRecord());
        emit chatRecordPathChanged();
    }

    GlobalVariable::setMigrationState(false);
}
