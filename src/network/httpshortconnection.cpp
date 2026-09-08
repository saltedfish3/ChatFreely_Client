#include "httpshortconnection.h"
#include "tcplongconnection.h"
#include "../utils/userinfo.h"

HttpShortConnection &HttpShortConnection::getHttpClient()
{
    static HttpShortConnection hsc;
    return hsc;
}

void HttpShortConnection::uploadAvatar(const QString &filePath)
{
    if(!TcpLongConnection::getTcpClient().isConnect())
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        return;
    }
    if(filePath.isEmpty())
    {
        emit mainState(false, "上传失败，请稍后再试");
        return;
    }

    QImage image(filePath);
    if(image.isNull())
    {
        emit mainState(false, "上传失败，请稍后再试");
        return;
    }
    //备份当前头像
    UserInfo::getUserInfo().backupAvatarUrl();

    QImage image_scaled = image.scaled(400,400, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QByteArray imgData;
    QBuffer buffer(&imgData);
    buffer.open(QIODevice::WriteOnly);
    image_scaled.save(&buffer, "JPEG", 80);
    buffer.close();

    QString localUrl = "local://avatar_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    ImageCacheManager::getManager().insertCache(localUrl, imgData);

    QString cacheFilePath = ImageCacheManager::getManager().getCacheFilePath(localUrl);
    if(cacheFilePath.isEmpty())
    {
        emit mainState(false, "无法连接服务器，请稍后再试");
        ImageCacheManager::getManager().removeCache(localUrl);
        return;
    }

    uploadImage(cacheFilePath, [this, localUrl](const QString& url){
        ImageCacheManager::getManager().migrateCache(localUrl, url);
        UserInfo::getUserInfo().setAvatarUrl(url);
        TcpLongConnection::getTcpClient().sendUpadteAvatar(url);
    }, true, [this, localUrl](const QString& info){
        ImageCacheManager::getManager().removeCache(localUrl);
        emit mainState(false, info);
    });
}

void HttpShortConnection::uploadImage(const QString &filePath, std::function<void (const QString &)> cb_success, bool failed_notice, std::function<void (const QString &)> cb_failed)
{
    if(!TcpLongConnection::getTcpClient().isConnect())
    {
        if(failed_notice)
            emit mainState(false, "无法连接服务器，请稍后再试");
        if(cb_failed)
            cb_failed("无法连接服务器，请稍后再试");
        return;
    }

    if(filePath.isEmpty() || !QFile::exists(filePath))
    {
        if(failed_notice)
            emit mainState(false, "无法连接服务器，请稍后再试");
        if(cb_failed)
            cb_failed("无法连接服务器，请稍后再试");
        return;
    }

    //读取文件
    QFile file(filePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        if(failed_notice)
            emit mainState(false, "上传失败，请稍后再试");
        if(cb_failed)
            cb_failed("上传失败，请稍后再试");
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    if(fileData.size() > 10 * 1024 * 1024)
    {
        if(failed_notice)
            emit mainState(false, "图片大小要 ≤10 MB");
        if(cb_failed)
            cb_failed("图片大小要 ≤10 MB");
        return;
    }

    QByteArray format = getImageFormat(fileData);
    if(format.isEmpty() || (format != "JPEG" && format != "PNG"))
    {
        QImage image;
        if(!image.loadFromData(fileData))
        {
            if(failed_notice)
                emit mainState(false, "不支持的图片格式");
            if(cb_failed)
                cb_failed("不支持的图片格式");
            return;
        }

        //尝试转换为PNG
        QByteArray newData;
        QBuffer buffer(&newData);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        buffer.close();
        fileData = newData;
        format = "PNG";
    }

    QString mimeType = (format == "JPEG") ? "image/jpeg" : "image/png";
    QString fileName = QFileInfo(filePath).completeBaseName() + ((format == "JPEG") ? ".jpg" : ".png");

    //构建 multipart/form-data请求
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName));
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    imagePart.setBody(fileData);
    multiPart->append(imagePart);

    QNetworkRequest request(QUrl("http://192.168.153.128:9001/upload"));
    request.setRawHeader("Authorization", "Bearer " + UserInfo::getUserInfo().getAccessToken().toUtf8());

    QNetworkReply* reply = this->httpmanager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [reply, this, filePath, cb_success, cb_failed, failed_notice](){
        reply->deleteLater();
        if(reply->error() == QNetworkReply::AuthenticationRequiredError)
        {
            TcpLongConnection::getTcpClient().sendRefreshToken([this, filePath, cb_success, cb_failed, failed_notice](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                if(isSuccess)
                {
                    if(!newAccessToken.isEmpty())
                    {
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        uploadImage(filePath, cb_success, failed_notice, cb_failed);
                    }
                    else
                    {
                        emit refreshExpiredExit();
                    }
                    return;
                }
                else
                {
                    if(isRefreshTokenExpired)
                    {
                        emit refreshExpiredExit();
                        return;
                    }
                    else
                    {
                        if(failed_notice)
                            emit mainState(false, "上传失败，请稍后再试");
                        if(cb_failed)
                            cb_failed("上传失败，请稍后再试");
                        return;
                    }
                }
            });
            return;
        }

        if(reply->error() != QNetworkReply::NoError)
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        QByteArray responseData = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if(doc.isNull() || !doc.isObject())
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }
        QString url = doc.object().value("Url").toString();
        if(url.isEmpty())
        {
            if(failed_notice)
                emit mainState(false, "上传失败，请稍后再试");
            if(cb_failed)
                cb_failed("上传失败，请稍后再试");
            return;
        }

        if(cb_success)
            cb_success(url);
    });
}

void HttpShortConnection::getImage(const QString &url, size_t retryTime, std::function<void(const QByteArray&, ImageError)> onSuccess, bool failed_notice)
{
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", "Bearer " + UserInfo::getUserInfo().getAccessToken().toUtf8());
    QNetworkReply* reply = this->httpmanager->get(request);

    connect(reply, &QNetworkReply::finished, this, [reply, this, url, retryTime, failed_notice, onSuccess](){
        reply->deleteLater();

        if(reply->error() == QNetworkReply::AuthenticationRequiredError)
        {
            TcpLongConnection::getTcpClient().sendRefreshToken([this, url, retryTime, onSuccess, failed_notice](bool isSuccess, const QString& newAccessToken, bool isRefreshTokenExpired){
                if(isSuccess)
                {
                    if(!newAccessToken.isEmpty())
                    {
                        UserInfo::getUserInfo().setAccessToken(newAccessToken);
                        getImage(url, retryTime, onSuccess, failed_notice);
                    }
                    else
                    {
                        emit refreshExpiredExit();
                    }
                    return;
                }
                else
                {
                    if(isRefreshTokenExpired)
                    {
                        emit refreshExpiredExit();
                        return;
                    }
                    else
                    {
                        if(failed_notice)
                            emit mainState(false, "获取头像信息失败");
                    }
                }
            });
            return;
        }

        if(reply->error() == QNetworkReply::ContentNotFoundError)
        {
            if(onSuccess)
                onSuccess(QByteArray(), ImageError::NotFound);
        }

        if(reply->error() != QNetworkReply::NoError)
        {
            if(retryTime > 1)
            {
                QTimer::singleShot(2000, [url, retryTime, onSuccess, failed_notice](){
                    HttpShortConnection::getHttpClient().getImage(url, retryTime - 1, onSuccess, failed_notice);
                });
            }
            else
            {
                if(failed_notice)
                    emit mainState(false, "获取头像信息失败");
            }
            return;
        }

        QByteArray data = reply->readAll();
        if(onSuccess)
        {
            onSuccess(data, ImageError::NoError);
        }
    });
}

HttpShortConnection::HttpShortConnection(QObject *parent)
    : QObject{parent}
{
    httpmanager = new QNetworkAccessManager(this);
}

QByteArray HttpShortConnection::getImageFormat(const QByteArray &data) const
{
    if(data.startsWith("\xFF\xD8"))
        return "JPEG";
    if(data.startsWith("\x89PNG\r\n\x1A\n"))
        return "PNG";
    if(data.startsWith("GIF87a") || data.startsWith("GIF89a"))
        return "GIF";

    return QByteArray();
}
