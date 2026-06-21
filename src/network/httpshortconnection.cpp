#include "httpshortconnection.h"

HttpShortConnection &HttpShortConnection::getHttpClient()
{
    static HttpShortConnection hsc;
    return hsc;
}

void HttpShortConnection::uploadAvatar(const QString &filePath)
{
    if(!TcpLongConnection::getTcpClient().isConnect())
    {
        //提示用户连接服务器失败
        return;
    }
    if(filePath.isEmpty())
    {
        //提示用户上传失败
        return;
    }

    QImage image(filePath);
    if(image.isNull())
    {
        //提示用户上传失败
        return;
    }
    //备份当前头像
    UserInfo::getUserInfo().backupAvatar();

    QImage image_scaled = image.scaled(400,400, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QByteArray imgData;
    QBuffer buffer(&imgData);
    buffer.open(QIODevice::WriteOnly);
    image_scaled.save(&buffer, "JPEG", 80);
    buffer.close();

    //构建 multipart/form-data请求
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"%1\"; filename=\"avatar.jpg\"").arg("file"));
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, "image/jpeg");
    imagePart.setBody(imgData);
    multiPart->append(imagePart);

    QNetworkRequest request(QUrl("http://192.168.153.128:9001/upload"));

    QNetworkReply* reply = this->httpmanager->post(request, multiPart);
    multiPart->setParent(reply);

    //绑定发回来的事件
    connect(reply, &QNetworkReply::finished, this, [reply, this](){
        reply->deleteLater();
        if(reply->error() != QNetworkReply::NoError)
        {
            //提示用户上传失败
            qDebug()<<"上传失败:" << reply->errorString();
            return;
        }

        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if(doc.isNull() || !doc.isObject())
        {
            //提示用户上传失败
            qDebug()<<"错误的Json回复";
            return;
        }
        QString url = doc.object().value("Url").toString();
        if(url.isEmpty())
        {
            //提示用户上传失败
            qDebug()<<"没有收到url";
            return;
        }

        getAvatar(url, 3);
        TcpLongConnection::getTcpClient().sendUpadteAvatar(url);
    });
}

void HttpShortConnection::getAvatar(const QString &url, size_t retryTime)
{
    QNetworkRequest request(url);
    QNetworkReply* reply = this->httpmanager->get(request);

    connect(reply, &QNetworkReply::finished, this, [reply, this, url, retryTime](){
        reply->deleteLater();

        if(reply->error() != QNetworkReply::NoError)
        {
            if(retryTime > 1)
            {
                QTimer::singleShot(2000, [url, retryTime](){
                    HttpShortConnection::getHttpClient().getAvatar(url, retryTime - 1);
                });
            }
            else
            {
                //发送获取头像错误
            }
            return;
        }

        QPixmap avatar;
        avatar.loadFromData(reply->readAll());
        if(avatar.isNull())
        {
            //提示用户获取头像失败
            return;
        }
        emit AvatarReady(avatar);
    });
}

HttpShortConnection::HttpShortConnection(QObject *parent)
    : QObject{parent}
{
    httpmanager = new QNetworkAccessManager(this);
}
