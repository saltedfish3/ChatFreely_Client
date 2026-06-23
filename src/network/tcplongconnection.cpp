#include "tcplongconnection.h"
#include "httpshortconnection.h"
#include "../utils/userinfo.h"

TcpLongConnection &TcpLongConnection::getTcpClient()
{
    static TcpLongConnection client;
    return client;
}

void TcpLongConnection::sendLogin(QString email, QString password)
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

    UserInfo::getUserInfo().setEmail(email);

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit LoginState(false, "", "登录超时，请稍后再试");
        }
    });
}

void TcpLongConnection::sendRegister(QString username, QString email, QString password)
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

void TcpLongConnection::sendUpadteAvatar(QString url)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        UserInfo::getUserInfo().rollBackAvatar();
        return;
    }
    QString requestsID = QString::number(getRequestsId());
    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "Update_Avatar";
    obj["Url"] = url;

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    this->socket->write(data);
    this->socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit mainState(false, "连接超时，请稍后再试");
            UserInfo::getUserInfo().rollBackAvatar();
        }
    });
}

void TcpLongConnection::sendUpdateUsername(QString username)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        return;
    }
    if(username.isEmpty())
    {
        emit mainState(false, "更新失败, 用户名不能为空");
        return;
    }

    QJsonObject obj;
    QString requestsID = QString::number(getRequestsId());
    obj["Requests_id"] = requestsID;
    obj["Type"] = "UpdateUsername";
    obj["Username"] = username;

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    this->socket->write(data);
    this->socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit mainState(false, "连接超时，请稍后再试");
        }
    });
}

void TcpLongConnection::sendUnLogin()
{
    //发送退出登录请求json
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        return;
    }
    QJsonObject obj;
    QString requestsID = QString::number(getRequestsId());
    obj["Requests_id"] = requestsID;
    obj["Type"] = "UnLogin";

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";

    this->socket->write(data);
    this->socket->flush();
    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit mainState(false, "连接超时，请稍后再试");
        }
    });
}

bool TcpLongConnection::isConnect()
{
    return this->socket->state() == QAbstractSocket::ConnectedState;
}

void TcpLongConnection::sendHello()
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
    this->socket->write(data);
    this->socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            sendHello();
        }
    });
}

void TcpLongConnection::handleHelloResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(obj.contains("HeartbeatNum"))
    {
        int interval = obj.value("HeartbeatNum").toInt();
        if(interval == 0)
            return;
        this->clock_heartbeat->setInterval(interval*1000);
        this->clock_heartbeat->start();
    }
}

void TcpLongConnection::handleLoginResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(obj.contains("Result") && obj.contains("Info"))
    {
        bool result = obj.value("Result").toBool();
        if(result)
        {
            emit LoginState(result,"",obj.value("Info").toString());
            if(obj.contains("Username") && obj.contains("SID"))
            {
                UserInfo::getUserInfo().setUsername(obj.value("Username").toString());
                UserInfo::getUserInfo().setSID(obj.value("SID").toString());
                UserInfo::getUserInfo().setUID(obj.value("UID").toString());
                if(obj.value("Avatar_Url").toString().isEmpty())
                    UserInfo::getUserInfo().setAvatar(QPixmap(":/default/images/defaultAvatar.png"));
                else
                {
                    HttpShortConnection::getHttpClient().getAvatar(obj.value("Avatar_Url").toString(), 3);
                }

                UserInfo::getUserInfo().sendUpdateSignal();
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
    }
}

void TcpLongConnection::handleRegisterResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
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

void TcpLongConnection::handleUpdateAvatarResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    bool result = obj.value("Result").toBool();
    if(!result)
    {
        emit mainState(false, "上传失败，请稍后再试");
        UserInfo::getUserInfo().rollBackAvatar();
    }
    else
    {
        UserInfo::getUserInfo().confirmAvatar();
    }
}

void TcpLongConnection::handleUnLoginResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    //发送退出账号信号
    emit exitAccount();
}

void TcpLongConnection::handleUpdateUsernameResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        emit mainState(false, "更新失败，请稍后重试");
        return;
    }
    bool success = obj.value("Result").toBool();
    if(success)
    {
        emit mainState(true, "更新成功");
        UserInfo::getUserInfo().confirmUsername();
    }
    else
        emit mainState(false, "更新失败");
}

uint64_t TcpLongConnection::getRequestsId()
{
    static std::atomic<uint64_t> requestsid{0};
    return requestsid++;
}

TcpLongConnection::TcpLongConnection(QObject *parent)
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
                    else if(type == "UpdateAvatarResp")
                    {
                        this->handleUpdateAvatarResp(obj);
                    }
                    else if(type == "UnLoginResp")
                    {
                        this->handleUnLoginResp(obj);
                    }
                    else if(type == "UpdateUsernameResp")
                    {
                        this->handleUpdateUsernameResp(obj);
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
        obj["Requests_id"] = QString::number(getRequestsId());
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

TcpLongConnection::~TcpLongConnection()
{
}

void TcpLongConnection::startConnect()
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        this->socket->abort();
        this->socket->connectToHost(SADDR,SPORT);
    }
}
