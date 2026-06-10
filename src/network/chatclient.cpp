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
    QString requestsID = QString::number(getRequestsId());
    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "Login";
    obj["Email"] = email;
    obj["Password"] = password;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + '\n';
    this->socket->write(data);
    this->socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit LoginState(false, "", "登录超时，请稍后再试");
        }
    });
}

void ChatClient::sendRegister(QString username, QString email, QString password)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit RegisterState(false, "", "无法连接服务器，请稍后再试");
        return;
    }
    QString requestsID = QString::number(getRequestsId());
    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "Register";
    obj["Username"] = username;
    obj["Email"] = email;
    obj["Password"] = password;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + '\n';
    this->socket->write(data);
    this->socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit LoginState(false, "", "注册超时，请稍后再试");
        }
    });
}

void ChatClient::sendHello()
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        QTimer::singleShot(100,[this](){
            sendHello();
        });
        return;
    }
    QString requestsID = QString::number(getRequestsId());
    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "Hello";

    QJsonDocument doc(obj);
    QByteArray data =doc.toJson(QJsonDocument::Compact) + "\n";
    socket->write(data);
    socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            sendHello();
        }
    });
}

void ChatClient::handleHelloResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    if(obj.contains("HeartbeatNum"))
    {
        int interval = obj.value("HeartbeatNum").toInt();
        if(interval == 0)
            return;
        this->clock_heartbeat->setInterval(interval*1000);
        this->clock_heartbeat->start();
        this->waiting_requestsID.erase(requestsID.toStdString());
    }
}

void ChatClient::handleLoginResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    if(obj.contains("Result") && obj.contains("Info"))
    {
        bool result = obj.value("Result").toBool();
        if(result)
        {
            emit LoginState(result,"",obj.value("Info").toString());
            if(obj.contains("Username") && obj.contains("SID"))
            {
                emit UserData(obj.value("Username").toString(), obj.value("SID").toInteger());
                //若获取失败重新申请
            }
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
        this->waiting_requestsID.erase(requestsID.toStdString());
    }
}

void ChatClient::handleRegisterResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
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
    this->waiting_requestsID.erase(requestsID.toStdString());
}

uint64_t ChatClient::getRequestsId()
{
    static std::atomic<uint64_t> requestsid{0};
    return requestsid++;
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
        this->clock_retry->stop();
        sendHello();
    });

    connect(this->socket, &QTcpSocket::disconnected, this, [this](){
        this->clock_heartbeat->stop();
        if(!this->clock_retry->isActive())
            this->clock_retry->start();
    });

    //连接失败或发生错误处理
    connect(this->socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error){
        this->clock_heartbeat->stop();
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
                if(!obj.contains("Requests_id"))
                    return;
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
        startConnect();
    });

    connect(this->clock_heartbeat, &QTimer::timeout, this, [this](){
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
    {
        this->socket->abort();
        this->socket->connectToHost(SADDR,SPORT);
    }
}
