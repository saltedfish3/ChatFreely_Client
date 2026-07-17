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
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();
    obj["Url"] = url;
    this->avatarUrl_waitingRefresh = url;

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
    obj["AccessToken"] = UserInfo::getUserInfo().getAccessToken();
    obj["Username"] = username;
    this->username_waitingRefresh = username;

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

void TcpLongConnection::getNewFriendRequestsList(std::function<void ()> callBack)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit mainState(false, "nc");
        return;
    }
    QJsonObject obj;
    QString requestsID = QString::number(getRequestsId());
    obj["Requests_id"] = requestsID;
    obj["Type"] = "GetNewFriendRequestsList";
    obj["UID"] = UserInfo::getUserInfo().getUID();

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
    QString requestsID = QString::number(getRequestsId());
    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "AddNewFriendRequest";
    obj["UID"] = UserInfo::getUserInfo().getUID();
    if(sid.isEmpty())
    {
        obj["Receiver_Email"] = email;
    }
    else
    {
        obj["Receiver_SID"] = sid;
    }
    if(!verMsg.isEmpty())
        obj["VerMsg"] = verMsg;

    QJsonDocument doc(obj);
    QByteArray data =doc.toJson(QJsonDocument::Compact) + "\n";
    this->socket->write(data);
    this->socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit mainState(false, "申请添加好友超时，请稍后再试");
        }
    });
}

void TcpLongConnection::sendHandleNewFriendRequest(QString uid, QString handle_uid, bool isAgree)
{
    if(this->socket->state() != QAbstractSocket::ConnectedState)
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        return;
    }
    if(uid.isEmpty() && handle_uid.isEmpty())
    {
        emit mainState(false, "处理申请错误，请稍后再试");
        return;
    }
    QString requestsID = QString::number(getRequestsId());
    QJsonObject obj;
    obj["Requests_id"] = requestsID;
    obj["Type"] = "HandleNewFriendRequest";
    obj["Status"] = isAgree;
    obj["UID"] = uid;
    obj["HandleUID"] = handle_uid;

    QJsonDocument doc(obj);
    QByteArray data =doc.toJson(QJsonDocument::Compact) + "\n";
    this->socket->write(data);
    this->socket->flush();

    this->waiting_requestsID.insert(requestsID.toStdString());
    QTimer::singleShot(5000,[this,requestsID](){
        if(this->waiting_requestsID.erase(requestsID.toStdString()))
        {
            emit mainState(false, "处理好友申请超时，请稍后再试");
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
                        getNewFriendRequestsList([](){});
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
                getNewFriendRequestsList([](){});
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
    getNewFriendRequestsList([](){});
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
                        sendUpadteAvatar(this->avatarUrl_waitingRefresh);
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
}

void TcpLongConnection::handleUnLoginResp(QJsonObject obj)
{
    //判断unLogin的AccessToken是否过期
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        emit mainState(false, "注销失败，请重试");
        this->isTryToUnLoginRefresh = false;
        return;
    }
    bool success = obj.value("Result").toBool();
    if(!success)
    {
        if(!obj.contains("AccessTokenExpired") || !obj.value("AccessTokenExpired").isBool())
        {
            emit mainState(false, "注销失败，请重试");
            this->isTryToUnLoginRefresh = false;
            return;
        }
        bool isExpired = obj.value("AccessTokenExpired").toBool();
        if(isExpired && !this->isTryToUnLoginRefresh)
        {
            this->isTryToUnLoginRefresh = true;
            sendRefreshToken([this](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
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
            });
            return;
        }

        emit mainState(false, "注销失败，请重试");
        this->isTryToUnLoginRefresh = false;
        return;
    }
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
                        sendUpdateUsername(this->username_waitingRefresh);
                        return;
                    }
                    else
                    {
                        if(isRefreshTokenExpired)
                        {
                            emit refreshExpiredExit();
                            return;
                        }
                        emit mainState(false, "更新失败");
                    }
                });
                return;
            }

        }
        emit mainState(false, "更新失败");
    }
}

void TcpLongConnection::handleAddNewFriendRequestResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        emit mainState(false, "申请添加好友失败，请稍后重试");
        return;
    }
    bool success = obj.value("Result").toBool();
    if(success)
    {
        emit mainState(true, "成功发送添加好友申请");
    }
    else
    {
        if(!obj.contains("Info") || !obj.value("Info").isString())
        {
            emit mainState(false, "申请添加好友失败，请稍后重试");
            return;
        }
        emit mainState(false, obj.value("Info").toString());
    }
}

void TcpLongConnection::handleHandleNewFriendRequestResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        emit mainState(false, "处理好友申请失败，请稍后再试");
        return;
    }
    bool success = obj.value("Result").toBool();
    if(success)
    {
        emit mainState(true, "成功发送添加好友申请");
    }
    else
    {
        emit mainState(false, "处理好友申请失败，请稍后再试");
    }
}

void TcpLongConnection::handleGetNewFriendRequestsListResp(QJsonObject obj)
{
    QString requestsID = obj.value("Requests_id").toString();
    this->waiting_requestsID.erase(requestsID.toStdString());
    if(!obj.contains("Result") || !obj.value("Result").isBool())
    {
        emit mainState(false, "获取好友申请失败，请稍后再试");
        return;
    }
    bool success = obj.value("Result").toBool();
    if(success)
    {
        if(!obj.contains("List") || !obj.value("List").isArray())
        {
            emit mainState(false, "获取好友申请失败，请稍后再试");
            return;
        }
        const QJsonArray arr = obj.value("List").toArray();
        emit cleanNewFriendRequestsList();
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
        //显示没有好友申请
        qDebug()<<"没有好友申请";
    }
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
                    else if(type == "RefreshTokenResp")
                    {
                        this->handleRefreshTokenResp(obj);
                    }
                    else if(type == "AccessTokenLoginResp")
                    {
                        this->handleAccessTokenLoginResp(obj);
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
