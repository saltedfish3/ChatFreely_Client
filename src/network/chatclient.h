#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QCoreApplication>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include "../utils/GlobalVariable.h"

#define SADDR "127.0.0.1"
#define SPORT 9000

class ChatClient : public QObject
{
    Q_OBJECT
public:
    static ChatClient& getChatClient();
    ChatClient(const ChatClient&) = delete;
    ChatClient& operator=(const ChatClient&) = delete;

    void sendLogin(QString email, QString password);
    void sendRegister(QString email, QString password, QString username);

signals:
    void LoginState(bool isSuccess ,QString from, QString info);
    void RegisterState(bool isSuccess ,QString from, QString info);
private:
    explicit ChatClient(QObject *parent = nullptr);
    ~ChatClient();
    void startConnect();
    void sendHello();
    void handleHelloResp(QJsonObject obj);
    void handleLoginResp(QJsonObject obj);
    void handleRegisterResp(QJsonObject obj);

    QTcpSocket* socket;
    QTimer* clock_retry;
    QTimer* clock_heartbeat;
};

#endif // CHATCLIENT_H
