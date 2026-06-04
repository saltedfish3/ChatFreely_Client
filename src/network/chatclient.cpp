#include "chatclient.h"

ChatClient &ChatClient::getChatClient()
{
    static ChatClient client;
    return client;
}

void ChatClient::sendLogin(QString email, QString password)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit LoginState(false, "", "无法连接服务器，请稍后再试");
        return;
    }
    QJsonObject obj;
    obj["Type"] = "Login";
    obj["Email"] = email;
    obj["Password"] = password;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + '\n';
    this->socket->write(data);
    this->socket->flush();
    qDebug()<<"发送登录请求";
}

void ChatClient::sendRegister(QString username, QString email, QString password)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit RegisterState(false, "", "无法连接服务器，请稍后再试");
        return;
    }
    QJsonObject obj;
    obj["Type"] = "Register";
    obj["Username"] = username;
    obj["Email"] = email;
    obj["Password"] = password;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + '\n';
    this->socket->write(data);
    this->socket->flush();
    qDebug()<<"发送注册请求";
}

void ChatClient::sendHello()
{
    QJsonObject obj;
    obj["Type"] = "Hello";

    QJsonDocument doc(obj);
    QByteArray data =doc.toJson(QJsonDocument::Compact) + "\n";
    socket->write(data);
    socket->flush();
}

void ChatClient::handleHelloResp(QJsonObject obj)
{
    if(obj.contains("HeartbeatNum"))
    {
        int interval = obj.value("HeartbeatNum").toInt();
        if(interval == 0)
            return;
        this->clock_heartbeat->setInterval(interval*1000);
        this->clock_heartbeat->start();
    }
}

void ChatClient::handleLoginResp(QJsonObject obj)
{
    if(obj.contains("Result") && obj.contains("Info"))
    {
        bool result = obj.value("Result").toBool();
        if(result)
        {
            emit LoginState(result,"",obj.value("Info").toString());
        }
        else
        {
            if(obj.contains("From"))
            {
                emit LoginState(result, obj.value("From").toString(),obj.value("Info").toString());
            }
            else
            {
                emit LoginState(result,"",obj.value("Info").toString());
            }
        }
    }
}

void ChatClient::handleRegisterResp(QJsonObject obj)
{
    bool result = obj.value("Result").toBool();
    if(result)
    {
        emit RegisterState(result,"",obj.value("Info").toString());
    }
    else
    {
        if(obj.contains("From"))
        {
            emit RegisterState(result, obj.value("From").toString(),obj.value("Info").toString());
        }
        else
        {
            emit RegisterState(result,"",obj.value("Info").toString());
        }
    }
}

ChatClient::ChatClient(QObject *parent)
    : QObject{parent}
{
    this->socket = new QTcpSocket(this);
    this->clock_retry = new QTimer(this);
    this->clock_heartbeat = new QTimer(this);
    this->clock_retry->setInterval(5000);


    //连接成功处理
    connect(this->socket, &QTcpSocket::connected, this, [this](){
        qDebug() << "connect Success";
        this->clock_retry->stop();
        sendHello();
    });

    //连接失败或发生错误处理
    connect(this->socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error){
        qDebug()<<this->socket->errorString();
        if(!this->clock_retry->isActive())
            this->clock_retry->start();
    });

    connect(this->socket, &QTcpSocket::readyRead, this, [this](){
        while(this->socket->canReadLine())
        {
            QByteArray data = this->socket->readLine().trimmed();
            QJsonDocument qjd = QJsonDocument::fromJson(data);
            if(!qjd.isNull() && qjd.isObject())
            {
                QJsonObject obj = qjd.object();
                if(obj.contains("Type"))
                {
                    QString type = obj.value("Type").toString();
                    if(type == "HelloResp")
                    {
                        this->handleHelloResp(obj);
                    }
                    else if(type == "HeartbeatResp")
                    {
                        this->clock_heartbeat->start();
                    }
                    else if(type == "LoginResp")
                    {
                        this->handleLoginResp(obj);
                    }
                    else if(type == "RegisterResp")
                    {
                        this->handleRegisterResp(obj);
                    }
                }
            }
        }
    });

    connect(this->clock_retry, &QTimer::timeout, this, [this](){
        qDebug()<<"try connect";
        startConnect();
    });

    connect(this->clock_heartbeat, &QTimer::timeout, this, [this](){
        qDebug()<<"Heartbeat Send";
        QJsonObject obj;
        obj["Type"] = "Heartbeat";
        QJsonDocument doc(obj);
        QByteArray data = doc.toJson(QJsonDocument::Compact) + '\n';
        this->socket->write(data);
        this->socket->flush();
        QDateTime now = QDateTime::currentDateTime();
        qDebug() << "当前时间:" << now.toString("yyyy-MM-dd hh:mm:ss.zzz");
    });

    connect(qApp, &QCoreApplication::aboutToQuit, this, [this]() {
        if (this->socket && this->socket->state() == QAbstractSocket::ConnectedState) {
            this->socket->disconnectFromHost();

            if (this->socket->state() != QAbstractSocket::UnconnectedState) {
                this->socket->waitForDisconnected(500);
            }
        }
    });
    startConnect();
}

ChatClient::~ChatClient()
{
}

void ChatClient::startConnect()
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
        this->socket->connectToHost(SADDR,SPORT);
}
