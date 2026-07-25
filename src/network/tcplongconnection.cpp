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

    QVariantMap currentParams;
    QString requestsID;
    currentParams["Username"] = username;
    currentParams["email"] = email;
    currentParams["password"] = password;
    for(auto it = this->map_idempotentCache.begin(); it != this->map_idempotentCache.end(); it++)
    {
        if(it.value().name == Register && it.value().params == currentParams)
        {
            requestsID = it.key();
            break;
        }
    }

    if(requestsID.isEmpty())
    {
        requestsID = QString::number(getRequestsId());
        PendingRequest prt;
        prt.requestID = requestsID;
        prt.name = Register;
        prt.params = currentParams;

        this->map_idempotentCache[requestsID] = prt;
    }

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
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();
    obj["Url"] = url;

    QVariantMap tokenParams;
    tokenParams["url"] = url;

    PendingRequest prt;
    prt.requestID = requestsID;
    prt.name = UpdateAvatar;
    prt.tokenParams = tokenParams;
    this->map_idempotentCache[requestsID] = prt;

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
            this->map_idempotentCache.remove(requestsID);
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

    //幂等操作
    QVariantMap currentParams;
    QString requestsID;
    currentParams["Username"] = username;
    for(auto it = this->map_idempotentCache.begin(); it != this->map_idempotentCache.end(); it++)
    {
        if(it.value().name == UpdateUsername && it.value().params == currentParams)
        {
            requestsID = it.key();
            break;
        }
    }

    //是否复用幂等
    if(requestsID.isEmpty())
    {
        requestsID = QString::number(getRequestsId());
        PendingRequest prt;
        prt.requestID = requestsID;
        prt.name = UpdateUsername;
        prt.params = currentParams;

        QVariantMap tokenParams;
        tokenParams["Username"] = username;
        prt.tokenParams = tokenParams;

        this->map_idempotentCache[requestsID] = prt;
    }

    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "UpdateUsername";
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();
    obj["Username"] = username;
    // this->username_updateUsernameWaitingRefresh = username;

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

    QString requestsID;
    for(auto it = this->map_idempotentCache.begin(); it != this->map_idempotentCache.end(); it++)
    {
        if(it.value().name == UnLogin)
        {
            requestsID = it.key();
            break;
        }
    }

    //是否复用幂等
    if(requestsID.isEmpty())
    {
        requestsID = QString::number(getRequestsId());
        PendingRequest prt;
        prt.requestID = requestsID;
        prt.name = UnLogin;

        this->map_idempotentCache[requestsID] = prt;
    }

    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "UnLogin";
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();

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

void TcpLongConnection::sendRefreshToken(std::function<void(bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired)> callback)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        if(callback)
            callback(false, "", false);
        return;
    }

    QString refreshToken = UserInfo::getUserInfo().getRefreshToken();
    if(refreshToken.isEmpty())
    {
        if(callback)
            callback(false, "", false);
        return;
    }

    QString requestsID = QString::number(getRequestsId());
    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "RefreshToken";
    obj["RefreshToken"] = refreshToken;

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + '\n';
    socket->write(data);
    socket->flush();

    waiting_requestsID.insert(requestsID.toStdString());
    this->refreshCallBack[requestsID] = callback;

    QTimer::singleShot(5000, [this, requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            auto it = this->refreshCallBack.find(requestsID);
            if(it != this->refreshCallBack.end())
            {
                if(it->second)
                    it->second(false, "", false);
                this->refreshCallBack.erase(it);
            }
        }
    });
}

void TcpLongConnection::sendAccessTokenLogin()
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        return;
    }
    if(UserInfo::getUserInfo().isLogin())
    {
        if(this->isTryToAccessTokenLogin)
            return;
        this->isTryToAccessTokenLogin = true;
        QString accessToken = UserInfo::getUserInfo().getAccessToken();
        if(accessToken.isEmpty())
        {
            emit refreshExpiredExit();
            return;
        }
        QJsonObject obj;
        QString requestsID = QString::number(getRequestsId());
        obj["Requests_id"] = requestsID;
        obj["Type"] = "AccessTokenLogin";
        obj["AccessToken"] = accessToken;

        QJsonDocument doc(obj);
        QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";

        this->socket->write(data);
        this->socket->flush();
        this->waiting_requestsID.insert(requestsID.toStdString());
        QTimer::singleShot(10000,[this,requestsID](){
            if(this->waiting_requestsID.erase(requestsID.toStdString()))
            {
                emit refreshExpiredExit();
            }
        });
    }
}

void TcpLongConnection::getNewFriendRequestsList()
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        return;
    }
    QJsonObject obj;
    QString requestsID = QString::number(getRequestsId());
    obj["Requests_id"] = requestsID;
    obj["Type"] = "GetNewFriendRequestsList";
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";

    this->socket->write(data);
    this->socket->flush();
    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(7000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit mainState(false, "连接超时，请稍后再试");
        }
    });
}

void TcpLongConnection::getFriendList()
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        return;
    }
    QJsonObject obj;
    QString requestsID = QString::number(getRequestsId());
    obj["Requests_id"] = requestsID;
    obj["Type"] = "GetFriendList";
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";

    this->socket->write(data);
    this->socket->flush();
    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(7000,[this,requestsID](){
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
            if(this->socket->state() == QAbstractSocket::ConnectedState)
                sendHello();
        }
    });
}

void TcpLongConnection::sendAddNewFriendRequest(QString sid, QString email, QString verMsg)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        return;
    }
    if(sid.isEmpty() && email.isEmpty())
    {
        emit mainState(false, "申请添加好友失败");
        return;
    }

    QVariantMap currentParams;
    currentParams["sid"] = sid;
    currentParams["email"] = email;
    currentParams["verMsg"] = verMsg;

    QString requestsID;
    for(auto it = this->map_idempotentCache.begin(); it != this->map_idempotentCache.end(); it++)
    {
        if(it.value().name == AddNewFriendRequest && it.value().params == currentParams)
        {
            requestsID = it.key();
            break;
        }
    }

    if(requestsID.isEmpty())
    {
        requestsID = QString::number(getRequestsId());
        PendingRequest prt;
        prt.requestID = requestsID;
        prt.name = AddNewFriendRequest;
        prt.params = currentParams;

        QVariantMap tokenParams;
        tokenParams["sid"] = sid;
        tokenParams["email"] = email;
        tokenParams["verMsg"] = verMsg;
        prt.tokenParams = tokenParams;

        this->map_idempotentCache[requestsID] = prt;
    }

    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "AddNewFriendRequest";
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();

    if(sid.isEmpty())
        obj["Receiver_Email"] = email;
    else
        obj["Receiver_SID"] = sid;

    if(!verMsg.isEmpty())
        obj["VerMsg"] = verMsg;

    QJsonDocument doc(obj);
    QByteArray data =doc.toJson(QJsonDocument::Compact) + "\n";
    this->socket->write(data);
    this->socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
            emit mainState(false, "申请添加好友超时，请稍后再试");
    });
}

void TcpLongConnection::sendHandleNewFriendRequest(QString handle_uid, bool isAgree)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        return;
    }
    if(handle_uid.isEmpty())
    {
        emit mainState(false, "处理申请错误，请稍后再试");
        return;
    }
    //显示loading
    emit newFriendRequestsState(false);

    QVariantMap currentParams;
    QString requestsID;
    currentParams["handle_uid"] = handle_uid;
    currentParams["isAgree"] = isAgree;
    for(auto it = this->map_idempotentCache.begin(); it != this->map_idempotentCache.end(); it++)
    {
        if(it.value().name == HandleNewFriendRequest && it.value().params == currentParams)
        {
            requestsID = it.key();
            break;
        }
    }

    //是否复用幂等
    if(requestsID.isEmpty())
    {
        requestsID = QString::number(getRequestsId());
        PendingRequest prt;
        prt.requestID = requestsID;
        prt.name = HandleNewFriendRequest;
        prt.params = currentParams;

        QVariantMap tokenParams;
        tokenParams["handle_uid"] = handle_uid;
        tokenParams["isAgree"] = isAgree;
        prt.tokenParams = tokenParams;

        this->map_idempotentCache[requestsID] = prt;
    }

    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "HandleNewFriendRequest";
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();
    obj["Status"] = isAgree;
    obj["HandleUID"] = handle_uid;

    QJsonDocument doc(obj);
    QByteArray data =doc.toJson(QJsonDocument::Compact) + "\n";
    this->socket->write(data);
    this->socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit newFriendRequestsHandleResult(false);
            emit mainState(false, "处理好友申请超时，请稍后再试");
        }
    });
}

void TcpLongConnection::sendMessageTo(QString uid, QString message, QString tempMsgID)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        return;
    }
    if(message.trimmed().isEmpty())
    {
        emit mainState(false, "发送内容不能为空");
        return;
    }
    QJsonObject obj;
    QString requestsID = QString::number(getRequestsId());
    obj["Requests_id"] = requestsID;
    obj["Type"] = "SendMessage";
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();
    obj["ReceiverUID"] = uid;
    obj["Content"] = message;
    obj["TempMsgID"] = tempMsgID;

    QJsonDocument doc(obj);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";

    this->socket->write(data);
    this->socket->flush();
    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(10000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit mainState(false, "连接超时，请稍后再试");
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
    if(UserInfo::getUserInfo().isLogin())
    {
        sendAccessTokenLogin();
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
            if(obj.contains("Username") && obj.contains("SID") && obj.contains("UID") && obj.contains("Avatar_Url"))
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
                if((!obj.contains("RefreshToken") || !obj.value("RefreshToken").isString() || obj.value("RefreshToken").toString().isEmpty()) &&
                    (!obj.contains("AccessToken") || !obj.value("AccessToken").isString() || obj.value("AccessToken").toString().isEmpty()))
                {
                    if(!this->isTryToLoginAgain)
                    {
                        this->isTryToLoginAgain = true;
                        emit LoginState(false,"","", true);
                    }
                    else
                    {
                        this->isTryToLoginAgain = false;
                        emit LoginState(false, "", "登录凭证获取失败，请重新登录");
                    }
                    return;
                }
                else if(!obj.contains("AccessToken") || !obj.value("AccessToken").isString() || obj.value("AccessToken").toString().isEmpty())
                {
                    QString refreshToken = obj.value("RefreshToken").toString();
                    if(refreshToken.isEmpty())
                    {
                        if(!this->isTryToLoginAgain)
                        {
                            this->isTryToLoginAgain = true;
                            emit LoginState(false, "", "", true);
                        }
                        else
                        {
                            this->isTryToLoginAgain = false;
                            emit LoginState(false, "", "登录凭证获取失败，请重新登录");
                        }
                        return;
                    }
                    UserInfo::getUserInfo().setRefreshToken(obj.value("RefreshToken").toString());
                    //补发token   refresh一下token
                    QString info = obj.value("Info").toString();
                    sendRefreshToken([this, info](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                        if(!isSuccess || newAccessToken.isEmpty())
                        {
                            this->isTryToLoginAgain = false;
                            emit LoginState(isSuccess, "", "登录凭证获取失败，请重新登录");
                            return;
                        }
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        UserInfo::getUserInfo().setLogin(true);
                        getNewFriendRequestsList();
                        getFriendList();
                        UserInfo::getUserInfo().sendUpdateSignal();
                        this->isTryToLoginAgain = false;
                        emit LoginState(isSuccess, "", info);
                    });

                    return;
                }

                UserInfo::getUserInfo().setRefreshToken(obj.value("RefreshToken").toString());
                UserInfo::getUserInfo().setAccessToken(obj.value("AccessToken").toString());
                UserInfo::getUserInfo().setLogin(true);
                UserInfo::getUserInfo().sendUpdateSignal();
                this->isTryToLoginAgain = false;
                getNewFriendRequestsList();
                getFriendList();
                emit LoginState(result,"",obj.value("Info").toString());
            }
            else
            {
                //静默重新登录
                if(!this->isTryToLoginAgain)
                {
                    this->isTryToLoginAgain = true;
                    emit LoginState(false,"", "", true);
                }
                else
                {
                    this->isTryToLoginAgain = false;
                    emit LoginState(false, "", "登录失败，请重新登录");
                }
            }
        }
        else
        {
            if(obj.contains("From"))
            {
                emit LoginState(result, obj.value("From").toString(),obj.value("Info").toString());
                this->isTryToLoginAgain = false;
            }
            else
            {
                emit LoginState(result,"",obj.value("Info").toString());
                this->isTryToLoginAgain = false;
            }
        }
    }
}

void TcpLongConnection::handleAccessTokenLoginResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    this->isTryToAccessTokenLogin = false;

    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        sendRefreshToken([this](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired) {
            if(isSuccess && !newAccessToken.isEmpty())
            {
                UserInfo::getUserInfo().setAccessToken(newAccessToken);
                sendAccessTokenLogin();
            }else
            {
                if(isRefreshTokenExpired)
                    emit refreshExpiredExit();
            }
        });
        return;
    }
    bool result = obj.value("Result").toBool();
    if(!result)
    {
        if(obj.contains("AccessTokenExpired") && obj.value("AccessTokenExpired").isBool())
        {
            bool isExpired = obj.value("AccessTokenExpired").toBool();
            if(isExpired)
            {
                sendRefreshToken([this](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                    if(isSuccess)
                    {
                        if(newAccessToken.isEmpty())
                        {
                            emit refreshExpiredExit();
                            return;
                        }
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        sendAccessTokenLogin();
                    }
                    else
                    {
                        emit refreshExpiredExit();
                    }
                });
            }
            return;
        }
        emit refreshExpiredExit();
        return;
    }
    UserInfo::getUserInfo().setLogin(true);
    getNewFriendRequestsList();
    getFriendList();
}

void TcpLongConnection::handleRegisterResp(QJsonObject obj)
{
    if(!obj.contains("Result") || !obj.value("Result").toBool())
        return;

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
    this->map_idempotentCache.remove(requestsID);
}

void TcpLongConnection::handleUpdateAvatarResp(QJsonObject obj)
{
    if(!obj.contains("Result") || !obj.value("Result").isBool())
        return;

    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());

    bool result = obj.value("Result").toBool();
    if(!result)
    {
        if(obj.contains("AccessTokenExpired") && obj.value("AccessTokenExpired").isBool())
        {
            bool isExpired = obj.value("AccessTokenExpired").toBool();
            if(isExpired)
            {
                sendRefreshToken([this, requestsID](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                    if(isSuccess)
                    {
                        if(newAccessToken.isEmpty())
                        {
                            emit refreshExpiredExit();
                            this->map_idempotentCache.remove(requestsID);
                            return;
                        }
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);

                        auto it = this->map_idempotentCache.find(requestsID);
                        if(it != this->map_idempotentCache.end())
                            sendUpadteAvatar(it.value().tokenParams.value("url").toString());
                        this->map_idempotentCache.remove(requestsID);
                        return;
                    }
                    else
                    {
                        if(isRefreshTokenExpired)
                            emit refreshExpiredExit();
                        else
                        {
                            emit mainState(false, "上传失败，请稍后再试");
                            UserInfo::getUserInfo().rollBackAvatar();
                        }
                        this->map_idempotentCache.remove(requestsID);
                    }
                });
            }
            return;
        }
        emit mainState(false, "上传失败，请稍后再试");
        UserInfo::getUserInfo().rollBackAvatar();
    }
    else
    {
        UserInfo::getUserInfo().confirmAvatar();
    }
    this->map_idempotentCache.remove(requestsID);
}

void TcpLongConnection::handleUnLoginResp(QJsonObject obj)
{
    //判断unLogin的AccessToken是否过期
    if(!obj.contains("Result") || !obj.value("Result").isBool())
        return;

    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());

    bool success = obj.value("Result").toBool();
    if(!success)
    {
        if(!obj.contains("AccessTokenExpired") || !obj.value("AccessTokenExpired").isBool())
        {
            emit mainState(false, "注销失败，请重试");
            this->isTryToUnLoginRefresh = false;
            this->map_idempotentCache.remove(requestsID);
            return;
        }

        bool isExpired = obj.value("AccessTokenExpired").toBool();
        if(isExpired && !this->isTryToUnLoginRefresh)
        {
            this->isTryToUnLoginRefresh = true;
            sendRefreshToken([this, requestsID](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                if(isSuccess && !newAccessToken.isEmpty())
                {
                    UserInfo::getUserInfo().setAccessToken(newAccessToken);
                    sendUnLogin();
                }
                else
                {
                    UserInfo::getUserInfo().setLogin(false);
                    emit exitAccount();
                    UserInfo::getUserInfo().cleanALL();
                    this->isTryToUnLoginRefresh = false;
                }
                this->map_idempotentCache.remove(requestsID);
            });
            return;
        }

        emit mainState(false, "注销失败，请重试");
        this->map_idempotentCache.remove(requestsID);
        this->isTryToUnLoginRefresh = false;
        return;
    }
    this->map_idempotentCache.remove(requestsID);
    //发送退出账号信号
    UserInfo::getUserInfo().setLogin(false);
    emit exitAccount();
    this->isTryToUnLoginRefresh = false;
    UserInfo::getUserInfo().cleanALL();
}

void TcpLongConnection::handleRefreshTokenResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());

    auto it = refreshCallBack.find(requestsID);
    if(it == refreshCallBack.end())
        return;
    auto callback = it->second;
    this->refreshCallBack.erase(it);

    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        if(callback)
            callback(false, "", false);
        return;
    }
    bool success = obj.value("Result").toBool();
    if(success)
    {
        if(!obj.contains("AccessToken") || !obj.value("AccessToken").isString())
        {
            if(callback)
                callback(false, "", false);
            return;
        }
        QString accessToken = obj.value("AccessToken").toString();
        if(accessToken.isEmpty())
        {
            if(callback)
                callback(false, "", false);
            return;
        }
        UserInfo::getUserInfo().setAccessToken(accessToken);
        if(callback)
            callback(true, accessToken, false);
    }
    else
    {
        bool isExpired;
        if(obj.contains("RefreshTokenExpired") && obj.value("RefreshTokenExpired").isBool())
        {
            isExpired = obj.value("RefreshTokenExpired").toBool();
        }
        if(callback)
            callback(false, "", isExpired);
    }
}

void TcpLongConnection::handleUpdateUsernameResp(QJsonObject obj)
{
    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        //静默超时 等待用户重试命中缓存
        return;
    }
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());

    bool success = obj.value("Result").toBool();
    if(success)
    {
        emit mainState(true, "更新成功");
        UserInfo::getUserInfo().confirmUsername();
    }
    else
    {
        if(obj.contains("AccessTokenExpired") && obj.value("AccessTokenExpired").isBool())
        {
            bool isExpired = obj.value("AccessTokenExpired").toBool();
            if(isExpired)
            {
                sendRefreshToken([this, requestsID](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                    if(isSuccess)
                    {
                        if(newAccessToken.isEmpty())
                        {
                            emit refreshExpiredExit();
                            this->map_idempotentCache.remove(requestsID);
                            return;
                        }
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        auto it = this->map_idempotentCache.find(requestsID);
                        if(it != this->map_idempotentCache.end())
                            sendUpdateUsername(it.value().tokenParams.value("Username").toString());
                        return;
                    }
                    else
                    {
                        if(isRefreshTokenExpired)
                        {
                            emit refreshExpiredExit();
                            this->map_idempotentCache.remove(requestsID);
                            return;
                        }
                        this->map_idempotentCache.remove(requestsID);
                        emit mainState(false, "更新失败");
                    }
                });
                return;
            }
        }
        emit mainState(false, "更新失败");
    }
    this->map_idempotentCache.remove(requestsID);
}

void TcpLongConnection::handleAddNewFriendRequestResp(QJsonObject obj)
{
    if(!obj.contains("Result") || !obj.value("Result").isBool())
        return;

    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());

    bool success = obj.value("Result").toBool();
    if(success)
        emit mainState(true, "成功发送添加好友申请");
    else
    {
        if(obj.contains("AccessTokenExpired") && obj.value("AccessTokenExpired").isBool())
        {
            bool isExpired = obj.value("AccessTokenExpired").toBool();
            if(isExpired)
            {
                sendRefreshToken([this, requestsID](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                    if(isSuccess)
                    {
                        if(newAccessToken.isEmpty())
                        {
                            emit refreshExpiredExit();
                            this->map_idempotentCache.remove(requestsID);
                            return;
                        }
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        auto it = this->map_idempotentCache.find(requestsID);
                        if(it != this->map_idempotentCache.end())
                        {
                            const QVariantMap& tokenParams = it.value().tokenParams;
                            sendAddNewFriendRequest(tokenParams.value("sid").toString(),
                                                    tokenParams.value("email").toString(),
                                                    tokenParams.value("verMsg").toString());
                        }
                        return;
                    }
                    else
                    {
                        if(isRefreshTokenExpired)
                        {
                            emit refreshExpiredExit();
                            this->map_idempotentCache.remove(requestsID);
                            return;
                        }
                        emit mainState(false, "申请添加好友失败，请稍后重试");
                    }
                });
                return;
            }
            emit mainState(false, "申请添加好友失败，请稍后重试");
            return;
        }
        if(!obj.contains("Info") || !obj.value("Info").isString())
        {
            emit mainState(false, "申请添加好友失败，请稍后重试");
            this->map_idempotentCache.remove(requestsID);
            return;
        }
        emit mainState(false, obj.value("Info").toString());
    }
    this->map_idempotentCache.remove(requestsID);
}

void TcpLongConnection::handleHandleNewFriendRequestResp(QJsonObject obj)
{
    if(!obj.contains("Result") || !obj.value("Result").isBool())
        return;

    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    bool success = obj.value("Result").toBool();
    if(success)
    {
        emit newFriendRequestsHandleResult(true);
        emit mainState(true, "处理成功");
        this->map_idempotentCache.remove(requestsID);
        return;
    }
    else
    {
        if(obj.contains("AccessTokenExpired") && obj.value("AccessTokenExpired").isBool())
        {
            bool isExpired = obj.value("AccessTokenExpired").toBool();
            if(isExpired)
            {
                sendRefreshToken([this, requestsID](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                    if(isSuccess)
                    {
                        if(newAccessToken.isEmpty())
                        {
                            emit refreshExpiredExit();
                            this->map_idempotentCache.remove(requestsID);
                            return;
                        }
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        //静默重试 或者 让用户重新操作
                        auto it = this->map_idempotentCache.find(requestsID);
                        if(it != this->map_idempotentCache.end())
                            sendHandleNewFriendRequest(it.value().tokenParams.value("handle_uid").toString(),
                                                       it.value().tokenParams.value("isAgree").toBool());

                        return;
                    }
                    else
                    {
                        if(isRefreshTokenExpired)
                        {
                            emit refreshExpiredExit();
                            this->map_idempotentCache.remove(requestsID);
                            return;
                        }
                        emit newFriendRequestsHandleResult(false);
                        emit mainState(false, "处理好友申请失败，请稍后再试");
                        this->map_idempotentCache.remove(requestsID);
                    }
                });
                return;
            }
            emit newFriendRequestsHandleResult(false);
            emit mainState(false, "处理好友申请失败，请稍后再试");
            this->map_idempotentCache.remove(requestsID);
            return;
        }
        emit newFriendRequestsHandleResult(false);
        emit mainState(false, "处理好友申请失败，请稍后再试");
        this->map_idempotentCache.remove(requestsID);
    }
}

void TcpLongConnection::handleGetNewFriendRequestsListResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        emit newFriendRequestsState(true);
        return;
    }
    bool success = obj.value("Result").toBool();
    if(success)
    {
        if(!obj.contains("List") || !obj.value("List").isArray())
        {
            emit newFriendRequestsState(true);
            return;
        }
        const QJsonArray arr = obj.value("List").toArray();
        if(arr.isEmpty())
        {
            emit newFriendRequestsState(true);
            return;
        }
        emit cleanNewFriendRequestsList();
        emit newFriendRequestsState(false);
        for(const QJsonValue& jv : arr)
        {
            if(!jv.isObject())
                continue;
            QJsonObject obj = jv.toObject();

            emit newFriendRequests(obj.value("UID").toString(),
                                   obj.value("SID").toString(),
                                   obj.value("Username").toString(),
                                   obj.value("Avatar_Url").toString(),
                                   obj.value("VerMsg").toString());
        }
    }
    else
    {
        if(obj.contains("AccessTokenExpired") && obj.value("AccessTokenExpired").isBool())
        {
            bool isExpired = obj.value("AccessTokenExpired").toBool();
            if(isExpired)
            {
                sendRefreshToken([this](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                    if(isSuccess)
                    {
                        if(newAccessToken.isEmpty())
                        {
                            emit refreshExpiredExit();
                            return;
                        }
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        emit newFriendRequestsState(false);
                        getNewFriendRequestsList();
                        return;
                    }
                    else
                    {
                        if(isRefreshTokenExpired)
                            emit refreshExpiredExit();
                        else
                        {
                            emit newFriendRequestsState(true);
                        }
                    }
                });
            }
            return;
        }
        emit newFriendRequestsState(true);
    }
}

void TcpLongConnection::handleGetFriendListResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        emit newFriendRequestsState(true);
        return;
    }
    bool success = obj.value("Result").toBool();
    if(success)
    {
        if(!obj.contains("List") || !obj.value("List").isArray())
        {
            emit newFriendState(true);
            return;
        }
        const QJsonArray arr = obj.value("List").toArray();
        if(arr.isEmpty())
        {
            emit newFriendState(true);
            return;
        }
        emit cleanFriendList();
        emit newFriendState(false);
        for(const QJsonValue& jv : arr)
        {
            if(!jv.isObject())
                continue;
            QJsonObject obj = jv.toObject();

            emit newFriend(obj.value("UID").toString(),
                                   obj.value("SID").toString(),
                                   obj.value("Username").toString(),
                                   obj.value("Avatar_Url").toString(),
                                   obj.value("Email").toString(),
                                   obj.value("IsOnline").toBool(false));
        }
    }
    else
    {
        if(obj.contains("AccessTokenExpired") && obj.value("AccessTokenExpired").isBool())
        {
            bool isExpired = obj.value("AccessTokenExpired").toBool();
            if(isExpired)
            {
                sendRefreshToken([this](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                    if(isSuccess)
                    {
                        if(newAccessToken.isEmpty())
                        {
                            emit refreshExpiredExit();
                            return;
                        }
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        emit newFriendState(false);
                        getFriendList();
                        return;
                    }
                    else
                    {
                        if(isRefreshTokenExpired)
                            emit refreshExpiredExit();
                        else
                        {
                            emit newFriendState(true);
                        }
                    }
                });
            }
            return;
        }
        emit newFriendState(true);
    }
}

void TcpLongConnection::handlePushNewFriendRequests(QJsonObject obj)
{
    if(obj.contains("SID") && obj.value("SID").isString() &&
        obj.contains("UID") && obj.value("UID").isString() &&
        obj.contains("Username") && obj.value("Username").isString() &&
        obj.contains("VerMsg") && obj.value("VerMsg").isString() &&
        obj.contains("Avatar_Url") && obj.value("Avatar_Url").isString())
    {
        emit newFriendRequests(obj.value("UID").toString(),
                               obj.value("SID").toString(),
                               obj.value("Username").toString(),
                               obj.value("Avatar_Url").toString(),
                               obj.value("VerMsg").toString());
        return;
    }
    getNewFriendRequestsList();
}

void TcpLongConnection::handlePushNewFriend(QJsonObject obj)
{
    if(obj.contains("SID") && obj.value("SID").isString() &&
        obj.contains("UID") && obj.value("UID").isString() &&
        obj.contains("Username") && obj.value("Username").isString() &&
        obj.contains("Email") && obj.value("Email").isString() &&
        obj.contains("Avatar_Url") && obj.value("Avatar_Url").isString() &&
        obj.contains("IsOnline") && obj.value("IsOnline").isBool())
    {
        emit newFriend(obj.value("UID").toString(),
                               obj.value("SID").toString(),
                               obj.value("Username").toString(),
                               obj.value("Avatar_Url").toString(),
                               obj.value("Email").toString(),
                               obj.value("IsOnline").toBool());
        return;
    }
    getFriendList();
}

void TcpLongConnection::handlePushFriendStatus(QJsonObject obj)
{
    if(obj.contains("UID") && obj.value("UID").isString() && obj.contains("IsOnline") && obj.value("IsOnline").isBool())
    {
        emit FriendStatus(obj.value("UID").toString(), obj.value("IsOnline").toBool());
    }
}

void TcpLongConnection::handleSendMessageResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        //加载错误
        return;
    }
    bool success = obj.value("Result").toBool();
    if(success)
    {
        emit newFriendRequestsHandleResult(true);
        emit mainState(true, "处理成功");
        this->isAgree_handleAddNewFriendWaitingRefresh.reset();
        this->uid_handleAddNewFriendWaitingRefresh.clear();
        return;
    }
    else
    {
        if(obj.contains("AccessTokenExpired") && obj.value("AccessTokenExpired").isBool())
        {
            bool isExpired = obj.value("AccessTokenExpired").toBool();
            if(isExpired)
            {
                sendRefreshToken([this](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                    if(isSuccess)
                    {
                        if(newAccessToken.isEmpty())
                        {
                            emit refreshExpiredExit();
                            this->isAgree_handleAddNewFriendWaitingRefresh.reset();
                            this->uid_handleAddNewFriendWaitingRefresh.clear();
                            return;
                        }
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        //静默重试 或者 让用户重新操作
                        if(this->isAgree_handleAddNewFriendWaitingRefresh.has_value())
                            sendHandleNewFriendRequest(this->uid_handleAddNewFriendWaitingRefresh,
                                                       this->isAgree_handleAddNewFriendWaitingRefresh.value());
                        return;
                    }
                    else
                    {
                        if(isRefreshTokenExpired)
                        {
                            emit refreshExpiredExit();
                            this->isAgree_handleAddNewFriendWaitingRefresh.reset();
                            this->uid_handleAddNewFriendWaitingRefresh.clear();
                            return;
                        }
                        emit newFriendRequestsHandleResult(false);
                        emit mainState(false, "处理好友申请失败，请稍后再试");
                        this->isAgree_handleAddNewFriendWaitingRefresh.reset();
                        this->uid_handleAddNewFriendWaitingRefresh.clear();
                    }
                });
                return;
            }
            emit newFriendRequestsHandleResult(false);
            emit mainState(false, "处理好友申请失败，请稍后再试");
            this->isAgree_handleAddNewFriendWaitingRefresh.reset();
            this->uid_handleAddNewFriendWaitingRefresh.clear();
            return;
        }
        emit newFriendRequestsHandleResult(false);
        emit mainState(false, "处理好友申请失败，请稍后再试");
        this->isAgree_handleAddNewFriendWaitingRefresh.reset();
        this->uid_handleAddNewFriendWaitingRefresh.clear();
    }
}

uint64_t TcpLongConnection::getRequestsId()
{
    static std::atomic<uint64_t> requestsid{0};
    while(requestsid >= 125433700 && requestsid <= 125433710)
        requestsid++;
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
                    else if(type == "AddNewFriendRequestResp")
                    {
                        this->handleAddNewFriendRequestResp(obj);
                    }
                    else if(type == "HandleNewFriendRequestResp")
                    {
                        this->handleHandleNewFriendRequestResp(obj);
                    }
                    else if(type == "GetNewFriendRequestsListResp")
                    {
                        this->handleGetNewFriendRequestsListResp(obj);
                    }
                    else if(type == "GetFriendListResp")
                    {
                        this->handleGetFriendListResp(obj);
                    }
                    else if(type == "RefreshTokenResp")
                    {
                        this->handleRefreshTokenResp(obj);
                    }
                    else if(type == "AccessTokenLoginResp")
                    {
                        this->handleAccessTokenLoginResp(obj);
                    }
                    else if(type == "PushNewFriendRequests")
                    {
                        //125433701
                        if(obj.value("Requests_id").toString() == "125433701")
                        {
                            this->handlePushNewFriendRequests(obj);
                        }
                    }
                    else if(type == "PushNewFriend")
                    {
                        //125433702
                        if(obj.value("Requests_id").toString() == "125433702")
                        {
                            this->handlePushNewFriend(obj);
                        }
                    }
                    else if(type == "PushFriendStatus")
                    {
                        //125433703
                        if(obj.value("Requests_id").toString() == "125433703")
                        {
                            this->handlePushFriendStatus(obj);
                        }
                    }
                    else if(type == "SendMessageResp")
                    {
                        this->handleSendMessageResp(obj);
                    }
                    else
                    {
                        qDebug()<<"type error";
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

void TcpLongConnection::startConnect()
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        this->socket->abort();
        this->socket->connectToHost(SADDR,SPORT);
    }
}
