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

#define SADDR "127.0.0.1"
#define SPORT 9000

class UserInfo;
class HttpShortConnection;

class TcpLongConnection : public QObject
{
    Q_OBJECT
public:
    static TcpLongConnection& getTcpClient();
    TcpLongConnection(const TcpLongConnection&) = delete;
    TcpLongConnection& operator=(const TcpLongConnection&) = delete;

    void sendLogin(QString email, QString password);
    void sendRegister(QString email, QString password, QString username);
    void sendUpadteAvatar(QString url);
    void sendUpdateUsername(QString username);
    void sendAddNewFriendRequest(QString sid = "", QString email = "", QString verMsg = "");
    void sendHandleNewFriendRequest(QString handle_uid, bool isAgree);
    void sendMessageTo(QString uid, QString message, QString tempMsgID);
    void sendUnLogin();
    void sendRefreshToken(std::function<void(bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired)> callback);
    void sendAccessTokenLogin();

    void getNewFriendRequestsList();
    void getFriendList();
    void getConversationsFriendUIDListAndSeq(int64_t syncConvSeq);
    void sendSyncNewMessages(const QString& friendUID, qint64 startConvSeq, int limit = 30);

    bool isConnect();

signals:
    void reconnectSuccess();

    void LoginState(bool isSuccess, const QString& from, const QString& info, bool reLogin = false);
    void accessTokenLoginSuccess();
    void RegisterState(bool isSuccess ,QString from, QString info);
    void mainState(bool isSuccess, QString info);
    void newFriendRequestsState(bool isEmpty);
    void newFriendRequestsHandleResult(bool isSuccess);
    void newFriendRequests(QString uid, QString sid, QString username, QString avatar_url, QString verMsg);

    //全量获取好友列表
    void allFriendList(const QList<QVariantMap>& list);
    //接收服务器的push
    void newFriend(QString uid, QString sid, QString username, QString avatar_url, QString email, bool isOnline);

    //用于通知业务层getConversationsListAndSeq是否成功
    void newSyncConvStatus(bool isSuccess, const QList<QString>& conversations, int64_t newSyncConvSeq);
    void syncMessagesStatus(bool isSuccess, const QString& conversationID, const QList<Message>& messages);

    //更新好友的状态信息
    void FriendStatus(QString uid, bool isOnline);
    void FriendUsername(QString uid, QString username);
    void FriendAvatar(QString uid, QString avatar);

    void sendMessageStatus(bool isSuccess, QString tempMsgID, QString receiverUID, QString messageID = "", int64_t timeStamp = 0, int64_t convSeq = 0);
    void pushMessage(QString senderUID, QString content, QString messageID, int64_t timeStamp, int64_t convSeq);
    void cleanNewFriendRequestsList();
    // void cleanFriendList();
    void refreshExpiredExit();
    void exitAccount();

private:
    explicit TcpLongConnection(QObject *parent = nullptr);

    void startConnect();
    void sendHello();
    void handleHelloResp(QJsonObject obj);
    void handleLoginResp(QJsonObject obj);
    void handleAccessTokenLoginResp(QJsonObject obj);
    void handleRegisterResp(QJsonObject obj);
    void handleUpdateAvatarResp(QJsonObject obj);
    void handleUpdateUsernameResp(QJsonObject obj);
    void handleAddNewFriendRequestResp(QJsonObject obj);
    void handleHandleNewFriendRequestResp(QJsonObject obj);
    void handleGetNewFriendRequestsListResp(QJsonObject obj);
    void handleGetFriendListResp(QJsonObject obj);

    void handleGetConversationsFriendUIDListAndSeqResp(QJsonObject obj);
    void handleSyncNewMessagesResp(QJsonObject obj);

    void handlePushNewFriendRequests(QJsonObject obj);
    void handlePushNewFriend(QJsonObject obj);
    void handlePushFriendStatus(QJsonObject obj);
    void handlePushNewMessage(QJsonObject obj);
    void handlePushFriendUsername(QJsonObject obj);
    void handlePushFriendAvatar(QJsonObject obj);
    void handleSendMessageResp(QJsonObject obj);
    void handleUnLoginResp(QJsonObject obj);

    void handleRefreshTokenResp(QJsonObject obj);

    uint64_t getRequestsId();

    QTcpSocket* socket;
    QTimer* clock_retry;
    QTimer* clock_heartbeat;

    std::unordered_set<std::string> waiting_requestsID;
    QHash<QString, QTimer*> hash_timeoutTimers;
    QHash<QString, QString> hash_requestsToFriendUID;
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
        Register,
        UnLogin,
        HandleNewFriendRequest
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
        QString requestID;
        QString content;
        QString receiverUID;
        uint64_t reqCount = 0;
    };
    QMap<QString, PendingRequest> map_idempotentCache;//QString是RequestID
    QMap<QString, MessageRequest> map_messageCache;//QString是tempMsgID

};

#endif // TCPLONGCONNECTION_H
