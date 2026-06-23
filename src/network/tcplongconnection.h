#ifndef TCPLONGCONNECTION_H
#define TCPLONGCONNECTION_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QCoreApplication>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
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
    void sendUnLogin();

    bool isConnect();

signals:
    void LoginState(bool isSuccess ,QString from, QString info);
    void RegisterState(bool isSuccess ,QString from, QString info);
    void mainState(bool isSuccess, QString info);
    void exitAccount();

private:
    explicit TcpLongConnection(QObject *parent = nullptr);
    ~TcpLongConnection();
    void startConnect();
    void sendHello();
    void handleHelloResp(QJsonObject obj);
    void handleLoginResp(QJsonObject obj);
    void handleRegisterResp(QJsonObject obj);
    void handleUpdateAvatarResp(QJsonObject obj);
    void handleUpdateUsernameResp(QJsonObject obj);
    void handleUnLoginResp(QJsonObject obj);
    uint64_t getRequestsId();

    QTcpSocket* socket;
    QTimer* clock_retry;
    QTimer* clock_heartbeat;

    std::unordered_set<std::string> waiting_requestsID;
};

#endif // TCPLONGCONNECTION_H
