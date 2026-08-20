#ifndef GLOBALINITCONTROLLER_H
#define GLOBALINITCONTROLLER_H

#include <QObject>
#include "../network/tcplongconnection.h"
#include "../utils/friendmanage.h"
#include "../database/databasemanager.h"
#include "../chat/conversationitem.h"
#include "../chat/conversationmanager.h"

class GlobalInitController : public QObject
{
    Q_OBJECT
public:
    enum class Step
    {
        Free,
        LoadingFriends,
        LoadingNewFriendRequests,
        LoadingLocalConversations,
        SyncingGlobalSeq,
        SyncingMessages,
        MigratingFile
    };

    static GlobalInitController& getController();
    GlobalInitController& operator=(const GlobalInitController&) = delete;
    GlobalInitController(const GlobalInitController&) = delete;

    void start();
    Step step() const;
    bool restartFromLogin();

    //加载好友列表
    void loadFriendList();
    //加载新好友申请列表
    void loadNewFriendRequests();
    //加载会话列表
    void loadLocalConversations();
    //同步最新全局Seq
    void syncGlobalSeq();
    //同步会话最新消息
    void syncNewMessages();

    //开始数据库等文件迁移
    void startMigrateFile(const QString& newDBPath);

signals:
    //登录成功与否信号
    void loginResult(bool isSuccess, const QString& from, const QString& info, bool reLogin);

    //好友首次加载完成
    void friendListFirstLoaded();

    //会话列表加载完成
    void allConversationsLoaded(const QList<QPair<ConversationItem *, DatabaseManager::ConversationInfo>>& list);

    //迁移文件结果
    void migrateFinished(bool isSuccess);

    //通知设置界面更新聊天文件存储路径信息
    void chatRecordPathChanged();

private:
    explicit GlobalInitController(QObject *parent = nullptr);
    bool migrateDirectory(const QString& oldDir, const QString& newDir);
    void checkUnFinishMigrate();

    Step nowStep = Step::Free;

    QSet<QString> waitingSyncConversationID;
    QSet<QString> syncingConversationID;
    QHash<QString, int> retryCounts;
};

#endif // GLOBALINITCONTROLLER_H
