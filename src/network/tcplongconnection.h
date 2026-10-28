#ifndef TCPLONGCONNECTION_H
#define TCPLONGCONNECTION_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QCoreApplication>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QList>
#include <QVariantMap>
#include <atomic>
#include <unordered_set>
#include "../utils/GlobalVariable.h"
#include "../chat/message.h"
#include "../utils/imagecachemanager.h"

#define SADDR "192.168.153.128"
#define SPORT 9002

class UserInfo;
class HttpShortConnection;

class TcpLongConnection : public QObject
{
    Q_OBJECT
public:
    static TcpLongConnection& getTcpClient();
    TcpLongConnection(const TcpLongConnection&) = delete;
    TcpLongConnection& operator=(const TcpLongConnection&) = delete;

    //账号
    void sendLogin(QString email, QString password);
    void sendRegister(QString email, QString password, QString username);
    void sendUpadteAvatar(QString url);
    void sendUpdateUsername(QString username);
    void sendUnLogin();
    void sendRefreshToken(std::function<void(bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired)> callback);
    void sendAccessTokenLogin();

    //好友
    void sendAddNewFriendRequest(QString sid = "", QString email = "", QString verMsg = "");
    void sendHandleNewFriendRequest(QString handle_uid, bool isAgree);
    void getNewFriendRequestsList();
    void getFriendList();
    void sendDeleteFriend(const QString& conversationID);

    //群聊
    void sendCreateGroupRequest(const QString& groupName, const QList<QString>& inviteUIDs, const QString& inviteMsg = "");
    void sendHandleNewGroupJoinRequest(const QString& handle_requestID, bool isAgree);
    void getGroupList();
    void getGroupInfo(const QString& conversationID);
    void getNewGroupJoinRequestsList();
    void sendApplyJoinGroupRequest(const QString& groupID, const QString& verMsg);
    void sendUpdateGroupName(const QString& conversationID, const QString& groupName);
    void sendUpdateGroupAvatar(const QString& conversationID, const QString& avatarUrl);
    void sendExitGroup(const QString& conversationID);
    void sendDismissGroup(const QString& conversationID);
    void sendRemoveGroupMember(const QString& conversationID, const QString& removeUID);

    //信息获取
    void getDeletedConversationInfo(const QStringList& friendConversationIDs, const QStringList& groupConversationIDs);

    //消息
    void sendMessageTo(QString conversationID, QString message, QString tempMsgID, ContentType type);
    void sendSyncNewMessages(const QString& conversationID, qint64 startConvSeq, int limit = 30);

    void getConversationsFriendUIDListAndSeq(int64_t syncConvSeq);
    bool isConnect();

    //测试
    void closeConnection();
    void reConnect();

signals:
    void reconnectSuccess();

    //账号
    void LoginState(bool isSuccess, const QString& from, const QString& info, bool reLogin = false);
    void accessTokenLoginSuccess();
    void RegisterState(bool isSuccess, const QString& from, const QString& info);
    void mainState(bool isSuccess, const QString& info);

    //好友
    void newFriendRequestsState(bool isEmpty);
    void newFriendRequestsHandleResult(const QString& handleUID, bool isSuccess);
    void newFriendRequests(const QString& uid, const QString& sid, const QString& username, const QString& avatar_url, const QString& verMsg);
    //全量获取好友列表
    void allFriendList(const QList<QVariantMap>& list, bool isSuccess);
    //接收服务器的push
    void newFriend(const QString& conversationID, const QString& uid, const QString& sid, const QString& username, const QString& avatar_url, const QString& email, bool isOnline);
    //更新好友的状态信息
    void FriendStatus(const QString& uid, bool isOnline);
    void FriendUsername(const QString& uid, const QString& username);
    void FriendAvatar(const QString& uid, const QString& avatar);
    void friendDeleted(const QString& conversationID);

    //群聊
    void createGroupSuccess(const QString& groupID, const QString& conversationID, const QString& groupName);
    void allGroupList(const QList<QVariantMap>& list, bool isSuccess);
    void newGroupJoinRequestsState(bool isEmpty);
    void newGroupJoinRequest(const QString& requestID, const QString& groupName, int source, const QString& displayName,
                             const QString& displayAvatarUrl, const QString& verMsg, const QString& applicantSID, const QString& inviterName);
    void newGroupJoinRequestsHandleResult(const QString& handleRequestID, bool isSuccess);
    void newGroup(const QString& conversationID, const QString& groupID, const QString& groupName,
                  const QString& avatarUrl, const QString& ownerUID, const QVariantList& memberInfos);
    void groupInfoFetched(const QVariantMap& memberInfos);
    void updateGroupName(const QString& conversationID, const QString& newGroupName);
    void updateGroupNameFailed(const QString& conversationID);
    void updateGroupAvatar(const QString& conversationID, const QString& avatarUrl);
    void exitGroup(const QString& conversationID);
    void groupMemberLeft(const QString& conversationID, const QString& uid);

    //信息获取
    void groupDeletedConversationInfo(const QList<QVariantMap>& list, bool isSuccess);
    void friendDeletedConversationInfo(const QList<QVariantMap>& list, bool isSuccess);

    //用于通知业务层getConversationsListAndSeq是否成功
    void newSyncConvStatus(bool isSuccess, const QList<QString>& conversations, int64_t newSyncConvSeq);
    void syncMessagesStatus(bool isSuccess, const QString& conversationID, const QList<Message>& messages);

    void sendMessageStatus(bool isSuccess, const QString& tempMsgID, const QString& conversationID, const QString& messageID = "", int64_t timeStamp = 0, int64_t convSeq = 0);
    void pushMessage(const QString& conversationID, const QString& senderUID, ContentType type, const QString& content, const QString& messageID, int64_t timeStamp, int64_t convSeq);
    void cleanNewFriendRequestsList();
    void cleanNewGroupJoinRequestsList();
    void refreshExpiredExit();
    void exitAccount();

private:
    explicit TcpLongConnection(QObject *parent = nullptr);

    void startConnect();
    void sendHello();

    //账号
    void handleHelloResp(QJsonObject obj);
    void handleLoginResp(QJsonObject obj);
    void handleAccessTokenLoginResp(QJsonObject obj);
    void handleRegisterResp(QJsonObject obj);
    void handleUpdateAvatarResp(QJsonObject obj);
    void handleUpdateUsernameResp(QJsonObject obj);
    void handleUnLoginResp(QJsonObject obj);
    void handleRefreshTokenResp(QJsonObject obj);

    //好友
    void handleAddNewFriendRequestResp(QJsonObject obj);
    void handleHandleNewFriendRequestResp(QJsonObject obj);
    void handleGetNewFriendRequestsListResp(QJsonObject obj);
    void handleGetFriendListResp(QJsonObject obj);
    void handlePushNewFriendRequests(QJsonObject obj);
    void handlePushNewFriend(QJsonObject obj);
    void handlePushFriendStatus(QJsonObject obj);
    void handlePushFriendUsername(QJsonObject obj);
    void handlePushFriendAvatar(QJsonObject obj);
    void handleDeleteFriendResp(QJsonObject obj);
    void handlePushFriendDeleted(QJsonObject obj);

    //群聊
    void handleCreateGroupRequestResp(QJsonObject obj);
    void handleGetGroupListResp(QJsonObject obj);
    void handleGetGroupInfoResp(QJsonObject obj);
    void handlePushGroupJoinRequestResp(QJsonObject obj);
    void handleGetNewGroupJoinRequestsListResp(QJsonObject obj);
    void handleHandleNewGroupJoinRequestResp(QJsonObject obj);
    void handlePushNewGroupResp(QJsonObject obj);
    void handleApplyJoinGroupRequestResp(QJsonObject obj);
    void handleUpdateGroupNameRequestResp(QJsonObject obj);
    void handleUpdateGroupAvatarRequestResp(QJsonObject obj);
    void handleExitGroupResp(QJsonObject obj);
    void handleDismissGroupResp(QJsonObject obj);
    void handlePushGroupDismissResp(QJsonObject obj);
    void handleRemoveGroupMemberResp(QJsonObject obj);
    void handlePushRemoveGroupMemberResp(QJsonObject obj);

    //信息获取
    void handleGetDeletedConversationInfoResp(QJsonObject obj);

    //消息
    void handleGetConversationsFriendUIDListAndSeqResp(QJsonObject obj);
    void handleSyncNewMessagesResp(QJsonObject obj);
    void handlePushNewMessage(QJsonObject obj);
    void handleSendMessageResp(QJsonObject obj);

    uint64_t getRequestsId();

    QTcpSocket* socket;
    QTimer* clock_retry;
    QTimer* clock_heartbeat;

    std::unordered_set<std::string> waiting_requestsID;
    QHash<QString, QString> hash_conversationToCurrentReq;//conversation -> requestID
    std::map<QString, std::function<void(bool, const QString&, bool)>> refreshCallBack;

    //曾经是否连接成功过
    bool hasConnected = false;

    bool isTryToUnLoginRefresh = false;
    bool isTryToLoginAgain = false;
    bool isTryToAccessTokenLogin = false;

    //幂等变量
    enum OperationName
    {
        UpdateAvatar = 0,
        UpdateUsername,
        AddNewFriendRequest,
        CreateGroupRequest,
        Register,
        UnLogin,
        HandleNewFriendRequest,
        HandleNewGroupJoinRequest,
        ApplyJoinGroupRequest,
        updateGroupNameRequest,
        updateGroupAvatarRequest,
        DeleteFriendRequest,
        ExitGroup,
        GetDeletedConversationInfo,
        DismissGroup,
        RemoveGroupMember
    };
    struct PendingRequest
    {
        QString requestID;
        OperationName name;
        QVariantMap params;
        QVariantMap tokenParams;
    };
    struct MessageRequest
    {
        ContentType type;
        QString content;
        QString conversationID;
    };
    QMap<QString, PendingRequest> map_idempotentCache;//QString是RequestID
    QMap<QString, MessageRequest> map_messageCache;//QString是tempMsgID

};

#endif // TCPLONGCONNECTION_H
