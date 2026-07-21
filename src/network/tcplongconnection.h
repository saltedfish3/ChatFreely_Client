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
#include <atomic>
#include <unordered_set>
#include "../utils/GlobalVariable.h"

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
    void sendUnLogin();

    void sendRefreshToken(std::function<void(bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired)> callback);
    void sendAccessTokenLogin();

    void getNewFriendRequestsList();
    void getFriendList();

    bool isConnect();

signals:
    void LoginState(bool isSuccess ,QString from, QString info, bool reLogin = false);
    void RegisterState(bool isSuccess ,QString from, QString info);
    void mainState(bool isSuccess, QString info);
    void newFriendRequestsState(bool isEmpty);
    void newFriendRequestsHandleResult(bool isSuccess);
    void newFriendRequests(QString uid, QString sid, QString username, QString avatar_url, QString verMsg);
    void newFriend(QString uid, QString sid, QString username, QString avatar_url, QString email, bool isOnline);
    void newFriendState(bool isEmpty);
    void FriendStatus(QString uid, bool isOnline);
    void cleanNewFriendRequestsList();
    void cleanFriendList();
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
    void handlePushNewFriendRequests(QJsonObject obj);
    void handlePushNewFriend(QJsonObject obj);
    void handlePushFriendStatus(QJsonObject obj);
    void handleUnLoginResp(QJsonObject obj);

    void handleRefreshTokenResp(QJsonObject obj);

    uint64_t getRequestsId();

    QTcpSocket* socket;
    QTimer* clock_retry;
    QTimer* clock_heartbeat;

    std::unordered_set<std::string> waiting_requestsID;
    std::map<QString, std::function<void(bool, const QString&, bool)>> refreshCallBack;

    bool isTryToUnLoginRefresh = false;
    bool isTryToLoginAgain = false;
    bool isTryToAccessTokenLogin = false;

    QString avatarUrl_updateAvatarWaitingRefresh;
    QString username_updateUsernameWaitingRefresh;
    QString sid_addNewFriendWaitingRefresh;
    QString email_addNewFriendWaitingRefresh;
    QString verMsg_addNewFriendWaitingRefresh;
    QString uid_handleAddNewFriendWaitingRefresh;
    std::optional<bool> isAgree_handleAddNewFriendWaitingRefresh;

};

#endif // TCPLONGCONNECTION_H
